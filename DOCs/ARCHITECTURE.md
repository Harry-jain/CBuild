# CBuild Architecture

CBuild is a standalone, C++, no-dependency IDE workstation engineered for Operating System development.

## Constraints
1. **Zero GUI Dependencies:** The application relies entirely on Win32 API (`CreateWindowEx`, `BitBlt`) on Windows and X11 on Linux. Qt, wxWidgets, Electron, SDL are explicitly forbidden.
2. **Zero Network Libraries:** HTTP functionality is built on raw POSIX/Winsock sockets. No `libcurl`, `Boost.Asio`, etc.
3. **No External Parsers:** Custom JSON parser logic per requirements; no nlohmann/json or RapidJSON.
4. **Air-Gapped Operation:** All Docker and Ollama orchestrations hit local sockets (`/var/run/docker.sock`, `localhost:11434`).

## Technical Stack
- **OS Windowing:** Win32 `WNDCLASSEX` / Linux `Xlib`.
- **Framebuffer:** 1D array of `uint32_t` pixels, pushed to screen via `SetDIBitsToDevice` or `XPutImage`/`XShm`.
- **Text Rendering:** Single-header `stb_easy_font.h` manually copied to the framebuffer.
- **Math/Snap Engine:** High-performance Vector/AABB and computational geometry implementations for the canvas.

## System Layer
The system provides a non-blocking main loop multiplexing the message queue and non-blocking IPC sockets for real-time responsiveness.
