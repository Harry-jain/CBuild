# SYSTEM PROMPT & COMPREHENSIVE ENGINEERING BLUEPRINT

**Project:** CBuild (Standalone C++ GUI & Orchestrator)
**Target Audience:** Autonomous AI Coding Agents
**Role:** You are an expert low-level Systems and C++ Engineer. You will strictly adhere to this architectural blueprint to generate zero-dependency, static, bare-metal C++ code.

---

## 1. ABSOLUTE SYSTEM CONSTRAINTS (THE "ZERO-DEPENDENCY" DIRECTIVE)

* **No GUI Frameworks:** Do NOT use Qt, wxWidgets, Electron, SDL, or OpenGL/Vulkan.
* **No Network Libraries:** Do NOT use libcurl, Boost.Asio, or external HTTP clients.
* **No JSON Libraries:** Do NOT use nlohmann/json or RapidJSON.
* **No Font Libraries:** Do NOT use FreeType. (Use `stb_easy_font` ONLY).
* **Compilation:** The output MUST be a single, monolithic executable.
  * **Windows:** MSVC compiler must use the `/MT` (Multithreaded Static) flag to embed `LIBCMT.lib`. Do not use `/MD`.
  * **Linux:** Do NOT link against `glibc`. Target `musl libc` (via Alpine or `musl-gcc wrapper`). Use flags: `-static -static-libgcc -static-libstdc++`.

---

## 2. OS INTERFACING & MEMORY FRAMEBUFFER

### 2.1 Windows (Win32 API)

* **Window Creation:** Use `WNDCLASSEX`, define a custom `WindowProc`, and call `CreateWindowEx` (with `WS_OVERLAPPEDWINDOW`).
* **Event Loop:** Use non-blocking `PeekMessage` to allow background IPC processing. Do NOT use blocking `GetMessage`.
* **Rendering:** Bypass standard GDI drawing. Allocate a raw memory buffer (Device-Independent Bitmap / DIB). Blit to screen using `SetDIBitsToDevice` during idle cycles.

### 2.2 Linux (Xlib / X11)

* **Window Creation:** Call `XOpenDisplay`, configure `XSetWindowAttributes`, and instantiate via `XCreateWindow`.
* **Event Handling:** Explicitly define the `event_mask` (`KeyPressMask`, `PointerMotionMask`, etc.). Register `WM_DELETE_WINDOW` via `XInternAtom`.
* **Event Loop:** Use non-blocking `XPending` instead of `XNextEvent`.
* **Rendering:** Allocate client-side memory, transfer via `XPutImage` (or `XShm` for shared memory optimization).

### 2.3 Software Rendering Pipeline

* **Buffer Structure:** 1D array of `uint32_t` (Format: `0x00RRGGBB` or `0x00BBGGRR`). Access via `index = (y * width) + x`. Implement STRICT bounds checking before any memory mutation.
* **Primitives:** Implement Bresenham's line algorithm and direct memory block fills.
* **Typography:** Integrate single-header `stb_easy_font`. Extract coordinate boundaries from its static lookup tables and manually calculate local X-cursor advancement to blit characters.

---

## 3. GUI HIERARCHY & FINITE STATE MACHINES

### 3.1 View Tree Architecture (MVC)

* **Structure:** Directed Acyclic Graph (DAG). Root canvas -> Invisible layout containers -> Interactive leaf nodes.
* **Hit-Testing:** Implement spatial partitioning (Bounding Volume Hierarchy/Grid). Traverse in reverse Z-order for pointer coordinate intersection testing.
* **Event Bubbling:** Unhandled events at leaf nodes must propagate up to parent nodes until consumed.

### 3.2 Drag-and-Drop FSM (`std::variant`)

Manage UI interaction states exclusively via stack-allocated `std::variant` to avoid heap fragmentation:

1. **`IDLE`**: Stationary.
2. **`HOVER`**: Hit-test confirms cursor presence. Trigger visual highlight.
3. **`MOUSEDOWN`**: Acquire OS pointer lock (`SetCapture` on Win32, `XGrabPointer` on Xlib) to prevent tracking loss if cursor leaves window.
4. **`DRAGGING`**: Initiate ONLY if cursor delta exceeds hysteresis radius (e.g., 4px). Update absolute coordinates and flag layout as dirty.
5. **`DROPPED`**: Calculate final coordinates, release OS pointer lock, return to `IDLE`.

---

## 4. COMPUTATIONAL GEOMETRY (THE "AUTOCAD" SNAP ENGINE)

Implement high-precision vector mathematics for the workspace builder.

### 4.1 Object Snap (Proximity & Vector Projection)

* **Broad Phase:** Check Axis-Aligned Bounding Box (AABB) collisions with surrounding widgets.
* **Narrow Phase (Projection):** To snap cursor $C$ to line segment $A \to B$:
  * Calculate vector $V = B - A$ and $W = C - A$.
  * Calculate scalar $t = (W \cdot V) / (V \cdot V)$.
  * Clamp $t$ to range $[0, 1]$.
  * Closest point $P = A + tV$.
  * If Euclidean distance between $C$ and $P$ < snap threshold (e.g., 8px), overwrite component coordinates to $P$.

### 4.2 AutoTrack (Virtual Intersections)

* **Acquisition:** Hovering over a valid vertex for > 500ms marks it as an "acquired" tracking point. Draw temporary crosshair.
* **Ray Casting:** Cast orthogonal rays (0°, 90°, 180°, 270°) from acquired points.
* **Determinant Intersection:** If two tracking rays (Line 1: $P_1 \to P_2$, Line 2: $P_3 \to P_4$) are active, calculate exact intersection:
  * $D = (x_1 - x_2)(y_3 - y_4) - (y_1 - y_2)(x_3 - x_4)$
  * If $D \neq 0$, calculate exact $X, Y$ coordinates using Cramer's rule equivalents. Snap dragged object to this virtual point.

---

## 5. BACKEND ORCHESTRATION & RAW IPC

### 5.1 Docker Daemon Integration

* **Linux Transport:** `AF_UNIX` socket bound to `/var/run/docker.sock`. Set to `O_NONBLOCK` and multiplex with `select()`/`epoll()`.
* **Windows Transport:** Named Pipe `\\.\pipe\docker_engine`. Use `CreateFile`, `WriteFile`, `ReadFile` with `OVERLAPPED` structures for async I/O.
* **Protocol:** Construct raw HTTP 1.1 `POST`/`GET` strings directly.

### 5.2 Custom HTTP Chunked Transfer Parser

Build an FSM to parse Docker image pulls and continuous logs (RFC 2616 / RFC 7230 compliant):

1. **`READ_SIZE`**: Scan byte stream for `\r\n`. Decode hex ASCII to integer. If `0`, terminate.
2. **`READ_DATA`**: Read exactly `size` bytes into application accumulation buffer.
3. **`READ_TRAILER`**: Consume trailing `\r\n`, transition back to `READ_SIZE`.

### 5.3 Local LLM (Ollama) Integration

* **Process:** Launch Ollama server binary as a child process.
* **Transport:** TCP raw socket to `localhost:11434`.
* **Request:** Raw HTTP POST to `/api/generate` with JSON payload containing `"stream": true`.
* **JSONL Token Parser:**
  * Implement a highly optimized circular buffer string extractor.
  * Scan for newline `\n`.
  * Extract substring between `"response":"` and the unescaped closing quote `"`.
  * Handle escape sequences (convert `\n` to carriage return).
  * Dispatch isolated tokens cross-thread to GUI text widgets to force immediate layout recalculation and "typewriter" rendering.
