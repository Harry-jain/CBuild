# CBuild IDE Workstation

## Overview
CBuild is a hyper-optimized, standalone, zero-dependency C++ IDE Workstation engineered explicitly for Operating System development and bare-metal compilation pipelines.

**RESTRICTION NOTICE**: This is a **CLOSED SOURCE**, proprietary project. Unauthorized distribution, modification, reverse engineering, or sharing of this source code repository is strictly prohibited.

## Architecture & Technology
- **Execution & Dependencies**: Absolutely zero external dependencies. The binary is statically compiled against `/MT` (Windows) and `musl-libc` (Linux) preventing any missing DLL/shared object failures. We do not use Qt, GLFW, ImGui, Boost, or GLAD.
- **Windowing**: Direct Win32 `WNDCLASSEX` and X11 raw bindings hooking native OS message routing via `PeekMessage`/`XPending`.
- **Rendering**: Proprietary 2D Software Rasterizer performing highly optimized SIMD-friendly memory blitting via `SetDIBitsToDevice` pushing a continuous `uint32_t` RAM framebuffer direct to the screen. No OpenGL/GPU execution context exists.
- **Math Layer**: Zero-dependency floating-point geometry engine containing explicit cramer's rule intersection algorithms and sweeping AABB dynamics.
- **Integration**: Direct IPC socket-layer communication communicating with custom backend orchestrators mapping to containerized clusters (Docker) or LLMs (Ollama).

## How to Run

### Windows (Primary Build Target)
1. Ensure you have **CMake (3.20+)** and **Visual Studio 2022 Build Tools (MSVC)** installed on your machine.
2. Open the **"x64 Native Tools Command Prompt for VS 2022"**.
3. Navigate to the CBuild repository root directory.
4. Execute the following commands to configure the CMake `/MT` strict static build bindings and trigger MSBuild:
   ```cmd
   cmake -B build
   cmake --build build --config Release
   ```
5. Run the generated production binary (this will spawn the OS-native rendering window without spawning console output):
   ```cmd
   Start-Process -NoNewWindow .\build\Release\CBuild.exe
   ```

### Linux / WSL (Secondary Targeting)
1. Ensure the explicit `musl-tools` static linking compilers and core essentials are installed:
   ```bash
   sudo apt-get update && sudo apt-get install -y musl-tools build-essential cmake libx11-dev
   ```
2. Execute the isolated build system:
   ```bash
   cmake -B build -DCBUILD_STATIC_MUSL=ON -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```
3. Run the binary executable payload:
   ```bash
   ./build/CBuild
   ```
