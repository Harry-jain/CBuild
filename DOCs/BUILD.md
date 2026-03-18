# Build Instructions

CBuild is designed to be statically compiled with no external dependencies. 

## Requirements
- CMake 3.20 or newer
- **Windows**: MSVC compiler (`cl.exe`) capable of `/MT` linking. Windows SDK for Win32 API headers.
- **Linux**: GCC or Clang. Explicitly requires `musl-libc` toolchain for the static binary, or standard glibc if not doing a strict static build. Requires `libx11-dev` (for X11 headers, though final binary links statically to musl if fully isolated, or dynamically to X11 if native).

## Compiling on Windows

1. Open **x64 Native Tools Command Prompt for VS 2022**.
2. Navigate to the CBuild directory:
   ```cmd
   cd d:\Projects\CBuild
   cmake -B build
   cmake --build build --config Release
   ```
3. The executable `CBuild.exe` will be located in `build\Release\`.

## Compiling on Linux

1. Ensure standard build tools are present:
   ```bash
   sudo apt-get install build-essential cmake libx11-dev
   ```
2. Build via CMake:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```
3. Execute `./build/CBuild`
