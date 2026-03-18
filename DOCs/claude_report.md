# SYSTEM PROMPT & DETAILED ENGINEERING SPECIFICATION: CBuild

## 1. Project Overview & AI Directives

**Name:** CBuild
**Mission:** Develop a pure C++, zero-dependency visual application builder and IDE workstation dedicated to bare-metal Operating System development.
**Language & Standard:** Strict C++17 or C++20.
**Core Philosophy:** The AI must operate at the lowest possible abstraction layer. Prioritize raw performance, deterministic execution, and explicit memory control over modern frameworks.

### 1.1 Strict Architectural Constraints

* **Zero GUI Dependencies:** NO Qt, Electron, GTK, wxWidgets, .NET, or JVM.
* **Static Linking Mandate:** The final artifact must be a single executable.
  * *Linux:* Use `musl libc` for static linking.
  * *Windows:* Compile with `/MT` (Multithreaded Static Runtime) via MSVC.
* **Air-Gapped Operation:** The tool must not rely on cloud services, external APIs, or internet connectivity post-compilation. All LLM (Ollama) and Docker orchestrations must resolve to `localhost` via local sockets.

---

## 2. Low-Level Architecture & Implementation Guides

### 2.1 OS Abstraction & Windowing Layer

Do not use abstraction libraries like SDL or GLFW. The AI must write direct bindings to the host OS windowing systems.

* **Windows (Win32):** * Use `RegisterClassEx`, `CreateWindowEx`, and a custom `WindowProc`.
  * **Framebuffer:** Use a Device-Independent Bitmap (DIB) section via `CreateDIBSection`. Blast the pixel buffer to the screen using `BitBlt` or `StretchDIBits` during the `WM_PAINT` message.
* **Linux (X11):** * Direct `Xlib` programming. Use `XOpenDisplay`, `XCreateSimpleWindow`.
  * **Framebuffer:** Use `XImage` and `XPutImage`. For performance optimization, implement X11 Shared Memory Extension (`XShm`) to avoid IPC copying of the framebuffer.

### 2.2 Software Rendering Engine

The application must own a contiguous block of memory representing the screen.

* **Buffer Structure:** A 1D array of `uint32_t` representing 2D pixels (ARGB format). Size: `width * height * 4` bytes.
* **Rasterization:** Implement custom math for drawing.
  * Lines: Bresenham's Line Algorithm.
  * Rectangles: Direct memory block filling (e.g., `std::memset` or optimized `for` loops).
  * Text: Integrate `stb_easy_font.h` (the only permitted 3rd-party header) mapped directly to the pixel buffer.
* **Optimization:** Implement "Dirty Rectangles" (only redraw bounding boxes of widgets that have state changes) to maintain high FPS without a GPU.

### 2.3 The Main Event Loop

The application must handle UI events and background IPC simultaneously without freezing.

* Implement a non-blocking main loop.
* Multiplex OS UI messages (Win32 message queue / X11 pending events) with backend socket polling (using `select()`, `poll()`, or `epoll` on Linux; `select()` or `WSAPoll` on Windows).

---

## 3. Widget System & Finite State Machines (FSM)

### 3.1 Base Widget Class Architecture

All UI components must inherit from a pure virtual base class. The AI should use the following signature concepts:

* `BoundingBox rect`: X, Y, Width, Height.
* `virtual void Draw(uint32_t* framebuffer, int stride) = 0;`
* `virtual bool HandleEvent(const InputEvent& event) = 0;` (Returns true if event was consumed).
* `Z-Index`: Integer for draw ordering and input interception.

### 3.2 Canvas & Interaction FSM

Interactions must be strictly modeled using C++ `std::variant` and `std::visit` for deterministic state transitions.

* **Drag State Machine:** `Idle` -> `MousePress(target_widget)` -> `Dragging(dx, dy)` -> `MouseRelease` -> `Idle`.
* **Snap Engine:** Implement Axis-Aligned Bounding Box (AABB) math. During the `Dragging` state, project vectors to nearby widget edges. If distance `< SNAP_THRESHOLD` (e.g., 5px), override the target X/Y coordinates and draw an alignment tracking line (1px dashed line) on the canvas.

---

## 4. Backend Orchestrator & IPC Network Layer

### 4.1 Socket Implementation

The AI must implement raw socket wrappers (Winsock on Windows, POSIX sockets on Linux). **NO libcurl.**

* **Process Manager:** Wrap `popen()`, `fork()`, `execvp()`, and `pipe()` to launch and monitor build tools (`make`, cross-compilers).
* **Docker Daemon:** Connect via Unix Domain Socket (`AF_UNIX`) to `/var/run/docker.sock`. Construct raw HTTP/1.1 `GET` and `POST` requests as strings and write them to the socket.
* **HTTP/1.1 Chunked Parser:** Crucial for reading streaming logs from Docker and streaming tokens from Ollama. The AI must build a state machine to parse hex chunk sizes, read the exact byte count, and dispatch the payload to the UI thread.

### 4.2 LLM (Ollama) Integration Protocol

* **Connection:** TCP socket to `localhost:11434`.
* **Payload:** Construct raw JSON `POST /api/generate`.
* **Response Handling:** Parse the JSONL (JSON Lines) stream chunk-by-chunk. Extract the `"response"` token and append it to the `ChatBubble` widget's text buffer dynamically.

---

## 5. OS Development Specific Workflows (Target Implementations)

When instructed to build specific panels, the AI must adhere to these functional requirements:

* **QEMU / GDB/MI Panel:** * Connect to `localhost:1234`. Send `-exec-step`, `-data-list-register-values`.
  * Parse the asynchronous GDB Machine Interface (GDB/MI) tuple output (`{key="value",...}`) using a custom string tokenizer, mapping the results into the UI `Table` widget.
* **Hex Viewer Widget:**
  * **Constraint:** Must use "Virtualized Rendering." Do not load a 500MB `disk.img` into UI memory. Read chunks via `fseek`/`fread` based on the current scrollbar position and render only the visible 16-byte lines.
* **Memory Map Visualizer:**
  * Parse binary Multiboot2 info structures (specifically tag type 6: Memory map). Convert base addresses and lengths into proportional `Rect` dimensions on the canvas, color-coded by type (1=Available, 2=Reserved, 3=ACPI).
* **Serial Terminal (UART):**
  * Connect to QEMU's `-serial tcp:localhost:PORT`. Render incoming bytes as standard ASCII text, handling `\r\n` carriage returns correctly within the software-rendered text area.

---

## 6. Development Phasing Roadmap

1. **Phase 1: Bootstrapping (Host OS & Render):** Initialize Win32/X11. Allocate the `uint32_t` framebuffer. Implement Bresenham lines, `memset` rects, and push the buffer to the screen.
2. **Phase 2: Event Loop & UI Foundation:** Implement keyboard/mouse polling. Build the base `Widget` class, `Button`, and `Panel`. Implement the event routing hierarchy (Z-order intersection testing).
3. **Phase 3: The Canvas & Math Engine:** Implement the `std::variant` FSM for dragging. Implement AABB collision and the computational geometry required for the vector snap engine.
4. **Phase 4: Raw IPC & Network:** Build the POSIX/Winsock wrappers. Implement the raw HTTP/1.1 string builder and the chunked transfer encoding stream parser.
5. **Phase 5: Backend Orchestration:** Hook up `/var/run/docker.sock` for cross-compiler orchestration and `localhost:11434` for the Ollama streaming chat bubble.
6. **Phase 6: OS Dev Tooling Synthesis:** Build the complex widgets: GDB/MI Register Table, Hex Viewer, Multiboot2 Memory Mapper, and QEMU Serial console.
