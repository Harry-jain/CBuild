# CBuild — AI Agent Task Master

## Complete Build Specification: 510 Atomic Tasks

> **How to use this document:** Each row is a self-contained task for an AI coding agent.  
> **Priority levels:** `Critical` (blocks everything) · `High` (core feature) · `Medium` (important extension) · `Low` (polish/optional)  
> Tasks should be executed in section order (**A → N**) as later sections depend on earlier ones.

---

## Table of Contents

| Section | Name | Tasks |
|---------|------|-------|
| [A](#a-project-foundation) | Project Foundation | 35 |
| [B](#b-math-and-geometry-layer) | Math and Geometry Layer | 30 |
| [C](#c-platform-and-window-layer) | Platform and Window Layer | 36 |
| [D](#d-2d-renderer) | 2D Renderer | 44 |
| [E](#e-widget-system) | Widget System | 44 |
| [F](#f-canvas-and-scene-graph) | Canvas and Scene Graph | 37 |
| [G](#g-snap-engine) | Snap Engine | 35 |
| [H](#h-builder-app-and-ide-shell) | Builder App and IDE Shell | 39 |
| [I](#i-backend-integration-layer) | Backend Integration Layer | 43 |
| [J](#j-static-linking-and-distribution) | Static Linking and Distribution | 15 |
| [K](#k-os-development-specialist-tools) | OS Development Specialist Tools | 20 |
| [L](#l-advanced-ui-features) | Advanced UI Features | 35 |
| [M](#m-testing-and-quality) | Testing and Quality | 49 |
| [N](#n-advanced-features-and-integrations) | Advanced Features and Integrations | 48 |

---

## Priority Summary

| Priority | Count | Estimated Hours | Impact |
|----------|-------|----------------|--------|
| Critical | ~180 | ~360h | Blocks all other work |
| High | ~260 | ~390h | Core features, daily use |
| Medium | ~90 | ~180h | Important extensions |
| Low | ~50 | ~75h | Polish and optionals |
| **Total** | **~510** | **~1,005h** | — |

---

## A. Project Foundation

> These 35 tasks establish the build system, dependency management, and repository structure. No code is shipped without these foundations. The CI workflows must pass before any feature work begins.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 001 | Repository initialization | Create Git repository with `.gitignore` (ignoring `build/`, `.cache/`, `*.o`, `*.a`). Set up branch strategy: `main`, `dev`, `feature/*` naming. Add MIT license. **Acceptance:** `git init` and first commit with `README.md` template. | Critical |
| 002 | CMakeLists.txt scaffold | Create root `CMakeLists.txt` with `cmake_minimum_required(VERSION 3.20)`, `project()` declaration, C++17 standard enforcement, and empty executable target. Must produce zero warnings on `cmake -B build`. **Acceptance:** `cmake -B build` exits code 0. | Critical |
| 003 | Vendor directory structure | Create `vendor/` directory with subdirectories for `glad/`, `stb/`, and any future vendored single-header libs. Add `vendor/README.md` explaining what is vendored and why. **Acceptance:** Directory structure matches architectural spec. | High |
| 004 | GLAD OpenGL loader vendor setup | Download GLAD for OpenGL 3.3 Core Profile from glad.dav1d.de. Place `glad.h` in `vendor/glad/include/glad/`, `khrplatform.h` in `vendor/glad/include/KHR/`, `glad.c` in `vendor/glad/src/`. Add glad `STATIC` library target to CMakeLists.txt. **Acceptance:** `glad.c` compiles without errors. | Critical |
| 005 | stb single-header libraries fetch | Configure `CMakeLists.txt` FetchContent for stb repository. Ensure `stb_truetype.h` and `stb_easy_font.h` are accessible from include paths. **Acceptance:** `#include <stb_truetype.h>` compiles from any `src/` file. | High |
| 006 | GLFW FetchContent integration | Add `FetchContent_Declare` for GLFW 3.4 with `GIT_SHALLOW ON`. Disable `GLFW_BUILD_DOCS`, `GLFW_BUILD_TESTS`, `GLFW_BUILD_EXAMPLES`. Link `glfw` to main target. **Acceptance:** Empty GLFW window opens on all three target platforms. | Critical |
| 007 | nlohmann/json FetchContent integration | Add `FetchContent_Declare` for nlohmann/json v3.11.3. Set `JSON_BuildTests OFF`. Link `nlohmann_json::nlohmann_json` to main target. **Acceptance:** `#include <nlohmann/json.hpp>` compiles and basic JSON parse works in a test. | High |
| 008 | libcurl system package detection | Add `find_package(CURL REQUIRED)` to CMakeLists.txt. Link `CURL::libcurl`. Add platform-specific fallback message if not found. **Acceptance:** cmake fails with a helpful error message if libcurl is not installed, rather than a cryptic linker error. | Critical |
| 009 | Linux pthreads and dl linkage | On `UNIX AND NOT APPLE` platforms, `find_package(Threads REQUIRED)` and link `Threads::Threads` and `dl`. **Acceptance:** Worker threads link without `-pthread` linker errors on Linux. | High |
| 010 | macOS framework linkage | On `APPLE` platform, link `CoreFoundation`, `AppKit`, `IOKit`, `CoreVideo` frameworks. **Acceptance:** CMake build completes on macOS without unresolved symbol errors from GLFW. | High |
| 011 | Windows Winsock linkage | On `WIN32` platform, link `ws2_32`, `opengl32`, `winmm`. **Acceptance:** CMake build completes on Windows x64 without linker errors. | High |
| 012 | Compiler warning flags | Add `-Wall -Wextra -Wpedantic` for GCC/Clang. Add `/W4` for MSVC. Suppress `-Wno-unused-parameter` for event handler virtuals. **Acceptance:** Full build produces zero warnings after flags are applied. | Medium |
| 013 | Release build optimization flags | For `CMAKE_BUILD_TYPE=Release`: add `-O3 -DNDEBUG -march=native` (GCC/Clang) or `/O2 /DNDEBUG` (MSVC). **Acceptance:** Release binary is measurably faster than Debug binary on a rendering benchmark. | Medium |
| 014 | Debug build sanitizer flags | For `CMAKE_BUILD_TYPE=Debug`: add `-fsanitize=address,undefined` and `-g`. **Acceptance:** Debug binary reports heap-use-after-free and undefined behavior correctly in test programs. | High |
| 015 | Asset copy post-build command | Add CMake `POST_BUILD` command to copy `assets/` directory next to the built binary. **Acceptance:** `./build/CBuild` can load font files from `./build/assets/fonts/` without path manipulation. | Medium |
| 016 | CMake build summary message | Add `message(STATUS ...)` calls at the end of CMakeLists.txt printing: build type, compiler, OpenGL loader, all dependency versions. **Acceptance:** `cmake -B build` output ends with a readable summary box. | Low |
| 017 | setup_glad.sh script | Write a bash script that downloads GLAD files via curl from the correct Dav1dde/glad URLs. Script must create the correct directory structure. **Acceptance:** Running script on a clean machine correctly populates `vendor/glad/`. | High |
| 018 | Project directory scaffold script | Write `scripts/new_project.sh` that creates a new CBuild project directory with the correct `.cbuild` file template, `assets/` folder, and `backend/` stub. **Acceptance:** Running the script produces a valid empty CBuild project that can be opened. | Medium |
| 019 | CI workflow: Ubuntu build | Create `.github/workflows/build-ubuntu.yml` that checks out the repo, installs `libcurl4-openssl-dev`, runs `setup_glad.sh`, builds with cmake. **Acceptance:** Workflow passes on Ubuntu 22.04 runner. | High |
| 020 | CI workflow: macOS build | Create `.github/workflows/build-macos.yml`. Install curl via brew. Run full build. **Acceptance:** Workflow passes on `macos-latest` runner. | Medium |
| 021 | CI workflow: Windows build | Create `.github/workflows/build-windows.yml` using Visual Studio 2022 runner. Install curl via vcpkg. **Acceptance:** Workflow passes and produces a `.exe` artifact. | Medium |
| 022 | Static analysis integration | Integrate `clang-tidy` into the CMake build (`CMAKE_CXX_CLANG_TIDY`). Write a `.clang-tidy` config that enforces modern C++ rules. **Acceptance:** cmake build runs clang-tidy on all source files and reports any violations. | Low |
| 023 | Valgrind memory test target | Add a CMake custom target `valgrind` that runs the binary under `valgrind --leak-check=full`. **Acceptance:** cmake `--target valgrind` exits clean after a 5-second run of the UI. | Low |
| 024 | ASAN integration test | Write `test/asan_test.cpp` that creates and destroys all major objects (Window, Renderer2D, Canvas, BackendManager) in sequence. **Acceptance:** No heap-use-after-free or memory leaks reported. | High |
| 025 | Version header generation | Create a CMake `configure_file` step that generates `src/version.hpp` embedding `PROJECT_VERSION_MAJOR`, `PROJECT_VERSION_MINOR`, `GIT_COMMIT_HASH`, and `BUILD_DATE`. **Acceptance:** The binary prints its version string at startup. | Low |
| 026 | Compile-time feature flags | Define CMake options: `CBUILD_ENABLE_OPENGL` (default ON), `CBUILD_ENABLE_BACKEND` (default ON), `CBUILD_ENABLE_DOCKER` (default ON). **Acceptance:** `cmake -DCBUILD_ENABLE_DOCKER=OFF` builds successfully without Docker code. | Medium |
| 027 | Multi-config generator support | Ensure CMakeLists.txt works with both single-config (Makefiles, Ninja) and multi-config (Visual Studio, Xcode) generators. Use generator expressions instead of `CMAKE_BUILD_TYPE` where needed. **Acceptance:** `cmake --build build --config Release` works on all generators. | Medium |
| 028 | Packaging: Linux AppImage target | Add a CMake target `package-appimage` using linuxdeploy to bundle CBuild into a portable AppImage. **Acceptance:** Running `CBuild-x86_64.AppImage` on a clean Ubuntu machine launches the application. | Medium |
| 029 | Packaging: Windows installer | Add a CMake target `package-win` using CPack with NSIS generator. **Acceptance:** Running `CBuildSetup.exe` on a clean Windows 10 machine installs and launches CBuild correctly. | Low |
| 030 | Packaging: macOS .app bundle | Add a CMake target `package-macos` using CPack with Bundle generator. Create `CBuild.app` with correct `Info.plist`. **Acceptance:** `CBuild.app` runs on macOS without 'damaged application' errors. | Low |
| 031 | Reproducible build flags | Add `-ffile-prefix-map`, `-fmacro-prefix-map`, and `SOURCE_DATE_EPOCH` support. **Acceptance:** Two builds from the same source at different times produce byte-identical binaries. | Low |
| 032 | ccache integration | Add `find_program(CCACHE_PROGRAM ccache)` and set compiler launcher if found. **Acceptance:** Second build of unchanged source completes in under 5 seconds via cache hits. | Low |
| 033 | Ninja build file default | Set `cmake -G Ninja` as the default in setup documentation. **Acceptance:** `cmake -B build -G Ninja` followed by `ninja -C build` builds in under 45 seconds on a modern laptop. | Low |
| 034 | docs/BUILD.md | Write a comprehensive build guide documenting every dependency, installation command per platform, and troubleshooting steps. **Acceptance:** A developer who has never built a C++ project can successfully build CBuild by following BUILD.md alone. | Medium |
| 035 | docs/ARCHITECTURE.md | Write an architectural overview documenting all four layers, the rendering pipeline, snap engine algorithm, backend socket protocol, and undo/redo command stack. Include ASCII diagrams. **Acceptance:** Document is accurate and matches the implemented code. | Medium |

---

## B. Math and Geometry Layer

> All geometry in CBuild is computed by this layer. Every widget, snap point, canvas transform, and rendering primitive uses `Vec2`, `Rect`, `Color`, or `Mat3`. Build and unit-test this layer before touching any graphics or UI code.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 036 | Vec2 struct implementation | Implement `Vec2` with `float x, y`. Constructors: default `(0,0)` and `(x,y)`. Operators: `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, `==`, `!=`. Methods: `dot()`, `cross()`, `length()`, `length_sq()`, `normalized()`, `distance_to()`, `distance_sq_to()`. All header-only. **Acceptance:** Unit tests for all operators pass. | Critical |
| 037 | Vec2 snapped() method | Implement `Vec2::snapped(float grid_size)` rounding each component to nearest multiple of `grid_size` using `std::round`. **Acceptance:** `Vec2(13.7, 7.2).snapped(10) == Vec2(10, 10)`. | Critical |
| 038 | Vec2 lerp() static method | Implement `Vec2::lerp(Vec2 a, Vec2 b, float t)` returning `a + (b-a)*t`. **Acceptance:** `Vec2::lerp({0,0},{10,10},0.5) == Vec2(5,5)`. `t=0` returns `a`, `t=1` returns `b`. | High |
| 039 | Rect struct implementation | Implement `Rect` with `x, y, w, h` floats. Accessors: `position()`, `size()`, `center()`, all four corners, all four edge midpoints, `right()`, `bottom()`. **Acceptance:** All nine snap points return correct coordinates for a known rect. | Critical |
| 040 | Rect::contains() method | Implement `Rect::contains(Vec2 p)` returning true if `p` is inside (inclusive on all edges). **Acceptance:** Contains returns true at all four corners and center, false just outside each edge. | Critical |
| 041 | Rect::intersects() method | Implement `Rect::intersects(Rect o)` returning true if any portion of `o` overlaps this rect. Must handle: partial overlap, containment, edge touching, no overlap. **Acceptance:** 20 intersection test cases all pass. | High |
| 042 | Rect::expanded() method | Implement `Rect::expanded(float amount)` returning a new Rect grown by `amount` on all four sides. **Acceptance:** `Rect(10,10,20,20).expanded(5) == Rect(5,5,30,30)`. | High |
| 043 | Rect::merged() method | Implement `Rect::merged(Rect o)` returning the smallest Rect containing both. **Acceptance:** Two non-overlapping rects produce a merged rect that contains both. | Medium |
| 044 | Rect::translated() method | Implement `Rect::translated(Vec2 delta)` returning a new Rect shifted by delta without changing size. **Acceptance:** `Rect(10,10,20,20).translated({5,-5}) == Rect(15,5,20,20)`. | High |
| 045 | Rect::clamp() method | Implement `Rect::clamp(Vec2 p)` returning `p` clamped to stay within this rect. **Acceptance:** Points outside are brought to the nearest edge. Points inside return unchanged. | Medium |
| 046 | Color struct with float channels | Implement `Color` with `r, g, b, a` floats `[0..1]`. Default constructor sets `a=1`. Constructors: `(r,g,b)` and `(r,g,b,a)`. **Acceptance:** `Color(1,0,0)` is red with full opacity. | Critical |
| 047 | Color::hex() static factory | Implement `Color::hex(uint32_t rgba)` decoding `0xRRGGBBAA` format. **Acceptance:** `Color::hex(0xFF0000FF) == Color(1,0,0,1)`. | High |
| 048 | Color::rgb() static factory | Implement `Color::rgb(uint32_t rgb_val)` decoding `0xRRGGBB` with full opacity. **Acceptance:** `Color::rgb(0x4F46E5)` produces the correct indigo color. | High |
| 049 | Color::tinted() method | Implement `Color::tinted(float factor)` multiplying `r,g,b` by factor and clamping to `[0,1]`. **Acceptance:** `Color(0.5,0.5,0.5).tinted(2.0) == Color(1,1,1)`. | High |
| 050 | Color::lerp() method | Implement `Color::lerp(Color to, float t)` interpolating all four channels. **Acceptance:** `Red.lerp(Blue, 0.5)` produces a correct intermediate color. | Medium |
| 051 | Color design system palette | Implement all `Color` static methods for the CBuild design system: `canvas_bg`, `panel_bg`, `panel_border`, `accent`, `accent_dim`, `text_primary`, `text_secondary`, `snap_grid`, `snap_track`, `snap_point`, `snap_center`, `selection`, `selection_border`. **Acceptance:** All palette colors are correct hex values as specified. | High |
| 052 | Mat3 struct implementation | Implement `Mat3` as 3x3 column-major float array `m[col][row]`. Default constructor produces identity matrix. **Acceptance:** `Mat3{} * Vec2(5,3) == Vec2(5,3)`. | High |
| 053 | Mat3::translation() factory | Implement `Mat3::translation(float tx, float ty)` and `Mat3::translation(Vec2)`. **Acceptance:** `Mat3::translation(10,20).transform_point({0,0}) == Vec2(10,20)`. | High |
| 054 | Mat3::scale() factory | Implement `Mat3::scale(float sx, float sy)`. **Acceptance:** `Mat3::scale(2,3).transform_point({5,4}) == Vec2(10,12)`. | High |
| 055 | Mat3::rotation() factory | Implement `Mat3::rotation(float angle_rad)`. **Acceptance:** `Mat3::rotation(M_PI/2).transform_point({1,0})` is approximately `Vec2(0,1)`. | High |
| 056 | Mat3 matrix multiplication | Implement `Mat3::operator*(Mat3 b)`. Must be correct column-major multiplication. **Acceptance:** `translation(5,0) * scale(2,2)` applied to `Vec2(1,0)` produces `Vec2(7,0)`. | High |
| 057 | Mat3::transform_point() method | Implement `transform_point(Vec2 p)` applying the matrix including translation (w=1). **Acceptance:** Correctly transforms points through translate + rotate + scale pipelines. | High |
| 058 | Mat3::transform_dir() method | Implement `transform_dir(Vec2 d)` applying only rotation/scale (w=0, no translation). **Acceptance:** A direction vector is rotated correctly but not translated. | Medium |
| 059 | geom::closest_point_on_segment() | Implement `geom::closest_point_on_segment(Vec2 a, Vec2 b, Vec2 p)` using vector projection (dot product algorithm). Must clamp `t` to `[0,1]`. **Acceptance:** Correct closest point returned for point off-segment, at midpoint, and beyond both endpoints. | Critical |
| 060 | geom::distance_to_segment() | Implement `geom::distance_to_segment(Vec2 a, Vec2 b, Vec2 p)` using `closest_point_on_segment`. **Acceptance:** Returns 0 for a point on the segment, correct perpendicular distance for a point directly to the side. | High |
| 061 | geom::line_intersection() | Implement `geom::line_intersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4)` using the determinant algorithm. Returns `std::optional<Vec2>`. Returns nullopt if lines are parallel. **Acceptance:** Correctly finds intersection of perpendicular lines; returns nullopt for parallel lines. | Critical |
| 062 | geom::screen_to_ndc() | Implement `geom::screen_to_ndc(Vec2 p, float w, float h)` mapping screen pixels to OpenGL NDC `[-1,1]`. Remember Y-flip. **Acceptance:** Top-left corner maps to `(-1,1)`, center maps to `(0,0)`, bottom-right maps to `(1,-1)`. | Medium |
| 063 | Math unit test suite | Write `test/math_test.cpp` covering every `Vec2` operator, `Rect` method, `Color` factory, `Mat3` transform, and `geom::` function. **Acceptance:** All tests pass. Test coverage >= 90% of math code paths. | High |
| 064 | AABB sweep test | Implement `geom::swept_aabb(Rect moving, Vec2 velocity, Rect stationary)` returning the fraction of velocity at which the moving rect first touches the stationary rect. **Acceptance:** A rect moving toward a stationary rect returns a fraction < 1.0 at contact, 1.0 if no contact. | Medium |
| 065 | Polar coordinate conversion | Implement `Vec2 geom::polar_to_cartesian(float angle_rad, float radius)` and the inverse. **Acceptance:** `polar_to_cartesian(0, 1) == Vec2(1,0)`. `polar_to_cartesian(M_PI/2, 1) ≈ Vec2(0,1)`. | Medium |

---

## C. Platform and Window Layer

> The window layer is the only GLFW-touching code in the entire codebase. Every input event and display operation flows through here. Correctness here is critical — bugs surface as input latency or rendering artifacts everywhere.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 066 | Window class header design | Design `window.hpp` defining `Window` class with all callbacks as `std::function` members. Define event structs: `KeyEvent`, `CharEvent`, `MouseMoveEvent`, `MouseButtonEvent`, `ScrollEvent`, `ResizeEvent`. No GLFW types in the header. **Acceptance:** Header compiles without GLFW include. | Critical |
| 067 | GLFW initialization and error callback | Implement `Window::create()`. Call `glfwInit()`. Register `glfw_error_callback`. **Acceptance:** `glfwInit` failure produces a readable error message and returns false. | Critical |
| 068 | OpenGL 3.3 Core context hints | Set GLFW hints: `CONTEXT_VERSION_MAJOR=3`, `MINOR=3`, `PROFILE=CORE`, `FORWARD_COMPAT=TRUE`, `SAMPLES=4`. **Acceptance:** `glGetString(GL_VERSION)` returns '3.3' or higher and reports Core Profile. | Critical |
| 069 | Window creation and user pointer | Call `glfwCreateWindow()`. Store `this` in `glfwSetWindowUserPointer` for static callback access. **Acceptance:** Window opens and user pointer retrieval in a static callback returns the correct `Window*`. | Critical |
| 070 | GLAD function pointer loading | In `main.cpp`, call `gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)` after context creation. Print `GL_VERSION` and `GL_RENDERER`. **Acceptance:** `gladLoadGLLoader` returns non-zero. GL function calls execute without crash. | Critical |
| 071 | Key callback registration and dispatch | Implement static `glfw_key_callback`. Map `GLFW_PRESS/RELEASE/REPEAT` to `KeyAction` enum. Call `on_key` callback if set. **Acceptance:** Key presses/releases reach the `on_key` handler with correct keycode and action. | Critical |
| 072 | Char callback registration | Implement static `glfw_char_callback`. Dispatch Unicode codepoint to `on_char` callback. **Acceptance:** Typing 'A' produces codepoint 65. Typing '€' produces codepoint 8364. | High |
| 073 | Cursor position callback | Implement static `glfw_cursor_pos_callback`. Track `prev_cursor_pos_` for delta calculation. Handle first event to prevent spurious large deltas. **Acceptance:** Delta is `(0,0)` on first event. Delta correctly reflects movement on subsequent events. | Critical |
| 074 | Mouse button callback | Map GLFW button constants to `MouseButton` enum. Map `GLFW_PRESS/RELEASE` to `MouseAction` enum. Pass cursor position at time of event. **Acceptance:** Left/middle/right button presses and releases reach the handler with correct enum values. | Critical |
| 075 | Scroll callback registration | Implement static `glfw_scroll_callback`. Pass scroll offset as `Vec2`. **Acceptance:** Mouse wheel up produces positive y offset. Horizontal scroll produces non-zero x offset. | High |
| 076 | Framebuffer resize callback | Implement static `glfw_framebuffer_size_callback`. Update `fb_width_`, `fb_height_`, `win_width_`, `win_height_`. **Acceptance:** Resizing the window calls `on_resize` with new pixel dimensions. `pixel_ratio()` returns correct value for HiDPI display. | High |
| 077 | Window close callback | Implement static `glfw_close_callback`. Call `on_close` if set. **Acceptance:** Clicking the OS window close button triggers `on_close`. | High |
| 078 | Window::poll_events() | Implement by calling `glfwPollEvents()`. **Acceptance:** Input events collected between frames are dispatched via callbacks during `poll_events()`. | Critical |
| 079 | Window::swap_buffers() | Implement by calling `glfwSwapBuffers(window_)`. **Acceptance:** The back buffer is presented to the screen after `swap_buffers()` is called. | Critical |
| 080 | Window::is_open() | Return `window_ != nullptr && !glfwWindowShouldClose(window_)`. **Acceptance:** Returns false immediately after `close()` or OS close button click. | Critical |
| 081 | Window::close() | Call `glfwSetWindowShouldClose(window_, GLFW_TRUE)`. **Acceptance:** `is_open()` returns false on the next check after `close()`. | High |
| 082 | Window destructor cleanup | In destructor, call `glfwDestroyWindow(window_)` and `glfwTerminate()`. Set `window_ = nullptr`. **Acceptance:** No GLFW resource leaks reported by ASAN or Valgrind after Window destruction. | High |
| 083 | Window::is_mouse_button_down() | Call `glfwGetMouseButton()` for the given button. **Acceptance:** Returns true during a held mouse button, false immediately after release. | High |
| 084 | Window::is_key_down() | Call `glfwGetKey()` for the given keycode. **Acceptance:** Returns true while a key is physically held, false after release. | High |
| 085 | Window::time() | Return `glfwGetTime()`. **Acceptance:** Values are monotonically increasing. Two calls 100ms apart differ by approximately 0.1. | Medium |
| 086 | Window::set_cursor() | Call `glfwCreateStandardCursor(cursor_type)` and `glfwSetCursor()`. **Acceptance:** Cursor changes to crosshair when `set_cursor(GLFW_CROSSHAIR_CURSOR)` is called. | Medium |
| 087 | HiDPI pixel ratio support | Implement `pixel_ratio()` as `(float)fb_width_ / (float)win_width_`. Use fb dimensions for OpenGL viewport. **Acceptance:** Text and UI elements appear at correct physical size on a 2x Retina/HiDPI display. | High |
| 088 | VSync toggle support | Pass `vsync` parameter to `Window::create()`. Call `glfwSwapInterval(1)` for vsync, `glfwSwapInterval(0)` for uncapped. **Acceptance:** FPS with vsync is capped at refresh rate. FPS without vsync exceeds refresh rate on fast hardware. | Medium |
| 089 | Window title update method | Add `Window::set_title(const std::string&)` calling `glfwSetWindowTitle()`. **Acceptance:** Title bar updates when called, including UTF-8 Unicode characters. | Low |
| 090 | Clipboard get/set methods | Add `Window::get_clipboard()` and `Window::set_clipboard(const std::string&)` using GLFW clipboard functions. **Acceptance:** Text copied in CBuild can be pasted in other apps, and vice versa. | Medium |
| 091 | GLFW key code constants header | Create `src/core/platform/keycodes.hpp` mapping platform-independent key name constants (`KEY_A`, `KEY_DELETE`, `KEY_CTRL`, etc.) to GLFW constants. **Acceptance:** All keycodes use `KEY_*` constants rather than magic GLFW integers. | Medium |
| 092 | Modifier key tracking | Implement `Window::is_ctrl_down()`, `is_shift_down()`, `is_alt_down()` checking both left and right modifier variants. **Acceptance:** Both Ctrl keys trigger `is_ctrl_down()`. Shift + click is correctly detected. | High |
| 093 | Window focus tracking | Add `on_focus` and `on_unfocus` callbacks using `glfwSetWindowFocusCallback`. **Acceptance:** Callbacks fire when window gains/loses focus. `is_focused()` returns correct value. | Medium |
| 094 | Window minimize/restore tracking | Add `on_minimize` and `on_restore` callbacks using `glfwSetWindowIconifyCallback`. **Acceptance:** Callbacks fire when window is minimized/restored. Resume rendering only after restore. | Low |
| 095 | Cursor visible/hidden toggle | Add `Window::set_cursor_visible(bool)` using `glfwSetInputMode GLFW_CURSOR_NORMAL / GLFW_CURSOR_HIDDEN`. **Acceptance:** Cursor is hidden when called with false, visible when called with true. | Medium |
| 096 | Raw mouse input mode | Add `Window::enable_raw_mouse()` using `glfwSetInputMode GLFW_RAW_MOUSE_MOTION`. **Acceptance:** Mouse delta reports raw hardware motion without OS cursor acceleration. | Medium |
| 097 | Window icon loading | Add `Window::set_icon(const char* png_path)` using stb_image to load and `glfwSetWindowIcon` to set. **Acceptance:** Application icon appears in taskbar and window title bar. | Low |
| 098 | Multi-monitor support query | Add `Window::get_monitor_info()` returning a vector of monitor descriptions (name, resolution, refresh rate, work area). **Acceptance:** On a dual-monitor setup, two monitors are reported with correct resolutions. | Low |
| 099 | Fullscreen toggle method | Add `Window::toggle_fullscreen()` switching between windowed and fullscreen mode. Save/restore window position on toggle. **Acceptance:** Pressing F11 switches to fullscreen and back without changing window position. | Low |
| 100 | Gamepad/joystick polling | Add optional gamepad polling using `glfwGetJoystickAxes/Buttons`, dispatched via an `on_gamepad` callback. **Acceptance:** Axes and buttons from a connected gamepad are reported correctly. | Low |
| 101 | Window::frame_time() helper | Track the delta between the last two `poll_events()` calls and expose as `Window::frame_time_ms()`. **Acceptance:** `frame_time_ms()` returns approximately 16.67 at 60fps, approximately 33.33 at 30fps. | Medium |

---

## D. 2D Renderer

> The batch renderer is CBuild's GPU interface. Every visible pixel passes through here. The renderer must handle 10,000+ quads per frame at 60fps. Correctness, performance, and anti-aliasing quality are all acceptance criteria.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 102 | Shader class: compile_stage() | Implement `Shader::compile_stage()` calling `glCreateShader`, `glShaderSource`, `glCompileShader`. On failure, call `glGetShaderInfoLog` and print to stderr. **Acceptance:** A deliberate GLSL syntax error prints a readable error message. | Critical |
| 103 | Shader class: compile() | Implement `Shader::compile(vert_src, frag_src)` creating a program, attaching stages, calling `glLinkProgram`. On link failure, print `glGetProgramInfoLog`. **Acceptance:** Both shaders compile and link without errors for all built-in GLSL sources. | Critical |
| 104 | Shader uniform location cache | Implement `Shader::loc(const char*)` using an `unordered_map` to cache `glGetUniformLocation` results. **Acceptance:** Cache miss rate is 0% after the first frame. | High |
| 105 | Shader::set_ortho() method | Implement `set_ortho(name, left, right, bottom, top)` building a standard orthographic projection matrix and uploading via `glUniformMatrix4fv`. **Acceptance:** Geometry submitted in screen-pixel coordinates renders at the correct position and size. | Critical |
| 106 | Quad vertex shader: GLSL | Write `QUAD_VERT` shader taking `a_pos` (screen pixels), `a_uv`, `a_color`, `a_rect` and transforming via `u_proj` orthographic matrix. **Acceptance:** Shader compiles without errors. Positions map correctly from screen pixels to NDC. | Critical |
| 107 | Quad fragment shader: SDF rounded corners | Write `QUAD_FRAG` shader implementing `sdf_rounded_rect()` SDF function. Compute `fill_alpha` using `smoothstep` anti-aliasing with `fwidth()`. **Acceptance:** Corners are perfectly anti-aliased at any zoom. Border is uniform thickness. | Critical |
| 108 | Line vertex and fragment shaders | Write `LINE_VERT` and `LINE_FRAG` shaders for colored line segments. **Acceptance:** Lines render at correct positions with correct colors. | Critical |
| 109 | Text vertex and fragment shaders | Write `TEXT_VERT` and `TEXT_FRAG` shaders. `TEXT_FRAG` samples a greyscale atlas texture, uses the red channel as alpha coverage. **Acceptance:** Text rendered from the atlas appears crisp with correct alpha blending. | Critical |
| 110 | Renderer2D::init(): GL state setup | Enable `GL_BLEND` with `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`. Disable `GL_DEPTH_TEST`. Disable `GL_CULL_FACE`. Enable `GL_MULTISAMPLE`. **Acceptance:** Transparent widgets render correctly over the canvas background. | Critical |
| 111 | Quad VAO/VBO/EBO initialization | Create quad batch VAO, VBO, EBO. Call `glBufferData` with `GL_DYNAMIC_DRAW` for `MAX_QUADS*4` vertices and `MAX_QUADS*6` indices. Set up vertex attrib pointers. **Acceptance:** `glGetError` returns `GL_NO_ERROR` after initialization. | Critical |
| 112 | Line VAO/VBO initialization | Create line batch VAO, VBO. `glBufferData` for `MAX_LINES` vertices `GL_DYNAMIC_DRAW`. Set up 2 vertex attrib pointers. **Acceptance:** No GL errors after initialization. | Critical |
| 113 | Text VAO/VBO/EBO initialization | Create text batch VAO, VBO, EBO. Same pattern as quad batch. Set up 3 vertex attrib pointers. **Acceptance:** No GL errors. Text batch is independent from quad batch (separate VAO). | Critical |
| 114 | begin_frame(): clear and reset | Implement `begin_frame(w, h, clear_color)`. Call `glViewport(0,0,w,h)`, `glClearColor`, `glClear(GL_COLOR_BUFFER_BIT)`. Clear all batch vertex and index vectors. **Acceptance:** Each frame starts with a clean slate. | Critical |
| 115 | push_quad(): vertex submission | Implement `push_quad(rect, fill, border, corner_r, border_w)`. Push 4 `QuadVertex` entries with correct UV, color, and rect data. Push 6 index entries for 2 triangles. **Acceptance:** A single `push_quad` call produces exactly 4 vertices and 6 indices. | Critical |
| 116 | flush_quads(): GPU upload and draw | Implement `flush_quads()`. `glBindVertexArray`, `glBufferSubData` for vertices and indices. `glDrawElements(GL_TRIANGLES)`. **Acceptance:** A batch of 100 quads is drawn in a single draw call. | Critical |
| 117 | flush_lines(): GPU draw | Implement `flush_lines()`. `glBufferSubData` for line vertices. `glDrawArrays(GL_LINES)`. **Acceptance:** 50 lines are drawn in a single draw call. | Critical |
| 118 | draw_rect() public method | Implement `draw_rect(rect, fill, corner_r, border_w, border_color)`. Calls `push_quad()`. If quad batch is full, flush first. **Acceptance:** Drawing 10,000 rects causes exactly `ceil(10000/8192)` flush calls. | Critical |
| 119 | draw_rect_outline() public method | Implement `draw_rect_outline(rect, color, thickness)` as four thin `push_quad` calls for top/bottom/left/right edges. Edges must not overlap at corners. **Acceptance:** Outline renders as a clean frame with no visible corner seams. | High |
| 120 | draw_line() public method | Implement `draw_line(a, b, color, thickness)`. Push two `LineVertex` entries. Handle line batch overflow by flushing. **Acceptance:** Lines of length 0 (degenerate) are silently skipped. Long lines render correctly. | Critical |
| 121 | draw_dashed_line() public method | Implement `draw_dashed_line(a, b, color, dash_len, gap_len, thickness)`. Walk from `a` to `b`, alternating draw segments and gaps. **Acceptance:** A 200px line with dash=8 gap=4 produces exactly the expected number of segments. | High |
| 122 | draw_circle() public method | Implement `draw_circle(center, radius, color, segments, thickness)` as a poly-line loop using cos/sin for segment endpoints. **Acceptance:** A circle with 24 segments appears round. Endpoints close exactly. | High |
| 123 | draw_crosshair() public method | Implement `draw_crosshair(center, size, color, thickness)` as two perpendicular `draw_line` calls. **Acceptance:** Crosshair is perfectly centered on the given point with equal arm length in all four directions. | High |
| 124 | draw_diamond() public method | Implement `draw_diamond(center, size, fill_color, border_color)` as a rotated square quad. **Acceptance:** Diamond renders as a 45-degree rotated square with fill and border. | High |
| 125 | draw_grid() public method | Implement `draw_grid(viewport, offset, zoom, minor_step, major_step)`. Compute starting positions from pan offset modulo step size. Draw major lines with higher opacity. **Acceptance:** Grid moves correctly with canvas pan. Grid lines disappear gracefully when zoomed out too far. | Critical |
| 126 | Scissor clip push/pop | Implement `push_clip_rect()` calling `glEnable(GL_SCISSOR_TEST)` and `glScissor()`. Implement `pop_clip_rect()` restoring previous clip. **Acceptance:** Content outside the clip rect is not rendered. Nested clips work correctly. | Critical |
| 127 | Canvas-to-screen coordinate transform | Implement `canvas_to_screen(Vec2 p)` as `p * zoom + pan`. Implement `screen_to_canvas(Vec2 p)` as `(p - pan) / zoom`. **Acceptance:** Round-tripping a point through both transforms returns the original point within floating point tolerance. | Critical |
| 128 | set_view_transform() method | Store `pan_` and `zoom_` fields. Update them in `set_view_transform(pan, zoom)`. **Acceptance:** View transform changes take effect on the next draw call. | Critical |
| 129 | draw_text() stub and measure_text_width() stub | Implement `draw_text()` as a no-op stub and `measure_text_width()` returning `text.length() * font_size * 0.6f`. **Acceptance:** Stubs compile without errors. Stubs provide reasonable estimates for layout purposes. | High |
| 130 | FontAtlas class header | Design `FontAtlas` class using `stb_truetype.h`. Fields: `GLuint texture_id`, `float font_size`, array of `GlyphInfo` per character. Methods: `load_from_file()`, `bake()`, `get_glyph(uint32_t codepoint)`. **Acceptance:** Header compiles. `FontAtlas` object can be created. | High |
| 131 | FontAtlas::load_from_file() | Implement loading a `.ttf` file via `fread` into a buffer. Call `stbtt_InitFont()`. **Acceptance:** Font file loads without error. Calling on a non-existent file returns false with an error message. | High |
| 132 | FontAtlas::bake() | Implement baking using `stbtt_BakeFontBitmap()`. Create a greyscale atlas texture (512x512 or 1024x1024). Upload via `glTexImage2D` with `GL_RED` internal format. **Acceptance:** Atlas texture appears correct when rendered. | High |
| 133 | draw_text() full implementation | Replace stub with real implementation. For each character, look up glyph UV from atlas. Push a `TextVertex` quad. Advance x cursor by glyph advance width. Handle newlines. **Acceptance:** Text renders correctly for all printable ASCII characters. | Critical |
| 134 | measure_text_width() full implementation | Replace stub: accumulate glyph advance widths for each character using atlas data. Return total width in pixels. **Acceptance:** Measured width matches rendered text width within 1px for any string. | High |
| 135 | Text vertical metrics | Expose `FontAtlas::ascent()`, `descent()`, `line_height()` from stbtt metrics. **Acceptance:** Text is vertically centered correctly within widget bounds regardless of font size. | High |
| 136 | stb_easy_font fallback | Add a fallback text renderer using `stb_easy_font.h` for when no TTF font is loaded. **Acceptance:** Text renders without a font file present. Switching to TTF font shows visual improvement. | Medium |
| 137 | Glyph advance caching | Cache stbtt glyph advance widths in an array indexed by ASCII codepoint. **Acceptance:** `measure_text_width()` performance improves by at least 5x after caching. | Medium |
| 138 | Unicode multi-byte text support | Implement UTF-8 decoding in `draw_text()` to handle multi-byte codepoints. **Acceptance:** Text containing characters outside ASCII (€, ñ, 日) renders correctly if the font supports those codepoints. | Medium |
| 139 | Renderer2D::shutdown() cleanup | Implement `shutdown()` deleting all VAOs, VBOs, EBOs, and shader programs. **Acceptance:** No OpenGL resource leaks after `shutdown()`. ASAN reports no memory leaks. | High |
| 140 | Frame statistics counters | Track `draw_calls`, `quads_last_frame`, `lines_last_frame` per frame. Expose via public accessors. Reset at `begin_frame`. **Acceptance:** Stats accurately reflect GPU work submitted in the previous frame. | Medium |
| 141 | Batch overflow auto-flush | Ensure every push function checks for batch overflow before inserting. If overflow would occur, flush the batch first, then insert. **Acceptance:** Drawing more than `MAX_QUADS` quads in one frame does not corrupt memory or crash. | Critical |
| 142 | Translucent layer ordering | Flush all batches in correct order in `end_frame()`: quads first, then lines, then text. **Acceptance:** Text always renders on top of widget fills even if submitted in arbitrary order. | High |
| 143 | OpenGL debug message callback | Register `glDebugMessageCallback` in debug builds. Print source, type, severity, and message to stderr. Filter to HIGH severity only in release. **Acceptance:** A `GL_DEBUG_TYPE_ERROR` message is printed when `glDrawElements` is called with incorrect parameters. | Medium |
| 144 | Render benchmark test | Write a benchmark drawing 10,000 colored rects per frame and measure FPS. **Acceptance:** Release build achieves at least 60 FPS with 10,000 quads on a mid-range GPU. | Medium |
| 145 | MSAA resolve quality | Verify that 4x MSAA produces visibly smoother edges on widget borders vs no MSAA. **Acceptance:** Widget borders at 45-degree angles show no pixel staircase aliasing with MSAA enabled. | Low |

---

## E. Widget System

> Every interactive element in CBuild IS-A Widget. Build the base class and all concrete widget types before the canvas or builder app. The factory system enables palette-based instantiation.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 146 | Widget base class: identity fields | Implement `id_`, `type_name_`, `label_` fields. Getters `id()`, `type_name()`, `label()`. `set_label()` calling `mark_dirty()`. **Acceptance:** Widget identity is immutable after construction except for label. | Critical |
| 147 | Widget base class: geometry fields | Implement `rect_` (`Rect`), `set_position()`, `set_size()`, `set_rect()`, `move_by()`. All call `mark_dirty()`. **Acceptance:** Moving a widget updates its `rect_` and propagates dirty to parent. | Critical |
| 148 | Widget base class: style and state | Implement `style_` (`WidgetStyle`) and `state_` (`WidgetState`) fields. **Acceptance:** Modifying style via `widget.style().fill = Color::red()` changes the widget's appearance on the next draw. | Critical |
| 149 | Widget::hit_test() base implementation | Implement `hit_test(Vec2 p)` returning `visible_ && rect_.contains(p)`. **Acceptance:** Returns false for invisible widgets. Returns true for the center of a visible widget. Returns false just outside the edge. | Critical |
| 150 | Widget virtual event handlers | Define virtual `on_mouse_enter()`, `on_mouse_leave()`, `on_mouse_press()`, `on_mouse_release()`, `on_mouse_move()`, `on_scroll()`, `on_key()`, `on_char()`, `on_focus()`, `on_unfocus()`. All return bool. **Acceptance:** Calling base event handlers does not crash and returns false. | Critical |
| 151 | Widget::effective_fill() helper | Implement `effective_fill()` returning `press_fill` if pressed, `hover_fill` if hovered, `fill.with_alpha(0.5)` if disabled, else `fill`. **Acceptance:** Correct color returned for each state combination. | High |
| 152 | Widget::draw_selection() default | Draw 8 resize handle squares at the 8 snap points of `screen_rect`. Draw selection bounding box outline. **Acceptance:** 8 white squares with accent borders appear around a selected widget. | High |
| 153 | Widget::get_properties() base | Return map with: `id`, `label`, `x`, `y`, `width`, `height`, `corner_radius`, `border_width`, `font_size`, `visible`, `locked`. **Acceptance:** All property keys and string-formatted values are present in the returned map. | High |
| 154 | Widget::set_property() base | Parse `x`, `y`, `width`, `height`, `corner_radius`, `border_width`, `font_size` as float from string. Parse `visible` and `locked` as bool. **Acceptance:** `set_property('x', '100')` correctly updates `rect_.x` to `100.0f`. | High |
| 155 | Widget children management | Implement `add_child()`, `remove_child()`, `children() const`, `parent()`. `add_child` sets `child->parent_`. **Acceptance:** Adding a child sets its parent pointer. Removing a child by ID leaves other children unchanged. | High |
| 156 | Widget::find_by_id() DFS | Implement depth-first search through children. Return self if id matches. **Acceptance:** Finds a deeply nested widget. Returns nullptr for unknown ID. | High |
| 157 | Widget dirty flag propagation | `mark_dirty()` sets `dirty_ = true` and calls `parent_->mark_dirty()` if parent exists. **Acceptance:** Marking a leaf widget dirty also marks all ancestor widgets dirty. | High |
| 158 | Widget callbacks: on_click and on_value_changed | Implement `std::function<void()> on_click` and `std::function<void(string)> on_value_changed`. `on_mouse_release()` calls `on_click` if `was_pressed`. **Acceptance:** Assigning `on_click = [] { ... }` and clicking the widget triggers the lambda. | Critical |
| 159 | Widget::serialize() | Implement `serialize()` producing a JSON-like string with `type`, `id`, and all property values. **Acceptance:** A ButtonWidget serializes to a parseable string containing all its properties. | High |
| 160 | Widget::deserialize() | Implement `deserialize(props map)` calling `set_property` for each key. **Acceptance:** A widget deserialized from a serialized string has the same visual state as the original. | High |
| 161 | ButtonWidget: constructor and defaults | Implement `ButtonWidget` with correct default colors (indigo fill, white text, 8px corner radius). Default size 120x36. **Acceptance:** A freshly constructed ButtonWidget visually matches the design spec. | Critical |
| 162 | ButtonWidget::draw() | Draw rounded rect with `effective_fill()`. Center the label text horizontally and vertically. **Acceptance:** Button text is centered. Hover state shows `hover_fill`. Press state shows `press_fill`. | Critical |
| 163 | ButtonWidget hover and press visual feedback | In `on_mouse_enter`, set `state_.hovered = true` and `mark_dirty`. In `on_mouse_press`, set `state_.pressed = true`. **Acceptance:** Visual changes are immediate on hover/press without frame delay. | High |
| 164 | LabelWidget: alignment | Implement `Align::Left`, `Center`, `Right` for `LabelWidget`. Left: text at `pad_left`. Center: `(w-text_w)*0.5`. Right: `w-text_w-pad_right`. **Acceptance:** Text is correctly positioned for all three alignment modes. | High |
| 165 | LabelWidget: transparent background | Default label `fill = Color::transparent()`. Only draw background if `fill.a > 0.01`. **Acceptance:** Label over a dark panel shows the panel color through the label background. | High |
| 166 | TextInputWidget: draw() | Draw background with focused/unfocused border color. Display `value_` if non-empty, else placeholder in 40% opacity. Draw blinking cursor line when focused. **Acceptance:** All three visual states (empty, filled, focused) render correctly. | Critical |
| 167 | TextInputWidget: on_char() | Append printable ASCII characters to `value_`. Call `on_value_changed`. Return true to consume the event. **Acceptance:** Typing 'Hello' produces `value_ == 'Hello'`. `on_value_changed` fires for each character. | Critical |
| 168 | TextInputWidget: backspace handling | Handle `GLFW_KEY_BACKSPACE` by calling `value_.pop_back()` if non-empty. **Acceptance:** Holding backspace clears the field. Backspace on empty field does nothing. | High |
| 169 | TextInputWidget: text cursor position | Track `cursor_pos_` (integer index into `value_`). Arrow keys move cursor. Cursor blink uses window time. **Acceptance:** Cursor appears at correct character position. Blinking period is approximately 500ms. | High |
| 170 | TextInputWidget: selection support | Track `sel_start_`, `sel_end_`. Shift+arrow selects text. Ctrl+A selects all. Ctrl+C copies selection. Ctrl+V pastes. **Acceptance:** Selected text is highlighted. Copy/paste via OS clipboard works. | Medium |
| 171 | PanelWidget: draw() | Draw panel background. Draw title bar strip with tinted fill. Draw label text in title bar. Clip children to panel bounds. **Acceptance:** Children are clipped at panel edges. | High |
| 172 | PanelWidget: resizable borders | Add resize handle detection for panel borders. Dragging the right or bottom edge resizes the panel. **Acceptance:** Dragging panel edge resizes it. Children remain inside. | Medium |
| 173 | ChatBubbleWidget: left and right sides | Implement `Side::Left` (dark bg, `text_primary`) and `Side::Right` (accent bg, white text). `update_colors()` called in constructor and `set_side()`. **Acceptance:** Left bubble looks like 'received' message, right looks like 'sent' message. | High |
| 174 | ChatBubbleWidget: word wrap | Implement word wrapping for long text: break at word boundaries to fit within widget width. Expand widget height automatically. **Acceptance:** A 200-character message wraps correctly within a 280px wide bubble. | High |
| 175 | SliderWidget: drag interaction | Track `dragging_` state. `on_mouse_press` starts drag. `on_mouse_move` updates value if dragging. `on_mouse_release` ends drag. **Acceptance:** Dragging the thumb smoothly adjusts value between min and max. | High |
| 176 | SliderWidget: click-to-position | Clicking anywhere on the track jumps the value to that position. **Acceptance:** Clicking the left edge sets value to min. Clicking the right edge sets value to max. | Medium |
| 177 | CheckboxWidget: toggle on click | `on_mouse_press` toggles `checked_` and calls `on_value_changed('true'/'false')`. **Acceptance:** Each click toggles the checkbox state. Visual checkmark appears/disappears immediately. | High |
| 178 | DropdownWidget | Create DropdownWidget with a list of string options. Clicking opens a popup overlay. Clicking an option selects it and closes popup. **Acceptance:** Dropdown displays selected value. Selecting an option fires `on_value_changed`. | Medium |
| 179 | ImageWidget | Create ImageWidget loading a PNG/JPG via stb_image. Render as a textured quad. Support fit/fill/stretch modes. **Acceptance:** A valid PNG file loads and renders. An invalid path shows a placeholder. | Medium |
| 180 | ProgressBarWidget | Create ProgressBarWidget with `value [0,1]`. Render filled portion in accent color, empty portion in bg. **Acceptance:** `value=0.75` renders 75% filled. | Medium |
| 181 | ScrollableContainerWidget | Container with internal scroll offset. Mouse wheel adjusts scroll within content bounds. Draw children offset by scroll. Clip to container rect. **Acceptance:** Content taller than the container is accessible via scroll. | Medium |
| 182 | TabWidget | Create TabWidget with a list of tabs. Tab bar renders at top. Clicking a tab shows its children and hides others. **Acceptance:** Switching tabs correctly shows/hides content. | Medium |
| 183 | SplitPanelWidget | Panel split horizontally or vertically with a draggable divider. **Acceptance:** Dragging the divider resizes both panels proportionally. | Medium |
| 184 | CodeEditorWidget | Multi-line text editor with syntax highlighting for C++. Line numbers on the left. Horizontal and vertical scroll. **Acceptance:** C++ code is highlighted correctly. Navigation with arrow keys and page up/down works. | Low |
| 185 | ConsoleWidget | Read-only text display that appends lines and auto-scrolls to bottom. Support colored text (error in red, warning in yellow, success in green). **Acceptance:** Appending 1000 lines is fast (< 5ms). Auto-scroll shows newest content. | High |
| 186 | WidgetFactory: registration system | Implement `WidgetFactory` singleton with `register_type()` and `create()` methods. `register_builtin_widgets()` function registers all built-in types. **Acceptance:** `WidgetFactory::instance().create('Button', 'btn_1')` returns a valid `ButtonWidget`. | Critical |
| 187 | Widget::clone() | Add virtual `clone()` returning a deep copy with a new ID. Used for copy-paste on canvas. **Acceptance:** A cloned `ButtonWidget` has the same style, rect, and label as the original but a different ID. | High |
| 188 | Widget Z-order within parent | Support `bring_forward()`, `send_backward()`, `bring_to_front()`, `send_to_back()` within a parent container. **Acceptance:** A widget moved to front renders on top of siblings. | Medium |
| 189 | Widget::measure_preferred_size() | Add virtual `measure_preferred_size()` returning the widget's natural size given its content. **Acceptance:** A Button with label 'Submit' returns a preferred width close to the text width + padding. | Medium |

---

## F. Canvas and Scene Graph

> The canvas manages the infinite design surface, all widget interactions, and the undo/redo system. Correctness of drag, resize, and selection mechanics directly determines CBuild's usability.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 190 | Canvas widget storage | Implement `widgets_` as `std::vector<WidgetPtr>`. Back of vector = highest Z-order. **Acceptance:** A widget at the back of the vector renders above all others. | Critical |
| 191 | Canvas::add_widget() with undo | Implement `add_widget()` creating and executing an `AddWidgetCommand`. **Acceptance:** Adding a widget and pressing Ctrl+Z removes it. | Critical |
| 192 | Canvas::remove_widget() with undo | Implement `remove_widget()` creating and executing a `DeleteWidgetCommand` that stores the original index. **Acceptance:** Deleting and undoing preserves Z-order. | Critical |
| 193 | Canvas hit_test_canvas() back-to-front | Iterate `widgets_` in reverse calling `widget->hit_test()`. Return first hit. **Acceptance:** When two widgets overlap, clicking returns the one with higher Z-order. | Critical |
| 194 | Canvas::select() with multi-select | Implement `select(id, add=false)`. With `add=false`, clear `selected_ids_` first. With `add=true`, append. **Acceptance:** Shift+click adds to selection. Plain click replaces selection. | Critical |
| 195 | Canvas::deselect_all() | Clear `selected_ids_`. Set all `widget state_.selected = false`. Fire `on_selection_cleared`. **Acceptance:** After `deselect_all()`, no widget has selected state set. | High |
| 196 | Canvas::rubber_band_select() | Find all widgets whose rect intersects the rubber-band `canvas_rect`. Add to selection. **Acceptance:** A rubber-band covering 3 widgets selects exactly those 3. | Critical |
| 197 | Canvas pan/zoom state | Implement `pan_` (`Vec2`) and `zoom_` (float, clamped to `[zoom_min, zoom_max]`). Implement `pan_by(delta)` and `zoom_at(new_zoom, screen_pivot)`. **Acceptance:** Zooming at a pivot point keeps that screen position stationary. | Critical |
| 198 | Canvas::zoom_at() pivot math | Implement pivot math: `canvas_pivot = screen_to_canvas(screen_pivot)`. After setting zoom, recompute `pan_ = screen_pivot - canvas_pivot * new_zoom`. **Acceptance:** The point under the cursor stays fixed during scroll-to-zoom. | Critical |
| 199 | Canvas::canvas_rect_to_screen() | Implement converting a canvas-space Rect to screen-space. `tl = canvas_to_screen(r.position())`, `size = r.size() * zoom_`. **Acceptance:** A 100x50 widget at canvas (200,100) with zoom=2 maps to screen rect with size 200x100. | Critical |
| 200 | Canvas drag: start | On LMB press over a widget: store `start_canvas`, `start_widget_pos`, `drag_offset` (cursor - widget_tl), widget pointer. **Acceptance:** Drag starts from the exact click position within the widget, not from the widget's top-left. | Critical |
| 201 | Canvas drag: move and snap | On mouse move with drag active: call `snap_engine.snap_rect()` with current canvas position and drag_offset. Apply snapped position to widget. **Acceptance:** Widget follows cursor exactly when not near snap targets. Snaps to grid/objects when within threshold. | Critical |
| 202 | Canvas drag: commit to undo stack | On LMB release with drag active: compute `delta = final_pos - start_widget_pos`. If `delta.length_sq > 0.1`, push `MoveCommand`. **Acceptance:** Moving a widget and pressing Ctrl+Z returns it to original position. | Critical |
| 203 | Canvas multi-widget drag | When multiple widgets are selected, drag all selected widgets simultaneously maintaining relative positions. **Acceptance:** Moving a group of 5 widgets preserves their relative layout. | High |
| 204 | Canvas resize: handle hit-test | Implement `hit_test_handles()` checking if `screen_pos` is within hs=5px of each of the 8 resize handle positions. **Acceptance:** Clicking within 5px of a corner handle starts resize. Clicking 6px away does not. | High |
| 205 | Canvas resize: compute_resized_rect() | Implement all 8 resize handle cases. Minimum size enforced (20x16). **Acceptance:** TopLeft handle changes `x, y, w, h`. RightCenter handle changes only `w`. All 8 cases produce correct rects. | High |
| 206 | Canvas resize: commit to undo stack | On mouse release from resize: create `ResizeCommand(old_rect, new_rect)`. **Acceptance:** Resize undo restores exact original dimensions. | High |
| 207 | Canvas rubber-band: draw | Draw selection rect as translucent fill + border during rubber-band drag. **Acceptance:** Rubber-band rect is visible and updates in real time during drag. | High |
| 208 | Canvas rubber-band: apply selection | On LMB release after rubber-band: compute `canvas_rect` from the two corners and call `rubber_band_select()`. **Acceptance:** Only widgets fully or partially inside the rubber-band rect are selected. | High |
| 209 | Canvas::bring_to_front() Z-order | Implement by finding widget by ID, removing from vector, appending to back. **Acceptance:** A widget moved to front renders above all others immediately. | High |
| 210 | Canvas::send_to_back() Z-order | Implement by finding widget by ID, removing from vector, inserting at front. **Acceptance:** A widget sent to back renders below all others immediately. | High |
| 211 | Canvas::on_scroll() zoom handling | Zoom at cursor position on scroll. Factor = `1 + offset.y * 0.1`. Clamp to `[zoom_min, zoom_max]`. **Acceptance:** Scroll up zooms in. Scroll down zooms out. Zoom stays within limits. | Critical |
| 212 | Canvas pan with middle mouse | Middle mouse press starts `pan_drag_`. Mouse move updates `pan_ = start_pan + (current - start_screen)`. **Acceptance:** Panning is smooth and matches cursor movement exactly. | Critical |
| 213 | Canvas::on_key() Ctrl+Z/Y | Handle `GLFW_KEY_Z + MOD_CTRL` for undo. Handle `GLFW_KEY_Y + MOD_CTRL` for redo. **Acceptance:** Ctrl+Z undoes the last action. Ctrl+Y redoes. Repeated Ctrl+Z walks back through the full undo stack. | Critical |
| 214 | Canvas::on_key() arrow nudge | 1px nudge on plain arrow keys. 10px nudge on Shift+arrow. Create `MoveCommand` for all selected widgets. **Acceptance:** Arrow nudge moves all selected widgets. Shift+arrow moves 10px. Undo reverses nudge. | High |
| 215 | Canvas::on_key() Ctrl+A select all | Select all widgets on Ctrl+A. **Acceptance:** All canvas widgets become selected. | High |
| 216 | Canvas::on_key() Delete | Delete all selected widgets on Delete key. **Acceptance:** Deleting 3 selected widgets and Ctrl+Z restores all 3. | High |
| 217 | Undo stack MAX_UNDO enforcement | Cap `undo_stack_` at `MAX_UNDO=100`. When at capacity, remove oldest entry. **Acceptance:** After 150 operations, `undo_stack_.size() == 100`. | High |
| 218 | Redo stack clearing on new action | Any new `push_command()` clears `redo_stack_`. **Acceptance:** After undo, then a new action, Ctrl+Y does nothing (redo is empty). | High |
| 219 | Canvas::draw() full render order | Render order: grid, all widgets back-to-front (culled by viewport), snap visuals (if drag active), selection overlays, rubber-band box. **Acceptance:** All elements appear in correct order with no Z-fighting artifacts. | Critical |
| 220 | Widget viewport culling | Skip `draw()` call for widgets whose `canvas_rect_to_screen()` does not intersect the viewport. **Acceptance:** A canvas with 1000 widgets where only 50 are visible makes exactly 50 widget draw calls. | High |
| 221 | Canvas viewport clip | Call `push_clip_rect(viewport)` at start of `Canvas::draw()` and `pop_clip_rect()` at end. **Acceptance:** Widget content does not render outside the canvas panel area. | Critical |
| 222 | Canvas::find_widget() by ID | Linear search through `widgets_` and their children. Return pointer or nullptr. **Acceptance:** O(n) search finds any widget by ID. Returns nullptr for unknown ID in < 1ms for 1000 widgets. | High |
| 223 | Widget hover state update on mouse move | When no drag is active, update hover state for all widgets: call `on_mouse_enter` for newly hovered, `on_mouse_leave` for previously hovered. **Acceptance:** Hover visual feedback updates smoothly as cursor moves between widgets. | High |
| 224 | Canvas keyboard focus system | Track `focused_widget_` pointer. LMB on a TextInput gives it focus. LMB elsewhere clears focus. **Acceptance:** Only one widget has focus at a time. Tab cycles focus. | High |
| 225 | Group command for multi-widget operations | Implement `CompositeCommand` that holds multiple `ICommands` and executes/undoes them as an atomic unit. **Acceptance:** Undoing a multi-widget move undoes all widgets simultaneously. | Medium |
| 226 | Canvas properties panel callback | `on_canvas_changed` fires after any mutation (add, delete, move, resize). **Acceptance:** The builder app's dirty flag updates immediately on every canvas change. | High |

---

## G. Snap Engine

> The snap engine is CBuild's most mathematically sophisticated component. Every snap mode must work independently and in combination.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 227 | SnapConfig struct | Define `SnapConfig` with: `grid_snap_enabled`, `object_snap_enabled`, `align_track_enabled`, `dist_snap_enabled` booleans; `grid_size`, `major_grid_size`, `snap_threshold_px`, `align_threshold_px` floats. **Acceptance:** Default config enables all snap modes with threshold 10px. | Critical |
| 228 | SnapResult struct | Define `SnapResult` with: `Vec2 position`, `SnapType snap_type`, `vector<SnapTrackLine> track_lines`, `vector<Vec2> snap_points`, `bool snapped`. **Acceptance:** Default `SnapResult` has `snapped=false`. | Critical |
| 229 | SnapType enum | Define `SnapType`: `None`, `Grid`, `WidgetCorner`, `WidgetEdge`, `WidgetCenter`, `AlignH`, `AlignV`, `Distribution`. **Acceptance:** All enum values have distinct integer values. | Critical |
| 230 | SnapTrackLine struct | Define `SnapTrackLine` with: `Vec2 from`, `Vec2 to`, `SnapType type`, `bool dashed`. **Acceptance:** Struct is trivially copyable and fits in a vector. | Critical |
| 231 | SnapEngine::snap_to_grid() | Round both components to nearest multiple of `config.grid_size` using `std::round`. **Acceptance:** `Vec2(13.7, 7.2)` with `grid_size=10` returns `Vec2(10, 10)`. | Critical |
| 232 | Screen-to-canvas threshold conversion | In `snap()`, compute `thresh_canvas = config.snap_threshold_px / zoom`. **Acceptance:** At zoom=2.0, a snap candidate 6 canvas-pixels away does NOT trigger snap. A candidate 4 canvas-pixels away DOES trigger snap. | Critical |
| 233 | Grid candidate generation | `generate_grid_candidates()` computes the nearest grid node and tests 4 surrounding nodes. Only candidates within `thresh_canvas` are included. **Acceptance:** At `grid_size=10`, cursor at (13, 7) generates candidates near (10,10) and (10,0). | Critical |
| 234 | Widget snap points method | `widget_snap_points(Rect r)` returns 9 `{Vec2, SnapType}` pairs: 4 corners (`WidgetCorner`), 4 edge midpoints (`WidgetEdge`), 1 center (`WidgetCenter`). **Acceptance:** All 9 points are geometrically correct for any given rect. | Critical |
| 235 | Object snap candidate generation | `generate_object_candidates()` iterates all non-excluded, visible widgets. For each widget's 9 snap points, adds a `SnapCandidate` if within `thresh_canvas`. Excludes the dragged widget itself. **Acceptance:** Snap candidates from 3 nearby widgets are all included. The dragged widget is excluded. | Critical |
| 236 | Snap candidate priority sort | Sort candidates by priority (float, lower = higher priority). Center=0.5, Corner=0.3, Edge=0.4, Grid=1.8. Ties broken by distance. **Acceptance:** Center snap wins over corner snap when both are within threshold. | Critical |
| 237 | Best candidate selection | Pick `candidates.front()` after sorting. Set `result.position`, `snap_type`, `snapped=true`. **Acceptance:** The correct snap point is selected in all tested scenarios. | Critical |
| 238 | Object snap track line generation | For object snap candidates with non-empty `source_id`, add a `SnapTrackLine` from the snap point to the cursor. **Acceptance:** A line connects from the snapped widget's snap point to the dragging widget. | High |
| 239 | Alignment tracking: horizontal lock | `generate_alignment_tracks()` tests `|cursor.y - ref_y| < align_thresh` for `ref_y` in `{widget.top, widget.center.y, widget.bottom}`. On match: set`result.position.y = ref_y`. Add horizontal track line spanning full canvas width. **Acceptance:** Dragging near another widget's top edge snaps Y and shows a horizontal dashed cyan line. | Critical |
| 240 | Alignment tracking: vertical lock | Same as horizontal but for X axis against `{widget.left, widget.center.x, widget.right}`. **Acceptance:** Dragging near another widget's left edge snaps X and shows a vertical dashed cyan line. | Critical |
| 241 | Simultaneous H+V alignment lock | Both horizontal and vertical alignment can fire simultaneously. Both are added to `track_lines`. **Acceptance:** Moving a widget near the corner of another shows both a horizontal and vertical guide line simultaneously. | High |
| 242 | snap_rect() drag offset handling | Implement `snap_rect(cursor, drag_offset, ...)` computing `widget_tl = cursor - drag_offset`. Snap `widget_tl`. Also try snapping widget center. Return the snap that yields the smallest distance. **Acceptance:** Dragging from inside the widget snaps the widget's top-left corner to grid, not the cursor. | Critical |
| 243 | Snap priority: object over grid | Object snap candidates (priority < 1.0) beat grid candidates (priority = 1.8). **Acceptance:** When a grid node and a widget corner are both within threshold, the widget corner wins. | Critical |
| 244 | Snap disabled flags | When `config.grid_snap_enabled = false`, `generate_grid_candidates()` returns immediately. **Acceptance:** Disabling grid snap via the toolbar does not call any grid math. | High |
| 245 | make_h_track() helper | Return `SnapTrackLine` from `(0, y)` to `(canvas_size.x, y)` with given type and `dashed=true`. **Acceptance:** Track line spans full canvas width regardless of current viewport. | High |
| 246 | make_v_track() helper | Return `SnapTrackLine` from `(x, 0)` to `(x, canvas_size.y)` with given type and `dashed=true`. **Acceptance:** Track line spans full canvas height. | High |
| 247 | SnapEngine::draw() track lines | For each `track_line` in result, convert `from/to` via `renderer.canvas_to_screen()`. Choose color: `snap_track` for `AlignH/AlignV`, `snap_grid` for others. Use `draw_dashed_line` for dashed tracks. **Acceptance:** Cyan dashed lines appear exactly on the alignment axis. | Critical |
| 248 | SnapEngine::draw() snap markers: center | For `WidgetCenter` snap: draw a circle of radius 7 and crosshair in `snap_center` (green). **Acceptance:** Green circle+crosshair appears at the snapped center point. | High |
| 249 | SnapEngine::draw() snap markers: corner | For `WidgetCorner` snap: draw a square outline in `snap_point` (orange). **Acceptance:** Orange square appears at the corner snap point. | High |
| 250 | SnapEngine::draw() snap markers: edge | For `WidgetEdge` snap: draw a diamond in `snap_point` (orange). **Acceptance:** Orange diamond appears at the edge midpoint snap point. | High |
| 251 | SnapEngine::draw() snap markers: grid | For Grid snap: draw a small crosshair in `snap_track` (cyan) at reduced opacity. **Acceptance:** Small cyan crosshair appears at the grid node. | High |
| 252 | SnapEngine::draw() snap markers: align | For `AlignH/AlignV`: draw a circle in `snap_track` (cyan). **Acceptance:** Cyan circle appears at the alignment intersection point. | High |
| 253 | Polar tracking: angle increment | Implement optional polar tracking mode. When enabled, constrain drag movement to angular increments (default 45°). **Acceptance:** With polar tracking, a widget dragged diagonally snaps to the nearest 45° angle. | Medium |
| 254 | Distribution snap detection | Detect when the dragged widget would be equally spaced between two other widgets. Add `SnapType::Distribution` candidate at the equal-spacing position. **Acceptance:** Dragging a widget between two others with gap=20px snaps when the dragged widget would be at gap=20px from each. | Low |
| 255 | Distribution visualization arrows | When Distribution snap is active, draw arrows showing the equal spacing between widgets. **Acceptance:** Small arrows with equal spacing labels appear between the three widgets. | Low |
| 256 | Snap threshold configuration UI | In the snap settings panel, add sliders for `snap_threshold_px` and `align_threshold_px`. Changes apply immediately. **Acceptance:** Reducing threshold to 2px makes snapping much harder to trigger. | Medium |
| 257 | Snap logging for debug | In debug builds, log snap events to stdout: candidate type, source widget ID, snap distance. **Acceptance:** Each snap event produces a single debug line with enough info to diagnose snap behavior. | Low |
| 258 | Snap performance optimization | Profile `snap()` with 1000 widgets on canvas. Optimize by early-exit if candidate is closer than grid snap threshold. **Acceptance:** `snap()` for 1000 widgets completes in under 0.5ms per frame. | Medium |
| 259 | Snap history for tracking acquisition | Implement acquisition state: hovering near an object snap point for >= 500ms marks it as acquired. **Acceptance:** Hovering near a corner for 500ms acquires it. Moving away shows the tracking ray. | Low |
| 260 | Two-point tracking intersection | With two acquired points, compute the virtual intersection of their tracking rays using `geom::line_intersection()`. Snap to the intersection point. **Acceptance:** Acquiring corners A and B, then moving to the intersection snaps to that virtual point. | Low |
| 261 | Snap settings persistence | Save snap configuration in the project file. Restore on load. **Acceptance:** Project loads with the same snap settings that were saved. | Medium |

---

## H. Builder App and IDE Shell

> The builder app is the three-panel IDE shell that integrates every subsystem. Panel layout, input routing, inspector editing, and project persistence are all handled here.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 262 | BuilderApp::init() wiring | Wire all window callbacks to BuilderApp handlers. Wire canvas callbacks to inspector and hierarchy update. Start backend discovery. **Acceptance:** Input events reach the correct handlers. Canvas selection updates inspector immediately. | Critical |
| 263 | layout_panels() on resize | Recompute all panel Rect values from window dimensions. Rebuild toolbar buttons. **Acceptance:** Resizing the window correctly repositions all panels with no gaps or overlaps. | Critical |
| 264 | Toolbar: New/Save/Load buttons | Implement button click handlers calling `new_project()`, `save_project()`, `load_project()`. **Acceptance:** Clicking New clears canvas. Save writes project file. Load reads and reconstructs widgets. | Critical |
| 265 | Toolbar: Undo/Redo buttons | Connect to `canvas_.undo()` and `canvas_.redo()`. Show buttons as grayed out when stack is empty. **Acceptance:** Undo/Redo buttons work identically to Ctrl+Z/Ctrl+Y. | High |
| 266 | Toolbar: snap toggle buttons | Implement active toggle state for Grid/OSnap/Align buttons. Toggle boolean and update `canvas_.snap_engine.config`. **Acceptance:** Button visually shows active/inactive state. Toggling off immediately disables that snap mode. | High |
| 267 | Toolbar: zoom level display | Display current zoom as percentage string in toolbar right section. **Acceptance:** Zoom shows '150%' when canvas zoom is 1.5. Updates every frame. | Medium |
| 268 | Toolbar: project name and dirty indicator | Display `project_name + ' •'` when dirty. **Acceptance:** Saving clears the dot. Any canvas change adds the dot. | Medium |
| 269 | draw_palette_panel() | Draw panel background and border. Draw 'Components' section header. Draw each palette item with category headers, color swatch, and display name. **Acceptance:** All items are visible and correctly labeled. | Critical |
| 270 | Palette hover detection | In `handle_mouse_move()`, iterate palette items testing `contains(pos)`. Set `palette_hovered_` index. **Acceptance:** Hovering over a palette item highlights its row background. | High |
| 271 | Palette drag initiation | On LMB press over a hovered palette item, set `palette_drag_.active = true` and copy the palette item. **Acceptance:** Click-and-hold on a palette item starts a drag. Click-and-release (no drag) does not place a widget. | Critical |
| 272 | Palette drag ghost rendering | `draw_palette_drag_ghost()` draws a semi-transparent rect with the widget name centered, following the cursor. **Acceptance:** Ghost is always centered on cursor. Ghost is visually distinct from placed widgets. | High |
| 273 | Palette drop on canvas | On LMB release with `palette_drag_.active` and cursor over `canvas_rect`: convert to canvas coords, snap to grid, create widget via factory, add to canvas. **Acceptance:** Widget appears at the snapped drop position. Drag that doesn't reach canvas cancels. | Critical |
| 274 | Palette item drag threshold | Only initiate palette drag after cursor moves more than 4px from the click point (hysteresis). **Acceptance:** Clicking a palette item without dragging does NOT start a drag. | High |
| 275 | draw_inspector_panel() header | Draw panel background and border. Show widget type and ID when a widget is selected. Show 'Nothing selected' text otherwise. **Acceptance:** Inspector correctly reflects selection state. | High |
| 276 | Inspector field rendering loop | Iterate `inspector_fields_`. For each field: draw label, draw value field rect, draw value text. Apply scroll offset. **Acceptance:** All properties for a ButtonWidget are visible in the inspector. | Critical |
| 277 | Inspector field click-to-edit | `handle_inspector_click()` computes which field row was clicked. Sets `inspector_editing_idx_` and copies value to `edit_buffer`. **Acceptance:** Clicking a field highlights it for editing. | Critical |
| 278 | Inspector text editing: char input | `handle_char()` appends printable chars to `edit_buffer` when `inspector_editing_idx_ >= 0`. **Acceptance:** Typing in an active inspector field updates the edit buffer in real time. | Critical |
| 279 | Inspector text editing: Enter commit | `handle_key()` on Enter: copy `edit_buffer` to `field.value`, call `selected_widget_->set_property(key, value)`, call `rebuild_inspector_fields()`. **Acceptance:** Pressing Enter applies the property change to the widget immediately. | Critical |
| 280 | Inspector text editing: Backspace | `handle_key()` on Backspace when editing: `pop_back()` from `edit_buffer`. **Acceptance:** Backspace deletes characters from the edit buffer one at a time. | High |
| 281 | Inspector text editing: Escape cancel | `handle_key()` on Escape when editing: clear `inspector_editing_idx_` without applying change. **Acceptance:** Pressing Escape discards the edit and shows original value. | High |
| 282 | Inspector scroll | `handle_scroll()` on `inspector_rect_`: adjust `inspector_scroll_` clamped to `[0, max_scroll]`. **Acceptance:** Scrolling in the inspector scrolls the field list when it exceeds panel height. | High |
| 283 | Inspector color picker fields | For properties with 'color' in the name, show a small colored swatch next to the value. Clicking it opens a color picker overlay. **Acceptance:** Fill color field shows the current fill color as a swatch. | Medium |
| 284 | draw_backend_panel() | When nothing is selected, inspector shows backend status: Ollama connection indicator, model name, Docker status. 'Manage Docker...' button. **Acceptance:** Backend panel shows correct connection state. | High |
| 285 | Ollama status ping | On startup, call `backend_.ollama.ping_async()`. On response, update `ollama_status_` via `MainThreadDispatcher`. **Acceptance:** 'Ollama: Connected' shows within 2 seconds of startup if Ollama is running. | High |
| 286 | draw_hierarchy_panel() | Draw panel background. For each widget, draw a card showing type tag, widget ID, and visibility dot. Highlight selected widgets. **Acceptance:** All canvas widgets appear in hierarchy. Selected widget is highlighted. | High |
| 287 | Hierarchy click-to-select | `handle_mouse_button()` on `hierarchy_rect_`: compute which widget card was clicked. Call `canvas_.select(id)`. **Acceptance:** Clicking a widget in the hierarchy selects it on the canvas and updates the inspector. | High |
| 288 | draw_status_bar() | Draw status bar background. Display: widget count, canvas pan position, Ollama status, draw call stats. **Acceptance:** All status items update in real time every frame. | Medium |
| 289 | BuilderApp::frame() main loop body | Call `backend_.update()` (flush deferred callbacks). Draw all panels in correct order. **Acceptance:** All panels render each frame without visible tearing or order artifacts. | Critical |
| 290 | project_dirty_ tracking | Set `project_dirty_ = true` on any `canvas_changed` callback. Clear on save. **Acceptance:** `•` indicator appears after any canvas change. Saving clears it. | High |
| 291 | BuilderApp::save_project() | Write project file: header with `project_name`, `ollama_model`, `ollama_host`. Then `[widget]` sections for each widget with all properties as `key=value` pairs. **Acceptance:** Saved file is valid UTF-8 text readable in any text editor. | Critical |
| 292 | BuilderApp::load_project() | Parse project file: extract header fields. On `[widget]` section, collect properties, create widget via factory, call `deserialize()`. **Acceptance:** A saved and loaded project is visually identical to the original. | Critical |
| 293 | IdGen: unique ID generation | Implement `IdGen::next(type)` returning `type_lowercased + '_' + incrementing counter` per type. Thread-safe. **Acceptance:** 100 calls to `IdGen::next('Button')` produce `btn_1` through `btn_100` with no duplicates. | High |
| 294 | Widget type search in palette | Add a TextInput search bar at the top of the palette. Filter visible items to those whose `display_name` contains the search string (case-insensitive). **Acceptance:** Typing 'but' shows only 'Button'. Clearing search shows all items. | Medium |
| 295 | Canvas zoom-to-fit button | Add a 'Fit' toolbar button that adjusts pan/zoom to fit all canvas widgets within the viewport. **Acceptance:** Clicking Fit shows all widgets at maximum possible zoom within the viewport. | Medium |
| 296 | Canvas zoom-to-selection button | Add a 'Fit Selection' button that zooms to fit only the selected widgets. **Acceptance:** With 3 widgets selected, Fit Selection zooms to show those 3 widgets filling the viewport. | Low |
| 297 | Canvas reset view button | Add a '100%' toolbar button setting `zoom=1` and centering the canvas origin. **Acceptance:** Clicking 100% resets to 1:1 zoom with canvas origin at viewport center. | Medium |
| 298 | Keyboard shortcut guide overlay | Press '?' to show/hide a keyboard shortcut reference overlay. **Acceptance:** The overlay shows all shortcuts with their descriptions. Pressing Escape or '?' dismisses it. | Low |
| 299 | Recent projects menu | Store last 5 project file paths in `~/.cbuild/recent.txt`. Show in a 'Recent' submenu from the toolbar. **Acceptance:** Recent projects are listed. Clicking one loads it. | Low |
| 300 | Auto-save functionality | Auto-save to a temp file every 5 minutes if `project_dirty_`. On startup, detect and offer to recover. **Acceptance:** Killing CBuild and relaunching offers to restore the unsaved work. | Low |

---

## I. Backend Integration Layer

> CBuild's backend layer handles connections to every type of external service: Ollama LLMs, Docker containers, REST APIs, databases, serial ports, QEMU, GDB, and custom processes.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 301 | HttpClient::post() via libcurl | Implement synchronous POST using `curl_easy`. Set URL, POSTFIELDS, Content-Type header, write callback, timeout. Return `HttpResponse`. **Acceptance:** POST to a test HTTP server receives correct status code and body. | Critical |
| 302 | HttpClient::get() via libcurl | Implement synchronous GET using `curl_easy`. **Acceptance:** GET to `http://localhost:11434/api/tags` returns correct JSON. | Critical |
| 303 | CURL global init/cleanup | Call `curl_global_init(CURL_GLOBAL_ALL)` in `HttpClient` constructor. `curl_global_cleanup()` in destructor. **Acceptance:** No memory leaks from CURL reported by Valgrind. | High |
| 304 | json_simple::escape() | Escape: `"` to `\"`, `\` to `\\`, `\n` to `\n`, `\r` to `\r`, `\t` to `\t`. **Acceptance:** `escape('hello "world"') == 'hello \"world\"'`. | High |
| 305 | json_simple::extract_string() | Find `'"key":"'`, advance past it, copy chars until unescaped closing quote. Handle escape sequences. **Acceptance:** Correctly extracts `'hello world'` from `'{"content":"hello world","done":false}'`. | Critical |
| 306 | OllamaClient::build_chat_json() | Build JSON string: `{"model":"...","stream":false/true,"messages":[...]}`. Use `json_simple::escape` for all string values. **Acceptance:** Output is valid JSON verified by parsing with nlohmann/json. | Critical |
| 307 | OllamaClient::chat_async() | Spin a detached `std::thread`. Build JSON, call `http.post(host + '/api/chat', body, 120000)`. Extract content field. Call `on_done` or `on_error`. **Acceptance:** Response arrives via `on_done` callback. Thread does not block main thread. | Critical |
| 308 | OllamaClient::generate_sync() | Build `{"model":...,"prompt":...,"stream":false}` payload. Call `http.post('/api/generate')`. Return `HttpResponse`. **Acceptance:** Synchronous call returns completed response. Suitable for use in worker threads. | High |
| 309 | OllamaClient::list_models_async() | GET `/api/tags` in a worker thread. Scan body for `'"name":"'` patterns. Return vector of model name strings via callback. **Acceptance:** Returns all pulled models. Names match `ollama list` output. | High |
| 310 | OllamaClient::ping_async() | GET `/api/tags` with 3000ms timeout. Call `cb(true)` if ok, `cb(false)` otherwise. **Acceptance:** `cb(true)` within 1 second if Ollama is running. `cb(false)` within 3 seconds if Ollama is down. | High |
| 311 | MainThreadDispatcher::post() | Lock `mutex_`, push task to `queue_`. Thread-safe: multiple worker threads may call `post()` simultaneously. **Acceptance:** 10 worker threads posting simultaneously, all callbacks execute without data races (verified with ThreadSanitizer). | Critical |
| 312 | MainThreadDispatcher::flush() | Swap `queue_` with a local queue under the lock. Execute all tasks from the local queue. **Acceptance:** Tasks execute on the calling (main) thread. `flush()` while a worker is posting does not deadlock. | Critical |
| 313 | DockerService::start_async() | Build `docker run` command from Config struct. Execute via `popen()`. Detect 'Error' in output for failure. **Acceptance:** Starting forge-ollama container produces the container ID. Starting an invalid image produces an error callback. | Critical |
| 314 | DockerService::stop_async() | Execute `docker stop container_name` via `popen()`. **Acceptance:** Stopping a running container produces `ok=true`. Stopping a non-existent container produces `ok=false`. | High |
| 315 | DockerService::status_async() | Execute `docker inspect --format='{{.State.Running}}'` via `popen()`. Parse 'true'/'false'. **Acceptance:** Returns true for a running container, false for stopped or non-existent. | High |
| 316 | DockerService::ensure_network() | Execute `docker network create --driver bridge forge-net` via `system()`. Ignore error if network already exists. **Acceptance:** Calling twice does not fail. Network appears in `docker network ls`. | High |
| 317 | BackendManager::ensure_ollama_running() | Check docker status. If not running, start with correct Config. Post result to dispatcher. **Acceptance:** On first launch, Ollama container is started automatically. On subsequent launches, it detects it's already running. | Critical |
| 318 | BackendManager::chat() | Call `ollama.chat_async()` with messages. On response, dispatch via `MainThreadDispatcher` to call `on_response` on main thread. **Acceptance:** Response arrives on the main thread. UI can be updated directly in `on_response`. | Critical |
| 319 | BackendManager::update() flush | Call `dispatcher.flush()`. Called every frame. **Acceptance:** Backend responses appear in the UI within one frame of arriving from the worker thread. | Critical |
| 320 | Docker log streaming | Implement `DockerService::stream_logs(container_name, on_line_callback)`. Open `docker logs -f` via `popen()` in a worker thread. **Acceptance:** Log lines from a running container arrive as callbacks in real time. | High |
| 321 | Docker exec implementation | Implement `DockerService::exec(container_name, command, on_done)`. Run `docker exec container_name command` via `popen()`. **Acceptance:** `exec('forge-ollama', 'ollama list')` returns model list. | High |
| 322 | Unix socket raw connection | Implement a `RawSocketClient` opening `AF_UNIX` socket to `/var/run/docker.sock`. Build a raw HTTP/1.1 GET request. **Acceptance:** GET `/v1.47/containers/json` returns the running containers list as JSON without using libcurl. | High |
| 323 | Chunked HTTP parser state machine | Implement `ChunkedParser` FSM with states: `READ_SIZE`, `READ_DATA`, `READ_TRAILER`. Parse hex chunk size, accumulate data, consume CRLF. **Acceptance:** Correctly reassembles a known chunked response. Handles chunks split across multiple `read()` calls. | High |
| 324 | Process manager: spawn child process | Implement `ProcessManager::spawn(command, args)` using `popen()` on Linux or `CreateProcess` on Windows. **Acceptance:** Spawning `echo hello` captures `hello` in stdout. | High |
| 325 | Process manager: stdout streaming | `ProcessManager::read_line()` reads one line from the child's stdout. Non-blocking with `select()`. Returns `std::optional<string>`. **Acceptance:** Spawning a process that prints 5 lines reads all 5 lines. Returns `nullopt` when no data is available. | High |
| 326 | Process manager: terminate | `ProcessManager::terminate()` kills the child process. Cleans up the `popen FILE*`. **Acceptance:** After `terminate()`, the child process no longer appears in `ps`. No zombie processes. | High |
| 327 | QEMU process manager integration | Add `QEMUManager` class using `ProcessManager` to spawn `qemu-system-x86_64` with configurable args. Connect serial output to a `ConsoleWidget`. **Acceptance:** Launching QEMU from CBuild shows boot output in the console widget. | High |
| 328 | QEMU monitor socket connection | Connect to QEMU's `-monitor unix:/tmp/qemu-monitor.sock` via `AF_UNIX` socket. Send HMP commands. **Acceptance:** Sending `info registers` to QEMU monitor returns current register state. | High |
| 329 | GDB/MI protocol connection | Connect via TCP to QEMU's `-s` port 1234. Send GDB/MI commands. Parse MI response format. **Acceptance:** Register values from a running QEMU kernel are displayed correctly. | Medium |
| 330 | WebSocket backend support | Implement a lightweight WebSocket client (handshake + frame encoding/decoding). **Acceptance:** Connects to a `ws://` server, sends a message, receives and decodes a response frame. | Medium |
| 331 | REST API generic backend connector | In `BackendManager`, add a `GenericRestBackend` configurable with a base URL and API key. **Acceptance:** Configuring it for httpbin.org and calling `get('/get')` returns the correct JSON. | High |
| 332 | Serial port backend connector | Implement `SerialBackend` connecting to `/dev/ttyUSB0` (Linux) or `COM1` (Windows). Configure baud rate, parity, stop bits. **Acceptance:** Connecting to a real UART device and sending `AT` receives correct response. | Medium |
| 333 | PostgreSQL backend connector | Implement a PostgreSQL connector using the libpq C library. Provides `execute_query()` returning rows as `vector<map<string,string>>`. **Acceptance:** Querying a local PostgreSQL database returns correct row data in a table widget. | Medium |
| 334 | Redis backend connector | Implement Redis RESP protocol client over TCP. Support GET, SET, HGETALL, SUBSCRIBE, PUBLISH commands. **Acceptance:** SET key value followed by GET key returns the same value. | Medium |
| 335 | Backend service registry | Implement `ServiceRegistry` mapping service names to backend connector instances. **Acceptance:** Registering two services and retrieving by name returns the correct connector. | Medium |
| 336 | Backend health check polling | Add periodic health checks (every 30 seconds) for all registered backends. Update UI status indicators. **Acceptance:** If Ollama goes offline, the status indicator changes within 30 seconds. | Medium |
| 337 | Backend configuration UI | In the backend panel, allow adding new backend connections: choose type, configure URL/port, test connection, save to project file. **Acceptance:** Adding a new REST backend and testing shows 'Connected' if the URL is reachable. | Medium |
| 338 | Ollama model management UI | Add a model manager panel: list installed models, button to pull a new model (showing download progress), button to delete a model. **Acceptance:** Pulling llama3 shows download progress. After pulling, it appears in the model list. | Medium |
| 339 | Docker image management UI | Add a Docker images panel listing local images with size, tag, and creation date. Button to pull new images. **Acceptance:** List shows all images from `docker images`. Pulling a new image shows progress. | Low |
| 340 | Custom backend template generator | When user adds a 'Custom Backend', generate starter code in the selected language with a health endpoint and example chat endpoint. **Acceptance:** Generated Node.js backend runs with `node server.js` and responds to `/health`. | Low |
| 341 | Kubernetes backend connector | Connect to `kubectl proxy` (localhost:8001). List pods, get logs, stream pod log. **Acceptance:** Listing pods returns the same output as `kubectl get pods`. | Low |
| 342 | SSH tunnel backend support | Allow backends to be accessed through an SSH tunnel. Start `ssh -L` forwarding in a child process. **Acceptance:** A backend running on a remote server is accessible via SSH tunnel configuration. | Low |
| 343 | Backend latency measurement | Track request/response time for each backend call. Display p50/p95 latency in the backend panel. **Acceptance:** Latency values update after each API call. | Low |

---

## J. Static Linking and Distribution

> A zero-dependency binary is a core requirement. Every task in this section contributes to CBuild's guarantee of running on any target machine without installation.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 344 | musl-libc static build target on Linux | Add CMake option `CBUILD_STATIC_MUSL`. When ON, configure compilation with `musl-gcc` wrapper or `zig cc --target x86_64-linux-musl`. Add `-static -static-libgcc -static-libstdc++` flags. **Acceptance:** `ldd` on the resulting binary shows 'not a dynamic executable'. | High |
| 345 | musl build environment setup script | Write `scripts/setup_musl.sh` that installs `musl-tools` on Ubuntu/Debian and tests compilation. **Acceptance:** Running the script and then `cmake -DCBUILD_STATIC_MUSL=ON` produces a static binary. | High |
| 346 | Windows /MT static runtime linking | For MSVC builds, replace `/MD` with `/MT` in compiler flags. Verify all dependencies are also built with `/MT`. **Acceptance:** The Windows binary has no dependency on `VCRUNTIME*.dll` or `UCRTBASE.dll` (verified with `dumpbin /dependents`). | High |
| 347 | libcurl static build on Linux | Add CMake option to link libcurl statically (`-Wl,-Bstatic -lcurl -Wl,-Bdynamic`). **Acceptance:** `curl_easy_init()` works in the static binary. `ldd` shows libcurl is not a dynamic dependency. | High |
| 348 | GLFW static build | GLFW is already built statically via FetchContent. Verify no shared library dependency is introduced. **Acceptance:** `ldd` on the binary shows no `libglfw` entry. | High |
| 349 | Binary size optimization | Add `-Os -ffunction-sections -fdata-sections -Wl,--gc-sections` for size-optimized builds. **Acceptance:** Size-optimized binary is at least 20% smaller than `-O3` binary. | Low |
| 350 | LTO (Link-Time Optimization) | Add `-flto` for GCC/Clang. `-GL` for MSVC. **Acceptance:** LTO build produces a binary that is measurably faster on the snap engine benchmark. | Low |
| 351 | Strip debug symbols for release | Add `CMAKE_STRIP` command in CPack or as a post-build step for release distributions. **Acceptance:** Released binary has no debug symbol section (verified with `nm --debug-syms`). | Low |
| 352 | Binary compatibility test: Ubuntu 18.04 | Test the musl static binary on Ubuntu 18.04. **Acceptance:** Binary runs correctly without any `GLIBC_X.Y not found` errors. | High |
| 353 | Binary compatibility test: Alpine Linux | Test on Alpine Linux (musl-native). **Acceptance:** Binary runs correctly. | High |
| 354 | Binary compatibility test: Windows 10 clean VM | Test the `/MT` binary on a clean Windows 10 VM with no Visual C++ redistributables installed. **Acceptance:** Binary runs correctly without any DLL missing errors. | High |
| 355 | Binary compatibility test: macOS 12 | Test on macOS 12 Monterey. Verify code signing situation. **Acceptance:** Binary runs without 'damaged app' error after: `xattr -d com.apple.quarantine CBuild`. | Medium |
| 356 | AppImage creation automation | Create `scripts/make_appimage.sh` using linuxdeploy. Bundle CBuild binary and assets/fonts into an AppImage. **Acceptance:** Running `CBuild-x86_64.AppImage` on Ubuntu 20.04 with no dependencies launches CBuild. | Medium |
| 357 | Self-update mechanism | Implement version checking against a GitHub releases API endpoint. If a newer version is available, offer to download and replace the binary. **Acceptance:** Version check completes within 5 seconds. Binary replacement executes correctly. | Low |
| 358 | Installation-free operation | Verify that CBuild works correctly when run from any directory. Project files and config stored in `~/.cbuild/`. **Acceptance:** Running CBuild from `/tmp` as a read-only binary works correctly. | High |

---

## K. OS Development Specialist Tools

> These tools make CBuild indispensable for OS development work specifically. Each widget is a specialized inspector, decoder, or management interface for hardware and kernel data structures.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 359 | HexViewerWidget | Create `HexViewerWidget` reading binary data. Display in 16-columns-per-row format with hex values and ASCII representation. Click on a byte to select and highlight it. **Acceptance:** A 512-byte boot sector displays with correct hex values and visible ASCII printable chars. | High |
| 360 | GDT decoder widget | Create a widget that takes 8-byte GDT entries as input. Decodes and displays: base address, limit, DPL, type, granularity, size bits, present bit. Highlights invalid entries in red. **Acceptance:** Correctly decodes a standard flat 32-bit code segment descriptor. | High |
| 361 | IDT decoder widget | Create a widget that takes 8-byte (32-bit) or 16-byte (64-bit) IDT gate descriptors. Decodes: offset, segment selector, type, DPL, present. **Acceptance:** Correctly decodes a standard ISR gate descriptor. | High |
| 362 | Register display widget | Create a `RegisterDisplayWidget` showing x86-64 register values: RAX, RBX, RCX, RDX, RSI, RDI, RSP, RBP, RIP, RFLAGS, CS, DS, ES, FS, GS, SS. Values updated by GDB/MI backend. **Acceptance:** Values update correctly after each GDB step command. | High |
| 363 | QEMU launch configuration widget | Create a PanelWidget for QEMU configuration: machine type, memory (MB), CPU count, boot drive, additional flags. 'Launch' button starts QEMU via `QEMUManager`. **Acceptance:** Launch button starts QEMU with the configured parameters. | High |
| 364 | Serial console widget | Create a `ConsoleWidget` connected to QEMU's `-serial tcp:localhost:4555`. Read lines and append to display. Text input at bottom sends to serial. **Acceptance:** Boot output from a kernel using 16550A UART appears in the console. | High |
| 365 | x86 memory map visualizer | Parse a multiboot2 memory map. Display a color-coded bar chart: BIOS reserved in red, kernel in blue, ACPI in yellow, free in green. **Acceptance:** All memory regions are displayed with correct addresses and types. | Medium |
| 366 | Page table walker widget | Accept a PML4 base address and a virtual address. Walk the 4-level page table using GDB memory reads via the GDB/MI backend. Display PML4E, PDPTE, PDE, PTE values. **Acceptance:** Correctly walks the identity mapping in a known QEMU kernel. | Medium |
| 367 | USB descriptor inspector widget | Accept raw descriptor bytes (hex input). Parse Device Descriptor, Configuration Descriptor, Interface Descriptors, Endpoint Descriptors per USB 3.2 spec. **Acceptance:** Correctly parses a known USB HID device descriptor. | Medium |
| 368 | PCI device browser widget | Connect to QEMU monitor. Send `info pci`. Parse output. Display PCI devices in a tree: Bus/Device/Function, Vendor ID, Device ID, BAR addresses, interrupt line. **Acceptance:** All QEMU virtual PCI devices appear with correct IDs. | Medium |
| 369 | Cross-compiler build panel | Panel with target selector (i686-elf, x86_64-elf, aarch64-elf). 'Build' button runs the cross-compiler inside a Docker container. **Acceptance:** Building a simple C file for i686-elf shows compiler output and exit code. | High |
| 370 | GRUB ISO builder panel | Panel for building a bootable ISO: source directory path, grub.cfg location, output ISO name. 'Build ISO' button runs `grub-mkrescue` inside Docker. **Acceptance:** Built ISO can be booted in QEMU. | High |
| 371 | Disk image management panel | Create/inspect/mount disk images. Buttons for: create raw image (dd), create partition table (fdisk), format filesystem (mkfs). **Acceptance:** Create a 64MB raw image, create a FAT32 partition, mount it via loopback. | Medium |
| 372 | QEMU snapshot management | Connect to QEMU monitor. List snapshots, create snapshot (savevm), load snapshot (loadvm), delete snapshot (delvm). **Acceptance:** Saving a snapshot and loading it restores the VM to the saved state. | Medium |
| 373 | Kernel symbol lookup widget | Load a `System.map` or nm output file. Accept a hex address. Look up and display the nearest symbol. **Acceptance:** Address `0xC0100000` correctly maps to `startup_32` in a known kernel. | Medium |
| 374 | ACPI table viewer widget | Accept raw ACPI table bytes. Decode header and MADT structures (Local APIC, I/O APIC, interrupt source overrides). **Acceptance:** Correctly decodes the MADT from a QEMU dump. | Low |
| 375 | Stack trace decoder widget | Accept a list of return addresses. Look up each in the symbol table. Display decoded stack trace. **Acceptance:** A known 5-frame stack trace displays with correct function names. | Low |
| 376 | Linker script visualizer | Parse a GCC linker script (`.ld` file). Display the section layout as a visual memory map: `.text`, `.data`, `.bss`, `.rodata` at their specified virtual and load addresses. **Acceptance:** Displays section addresses from a simple kernel linker script. | Low |
| 377 | Interrupt frequency monitor | Connect to a debug counter in the kernel (via GDB shared memory). Display a bar chart of interrupts per vector number. Refresh at 2Hz. **Acceptance:** Timer interrupt shows high frequency. Keyboard shows counts on keypress. | Low |
| 378 | ELF binary inspector widget | Load an ELF file. Display: ELF header fields, section table, program header table. **Acceptance:** Loading a compiled kernel ELF shows correct section addresses matching the linker script. | Medium |

---

## L. Advanced UI Features

> These tasks cover the higher-level UI polish: context menus, alignment tools, copy/paste, grouping, theme system, rulers, guidelines, and all the interaction details that distinguish a professional tool from a prototype.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 379 | Theme system: light mode | Implement a light color theme toggled via a settings panel. All `Color` design system values switch to light equivalents. **Acceptance:** Light mode renders comfortably with sufficient contrast for all text/background combinations. | Medium |
| 380 | Theme system: custom user theme | Allow saving and loading custom theme files (JSON with color key-value pairs). **Acceptance:** A user-created theme file loads correctly and overrides all system colors. | Low |
| 381 | Tooltip system | Show a tooltip when hovering over toolbar buttons and palette items for more than 800ms. **Acceptance:** Tooltips appear after delay, disappear immediately on mouse move. | Medium |
| 382 | Context menu on canvas right-click | Right-click on a widget opens a context menu: Bring to Front, Send to Back, Duplicate, Delete, Properties. **Acceptance:** All context menu items function correctly. | High |
| 383 | Context menu rendering | Implement a `ContextMenu` overlay widget drawn on top of everything. Dismisses on any click outside or Escape. **Acceptance:** Menu does not clip to panel bounds. Items highlight on hover. | High |
| 384 | Widget copy/paste (Ctrl+C/V) | Ctrl+C serializes selected widgets to clipboard as CBuild JSON format. Ctrl+V deserializes and places them offset by `(+10,+10)`. **Acceptance:** Pasting produces independent copies with new IDs. | High |
| 385 | Widget duplication (Ctrl+D) | Duplicate selected widgets in place, offset by `(+20,+20)`. **Acceptance:** Duplicated widgets are immediately selected. Undo removes them. | High |
| 386 | Widget group/ungroup | Ctrl+G groups selected widgets into a PanelWidget. Children positions become relative to panel. Ctrl+Shift+G ungroups. **Acceptance:** Grouped widgets move together. Ungrouping restores correct absolute positions. | Medium |
| 387 | Alignment tools toolbar | Add alignment buttons: Align Left, Right, Top, Bottom, Center Horizontally, Center Vertically. Operate on selected widgets. **Acceptance:** Selecting 4 buttons and clicking Align Top aligns all to the topmost widget's y coordinate. | High |
| 388 | Distribution tools toolbar | Add Distribute Horizontally and Distribute Vertically buttons. Equalize spacing between selected widgets. **Acceptance:** 4 widgets with uneven spacing become evenly spaced after distribution. | Medium |
| 389 | Widget locking | Right-click menu option Lock/Unlock widget. Locked widgets cannot be moved or resized. **Acceptance:** A locked widget's position cannot be changed by drag or keyboard nudge. | Medium |
| 390 | Canvas background color setting | Add a canvas background color picker. Stored in project file. **Acceptance:** Changing canvas background color updates immediately. Saved and loaded correctly. | Medium |
| 391 | Canvas rulers | Draw horizontal and vertical pixel rulers along the canvas edges. Ruler units adjust with zoom. **Acceptance:** At zoom=2, ruler shows 50px increments. At zoom=0.5, ruler shows 200px increments. | Medium |
| 392 | Canvas guidelines | Allow adding custom horizontal and vertical guide lines by clicking on rulers. Guides participate in the snap engine. **Acceptance:** Dragging from the ruler creates a guide. Widgets snap to guide lines. | Medium |
| 393 | Widget notes/annotations | Allow attaching sticky notes to widgets. Notes are visible in builder mode but not rendered in preview mode. **Acceptance:** Adding a note shows it as a yellow callout. Note text is editable. | Low |
| 394 | Multi-selection property editing | When multiple widgets are selected, the inspector shows only properties shared by all. Changing a shared property applies to all. **Acceptance:** Selecting 3 buttons and changing fill_color applies to all 3. | High |
| 395 | Widget search on canvas | Ctrl+F opens a search bar. Typing filters and highlights widgets by ID or label. **Acceptance:** Searching for 'btn' highlights all widgets with IDs containing 'btn'. | Medium |
| 396 | Canvas minimap | Small thumbnail preview showing all widgets and current viewport position. Clicking on the minimap pans the canvas. **Acceptance:** Minimap updates in real time. Clicking pans to the correct position. | Low |
| 397 | Widget templates library | Save selected widgets as a template with a name. Templates appear in the palette. Placing a template instantiates all constituent widgets. **Acceptance:** Saving a 'Login Form' template and placing it creates all constituent widgets. | Medium |
| 398 | Undo/redo history panel | Show the full undo stack as a scrollable list of action descriptions. Clicking on any item jumps to that undo state. **Acceptance:** History shows correct descriptions for move, resize, add, delete operations. | Low |
| 399 | Widget properties JSON export | Right-click menu option to export the selected widget's properties as a JSON file. **Acceptance:** Exported JSON is valid and contains all properties visible in the inspector. | Low |
| 400 | Canvas export as PNG | File menu option to export the canvas as a PNG image using the OpenGL framebuffer. **Acceptance:** Exported PNG is pixel-accurate at the current zoom level. | Medium |
| 401 | Canvas export as SVG | File menu option to export the canvas widget layout as an SVG file. **Acceptance:** Exported SVG opens in Inkscape with correct layout. | Low |
| 402 | Widget visibility toggle in hierarchy | Clicking the visibility dot in the hierarchy panel toggles widget visibility. **Acceptance:** Hidden widget disappears from canvas. Hidden state saved in project. | High |
| 403 | Reorder widgets in hierarchy via drag | Drag items in the hierarchy panel to reorder Z-order. **Acceptance:** Dragging a widget above another in hierarchy makes it render above on canvas. | Medium |
| 404 | Property type-specific editors | Inspector shows different editor widgets per property type: color picker for color properties, checkbox for booleans, dropdown for enum-type properties. **Acceptance:** Clicking a color property opens a color picker. Boolean properties show a checkbox. | Medium |
| 405 | Numeric property spinners | For `x`, `y`, `width`, `height`, `font_size`, add up/down arrow buttons. Click increments by 1, Shift+click increments by 10. **Acceptance:** Clicking up arrow on 'x' increases it by 1. | Medium |
| 406 | Hot-reload project file | Detect changes to the project file on disk and offer to reload. **Acceptance:** Saving the project file in a text editor triggers a reload offer within 2 seconds. | Low |
| 407 | Keyboard shortcut customization | Allow rebinding keyboard shortcuts via a settings panel. Stored in `~/.cbuild/keybindings.json`. **Acceptance:** Rebinding Ctrl+Z to Ctrl+U makes undo work with the new shortcut. | Low |
| 408 | Window session save/restore | Save window position, size, panel split positions, and last opened project on exit. Restore on next launch. **Acceptance:** CBuild reopens with the same window state and project as last session. | Medium |
| 409 | Accessibility: keyboard-only navigation | All palette items, canvas widgets, and inspector fields are reachable via Tab. **Acceptance:** Complete widget placement, configuration, and deletion without touching the mouse. | Low |
| 410 | Drag-to-resize panel dividers | The palette/canvas and canvas/inspector panel boundaries are draggable. **Acceptance:** Dragging the palette/canvas boundary changes both panel widths. Minimum panel width enforced. | Medium |
| 411 | Undo group for paste operations | Ctrl+V pasting multiple widgets creates a single undoable group command. **Acceptance:** Ctrl+Z after pasting 5 widgets removes all 5 at once. | High |
| 412 | Widget comment/description field | Add a `description` property to all widgets, shown in the inspector. Not rendered on canvas. **Acceptance:** Description text persists through save/load and appears in inspector. | Low |
| 413 | Zoom to pixel / zoom to fit entire canvas | Add buttons for zoom to exact 1:1 pixel ratio, and zoom to show full `canvas_size`. **Acceptance:** 1:1 button sets zoom to exactly 1.0. Fit Canvas sets zoom so `canvas_size` fills the viewport. | Medium |

---

## M. Testing and Quality

> These tasks establish the test infrastructure and define the quality gates. No section of CBuild ships without its corresponding unit tests, benchmarks, and regression tests.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 414 | Unit test framework integration | Integrate a header-only test framework (Catch2 or doctest) via FetchContent. Create `test/` directory. Add `ctest` CMake integration. **Acceptance:** Running `ctest` in `build/` executes tests and reports pass/fail counts. | High |
| 415 | Vec2 comprehensive unit tests | Write tests covering every `Vec2` method: all operators, length/normalize edge cases (zero vector), dot/cross products, `snapped()` rounding, `lerp` boundary conditions. **Acceptance:** All 30+ test cases pass. | High |
| 416 | Rect unit tests | Test all `Rect` methods: contains at boundary, intersects for all 8 relative positions, expanded, merged, translated, clamp inside and outside. **Acceptance:** All 40+ test cases pass. | High |
| 417 | Color unit tests | Test `Color::hex()`, `rgb()`, `with_alpha()`, `tinted()`, `lerp()`. **Acceptance:** `Color::hex(0xFF0000FF).r == 1.0f` within 0.001. | High |
| 418 | Mat3 unit tests | Test identity, translation, scale, rotation, multiplication. Verify `transform_point` and `transform_dir`. **Acceptance:** Round-trip through translate+scale+translate-back returns original point within 0.001. | High |
| 419 | Snap engine unit tests: grid | Test `snap_to_grid` for multiple positions and grid sizes. **Acceptance:** 20 grid snap test cases all return expected values. | High |
| 420 | Snap engine unit tests: threshold | Verify screen-to-canvas threshold conversion at multiple zoom levels. **Acceptance:** At zoom=0.5, a snap candidate at 8 canvas units is within the 10-screen-pixel threshold. | High |
| 421 | Snap engine unit tests: priority | Create test scenarios where grid snap and object snap are both within threshold. Verify object snap wins. **Acceptance:** Widget corner wins over nearest grid node in all 5 test scenarios. | High |
| 422 | Snap engine unit tests: alignment | Verify that alignment tracking fires for all three reference positions of a widget. **Acceptance:** Each of 6 alignment test cases (3 H, 3 V) correctly sets the locked axis. | High |
| 423 | Geometry intersection unit tests | Test `geom::line_intersection` for: perpendicular lines, 45-degree crossing, parallel lines (should return nullopt), collinear segments. **Acceptance:** All 10 intersection test cases return correct result or nullopt. | Critical |
| 424 | Geometry closest point unit tests | Test `geom::closest_point_on_segment` for: point perpendicular to middle, point beyond endpoint A, point beyond endpoint B, degenerate zero-length segment. **Acceptance:** All 8 test cases return geometrically correct point. | High |
| 425 | Widget base class unit tests | Test property get/set round-trip for all base properties. Test `mark_dirty` propagation. Test `hit_test` for points inside, on edge, and outside. **Acceptance:** All widget base tests pass. | High |
| 426 | ButtonWidget rendering test | Render a `ButtonWidget` to an offscreen framebuffer. Sample pixels at the center and outside. **Acceptance:** Center pixel is within tolerance of expected fill color. | High |
| 427 | Canvas undo/redo unit tests | Test full undo/redo for: add widget, delete widget, move widget, resize widget, multi-widget move. **Acceptance:** Each operation can be undone and redone correctly 3 times in sequence. | Critical |
| 428 | Canvas selection unit tests | Test: single select, multi-select with Shift, Ctrl+A select all, rubber-band select, deselect all. **Acceptance:** All selection scenarios result in correct `selected_ids_` contents. | High |
| 429 | Project save/load round-trip test | Create a project with 5 different widget types with non-default properties. Save. Load. Verify all widgets have identical properties. **Acceptance:** Round-trip preserves all tested properties within floating point precision. | Critical |
| 430 | Canvas pan/zoom unit tests | Test `canvas_to_screen` and `screen_to_canvas` at multiple zoom levels. Test `zoom_at` pivot behavior. **Acceptance:** Round-trip conversion returns original point within 0.001. Pivot point stays fixed during `zoom_at`. | High |
| 431 | Backend HttpClient integration test | Run a mock HTTP server. Call `HttpClient::post()` and `get()` against it. **Acceptance:** POST returns 200 with the posted body echoed back. | High |
| 432 | Ollama client unit test (mock) | Create a mock Ollama server using Python `http.server`. Call `OllamaClient::chat_async`. **Acceptance:** `on_done` receives non-empty string within 2 seconds. | High |
| 433 | MainThreadDispatcher thread safety test | Spawn 100 worker threads each posting 100 tasks. Call `flush()` from the main thread after all threads complete. **Acceptance:** Exactly 10,000 tasks execute. No crashes under ThreadSanitizer. | Critical |
| 434 | Docker service unit test | Test `DockerService::status_async` for a known running container and a known non-existent container. **Acceptance:** Returns true for running, false for non-existent, within 3 seconds. | High |
| 435 | Chunked HTTP parser unit test | Feed a known chunked HTTP response in multiple partial reads. **Acceptance:** Body assembled from 10-byte chunks matches the expected full body exactly. | High |
| 436 | Renderer batch overflow test | Submit `MAX_QUADS+1` draw_rect calls. Verify no crash and correct rendering. **Acceptance:** All quads render. Exactly 2 draw calls are issued (one per batch fill). | Critical |
| 437 | Widget factory completeness test | Call `WidgetFactory::instance().create()` for every registered type name. **Acceptance:** All registered types can be instantiated with a fresh ID. | High |
| 438 | Snap engine performance benchmark | Run `snap()` with 500 widgets on canvas, 1000 times. Measure total time. **Acceptance:** 1000 `snap()` calls for 500 widgets complete in under 500ms total. | High |
| 439 | Renderer frame rate benchmark | Render a frame with 1000 widgets with snap visuals active. Measure FPS. **Acceptance:** Release build achieves >= 60 FPS. | High |
| 440 | Memory leak test (ASAN) | Run the full UI loop for 60 seconds with ASAN build. Create/delete widgets, undo/redo, pan/zoom. **Acceptance:** ASAN reports zero memory leaks or heap-use-after-free. | Critical |
| 441 | Concurrent backend callback test | Start 5 Ollama chat requests simultaneously. Verify all responses arrive correctly via dispatcher. **Acceptance:** All 5 `on_done` callbacks fire with correct non-empty content. | High |
| 442 | UI input stress test | Simulate 1000 random mouse events programmatically. **Acceptance:** Application does not crash or deadlock after 1000 random events. | Medium |
| 443 | Widget property fuzzing | For each widget type, call `set_property()` with random strings for every property key. **Acceptance:** No crash or undefined behavior. Invalid values are rejected gracefully. | Medium |
| 444 | Cross-platform build test matrix | Run CI builds on: Ubuntu 22.04, Ubuntu 20.04, macOS 12, macOS 13, Windows 10 (VS2022), Windows 11 (VS2022). **Acceptance:** All 6 configurations build without errors. | High |
| 445 | Frame timing consistency test | Run the main loop for 10 seconds with vsync enabled. Measure frame time variance. **Acceptance:** 99% of frames complete within 16.7ms ± 1ms at 60Hz. | Medium |
| 446 | Save/load stress test | Create project with 100 widgets of all types. Save and load 10 times in sequence. **Acceptance:** Final loaded state is identical to the original. No memory leaks per iteration. | High |
| 447 | Static analysis CI gate | clang-tidy runs on all source files as part of CI. Build fails if new warnings are introduced. **Acceptance:** CI pipeline blocks PRs that introduce clang-tidy warnings. | Medium |
| 448 | Code coverage reporting | Add lcov/gcov instrumentation to debug builds. Generate HTML coverage report. **Acceptance:** Test suite achieves >= 70% line coverage on the math, canvas, and snap engine modules. | Medium |
| 449 | Regression test: undo after snap | Snap a widget to a corner, release drag (commit), press Ctrl+Z. Widget returns to exact pre-drag position. **Acceptance:** Undo is computed from the committed move delta, not the drag origin. | High |
| 450 | Regression test: resize then undo | Resize a widget, commit, Ctrl+Z. Widget returns to exact original rect. **Acceptance:** All 8 resize handle directions are tested. | High |
| 451 | Regression test: multi-select drag | Select 3 widgets, drag them together, release, Ctrl+Z. All 3 return to their original individual positions. **Acceptance:** No widget is left at an incorrect position after undo. | High |
| 452 | Regression test: project round-trip | Save a project. Close CBuild. Reopen. Load the project. Verify: widget count, all IDs, all positions, all styles match the saved state. **Acceptance:** Zero property differences between saved and loaded state for all tested widgets. | Critical |
| 453 | Performance regression CI | Add a benchmark CI step that fails if the snap engine or frame rate benchmark degrades by more than 10% from the baseline. **Acceptance:** CI catches performance regressions before they merge. | Medium |
| 454 | Valgrind clean run | Run the full UI loop for 30 seconds under Valgrind. **Acceptance:** Zero definitely lost, zero indirectly lost bytes. | High |
| 455 | Documentation test: all code examples compile | All code examples in docs/ are extracted and compiled as test programs. **Acceptance:** Every code example in the documentation compiles without modification. | Low |
| 456 | Accessibility audit | Run the UI with keyboard-only navigation and verify all interactive elements are reachable. **Acceptance:** A complete workflow is achievable without mouse. | Low |
| 457 | Localization readiness audit | Verify all user-visible strings are in a single `strings.hpp` header. **Acceptance:** 100% of UI strings are defined in `strings.hpp`, zero hardcoded UI strings in implementation files. | Low |
| 458 | Build time optimization audit | Measure clean build time. Identify the 3 slowest compilation units. Use forward declarations to reduce include chains. **Acceptance:** Clean build time reduced by at least 15% after optimization. | Low |
| 459 | Crash reporter | On `SIGSEGV` or `SIGABRT`, write a crash report to `~/.cbuild/crash_TIMESTAMP.txt` including the signal, backtrace, last known state, and CBuild version. **Acceptance:** A deliberate crash writes a readable crash report file. | Medium |
| 460 | User telemetry: usage event logging (opt-in) | Optionally log usage events to a local file. No network transmission. Opt-in at first launch. **Acceptance:** Logs are written locally if opted in. Zero network activity regardless of opt-in state. | Low |
| 461 | Build artifact checksums | CMake post-build step generates SHA256 checksum files for all distributable artifacts. **Acceptance:** Running `sha256sum -c CBuild.sha256` verifies the binary and assets are unmodified. | Low |
| 462 | Installation self-test | On first launch, CBuild runs a self-test: creates a window, initializes renderer, creates a test widget, renders one frame, destroys everything. **Acceptance:** Self-test completes in under 2 seconds and correctly reports GL version and capabilities. | High |

---

## N. Advanced Features and Integrations

> These final tasks extend CBuild beyond its core builder role: code generation, terminal emulator, database widgets, real-time charts, plugin system, collaborative editing, and the two definitive integration tests that validate the full system end-to-end.

| # | Task Name | Description & Acceptance Criteria | Priority |
|---|-----------|-----------------------------------|----------|
| 463 | CBuild project export as native C++ code | Generate C++ source code from the current canvas layout. Each widget placement becomes a constructor call with property setters. **Acceptance:** Generated C++ code compiles and produces the same visual layout as the canvas. | High |
| 464 | CBuild project export as React component | Generate a React JSX component from the canvas layout. Each widget maps to a corresponding HTML element with inline styles. **Acceptance:** Generated JSX renders in a browser with approximately the same visual layout. | Medium |
| 465 | CBuild project export as Qt .ui file | Generate a Qt Designer `.ui` file from the canvas. **Acceptance:** The `.ui` file opens in Qt Designer and shows a similar layout. | Low |
| 466 | CBuild scripting API (C-compatible) | Expose a C API: `cbuild_add_widget()`, `cbuild_set_property()`, `cbuild_connect_backend()`. **Acceptance:** A C program calling the API adds a widget to the running CBuild instance. | Low |
| 467 | Real-time collaborative editing | Two CBuild instances connect via TCP. Canvas mutations are serialized and broadcast to the peer. **Acceptance:** Adding a widget on instance A appears on instance B within 100ms. | Low |
| 468 | Plugin system: dynamic library loading | Define a CBuild plugin API header. Plugins are `.so/.dll` files in `~/.cbuild/plugins/`. **Acceptance:** A sample plugin adding a 'CanvasChart' widget type loads and appears in the palette. | Low |
| 469 | Ollama model fine-tuning interface | Add a panel for creating fine-tuning datasets from within CBuild. Dataset exported as JSONL. **Acceptance:** A 5-sample dataset is exported in correct Ollama training format. | Low |
| 470 | Chat history persistence | Save chat conversations to `~/.cbuild/chat_history.json`. **Acceptance:** Conversation history from previous sessions is visible and searchable. | Medium |
| 471 | Multi-model parallel inference panel | Send the same prompt to multiple Ollama models simultaneously. Display responses side-by-side. **Acceptance:** Three model responses arrive and display independently within the timeout. | Low |
| 472 | Streaming response typewriter animation | Display LLM response tokens one by one as they arrive (typewriter effect). **Acceptance:** Response text appears character by character, never all at once. | High |
| 473 | Code execution widget | A widget that contains a code editor and 'Run' button. Executes the code in a Docker sandbox. **Acceptance:** Pressing Run executes Python code in a Docker container and shows output in the widget. | Medium |
| 474 | Network request builder widget | A widget with URL, method, headers, body fields and a 'Send' button. **Acceptance:** Sending a GET request to httpbin.org/get shows the JSON response. | High |
| 475 | Database query widget | A widget with a SQL editor and 'Execute' button connected to a PostgreSQL backend. **Acceptance:** Executing `SELECT * FROM pg_tables` shows the results in a formatted table. | Medium |
| 476 | File browser widget | A widget displaying a directory tree. Navigate into subdirectories. Click a file to open it in the code editor widget. **Acceptance:** Navigating to `/home/user/src` shows the directory tree. Clicking a `.cpp` file opens its contents. | Medium |
| 477 | Terminal emulator widget | A widget providing a pseudo-terminal (pty) to a shell process. **Acceptance:** Running `ls -la` in the terminal widget shows the directory listing. | High |
| 478 | System resource monitor widget | A widget showing real-time CPU%, RAM%, and disk I/O usage via `/proc/stat` and `/proc/meminfo`. Updates at 1Hz. **Acceptance:** Values match those shown by `htop`. | Medium |
| 479 | Log aggregator widget | Connect to multiple backend log streams simultaneously. Aggregate lines with timestamps. Filter by source, severity, or text. **Acceptance:** Logs from Ollama container and custom backend appear in the same widget filtered independently. | Medium |
| 480 | Time-series chart widget | A widget plotting numerical data over time. Backend pushes data points. Widget draws a scrolling line chart. **Acceptance:** Plotting CPU usage over 60 seconds shows a continuous line chart that scrolls. | Medium |
| 481 | Image preview widget | A widget displaying PNG/JPG images from a file path or URL (via HttpClient). **Acceptance:** Image from a local path renders correctly at widget dimensions. | Low |
| 482 | Markdown renderer widget | A widget parsing and rendering Markdown text. Supports headings, bold, italic, code blocks, and lists. **Acceptance:** A README.md file renders with visible heading hierarchy, bold text, and formatted code blocks. | Low |
| 483 | cbuildrc configuration file | On startup, read `~/.cbuild/cbuildrc` for global settings: default window size, snap settings, theme, Ollama host. **Acceptance:** Setting `ollama_host=http://192.168.1.100:11434` in cbuildrc uses that host by default. | Medium |
| 484 | Environment variable expansion in project files | Support `$ENV_VAR` references in project file values. Expand on load. **Acceptance:** `OLLAMA_URL=http://localhost:11434` in environment is expanded when loading a project with `$OLLAMA_URL`. | Medium |
| 485 | Project template system | Pre-built project templates: 'Chatbot UI', 'Dashboard', 'Form Builder', 'OS Dev Workstation'. Accessible from File > New from Template. **Acceptance:** The 'Chatbot UI' template creates a canvas pre-populated with ChatBubble, TextInput, and Button widgets. | Medium |
| 486 | CBuild CLI interface | `cbuild --headless --load project.cbuild --run`: loads project and starts backends without showing UI. **Acceptance:** `cbuild --headless --load project.cbuild --check` returns 0 if project loads cleanly, 1 if invalid. | Low |
| 487 | Widget property binding expression | Allow widget property values to be expressions referencing other widget properties or backend state. **Acceptance:** A Label bound to a TextInput's value updates in real time as the user types. | Low |
| 488 | Conditional widget visibility | Add a `visibility_expression` property supporting simple conditions like `'checkbox_1.checked == true'`. **Acceptance:** A Panel hidden when a checkbox is unchecked appears when the checkbox is checked. | Low |
| 489 | Keyboard macro recording | Record a sequence of keyboard/mouse actions and replay them. Store as named macros. **Acceptance:** Recording 'add button, set label to OK, move to (100,100)' and replaying produces the same result. | Low |
| 490 | Canvas history timeline widget | A visual timeline showing all canvas states as thumbnail previews. Clicking jumps to that state. **Acceptance:** Timeline shows accurate thumbnails for the last 20 states. | Low |
| 491 | CBuild update system | Periodically check GitHub releases API for a newer version. Show a non-intrusive notification in the status bar. **Acceptance:** With a mock newer version in the API, notification appears within 5 minutes. | Low |
| 492 | Localization: string extraction | Extract all user-visible strings to a `strings.hpp` file with a `STR()` macro. Add English locale file. **Acceptance:** Adding a new locale file changes all UI text to that locale. | Low |
| 493 | Performance profiler overlay | Press F3 to show an overlay with per-system timing: render ms, snap engine ms, backend flush ms, total frame ms. **Acceptance:** Overlay shows accurate timings measured via `std::chrono`. | Medium |
| 494 | Drag-from-canvas to system | Allow dragging a serialized widget description from CBuild canvas to a text editor (system drag-and-drop). **Acceptance:** Dragging a widget to a text editor pastes its JSON representation. | Low |
| 495 | Drag-from-system to canvas | Allow dragging an image file from the file manager to the canvas. Auto-creates an ImageWidget at the drop position. **Acceptance:** Dragging a PNG from the system file manager creates an ImageWidget. | Low |
| 496 | Widget visibility hierarchy filter | The hierarchy panel has a filter to show only: visible widgets, invisible widgets, locked widgets, or all widgets. **Acceptance:** Filtering for 'locked' shows only locked widgets in hierarchy. | Low |
| 497 | Backend connection timeout configuration | Per-backend timeout configuration in the backend panel. **Acceptance:** Setting a 1-second timeout causes a timeout error callback after 1 second, not 30. | Medium |
| 498 | Export widget to stb_easy_font code | Right-click a TextWidget and 'Export as stb_easy_font code' generates C code using `stb_easy_font`. **Acceptance:** The generated C code, when compiled and run, produces visually similar text output. | Low |
| 499 | CBuild project format versioning | Add `version: 1` field to project files. Add migration logic for loading older project formats. **Acceptance:** A v1 project file loads correctly in CBuild v2 with deprecated properties handled gracefully. | Medium |
| 500 | Startup time optimization | Profile CBuild startup sequence. Defer non-critical initialization to background threads. **Acceptance:** Window appears within 100ms of launch. Full initialization completes within 2 seconds. | Medium |
| 501 | Widget batch operations via CLI | `cbuild --load project.cbuild --set-property widget_id key value --save`: modifies a property without opening UI. **Acceptance:** Changing a button label via CLI and reloading in GUI shows the updated label. | Low |
| 502 | Canvas measurement tool | Hold M key while dragging to display the distance and angle between start and end points in a floating label. **Acceptance:** Dragging 100 canvas pixels horizontally shows '100px, 0°'. | Low |
| 503 | Widget frame/no-frame toggle | Double-click a PanelWidget to hide its title bar and border, turning it into a transparent container. **Acceptance:** Double-click hides the panel border. Content remains visible and interactive. | Low |
| 504 | CBuild project file .gitignore template | Include a `.gitignore` template in the project file output. **Acceptance:** Template appears in generated project directory and prevents cluttering git status. | Low |
| 505 | Drag-and-drop reordering of backends | In the backend panel, drag-and-drop to reorder backend services. **Acceptance:** Dragging Ollama above the custom backend changes its position in the backend list. | Low |
| 506 | Backend query history | Track the last 20 queries sent to each backend. Show in a dropdown. **Acceptance:** After sending 5 queries to Ollama, the history shows all 5 in reverse chronological order. | Low |
| 507 | Quick-access widget favorites | Mark palette items as favorites. Favorites appear at the top of the palette. **Acceptance:** Marking Button and TextInput as favorites shows them at top regardless of category. | Low |
| 508 | CBuild API documentation generation | Run doxygen on all header files to generate HTML documentation. **Acceptance:** doxygen produces docs without warnings. All public classes and methods have documentation entries. | Low |
| 509 | **Final integration test: chatbot app** | Build a complete chatbot application using CBuild: TextInput for user message, Button to send, ChatBubble display panel, Ollama backend connected. Save as project. Load. Verify all connections work. **Acceptance:** Typing a message and pressing Send calls Ollama and shows the response in a ChatBubble within 10 seconds. | **Critical** |
| 510 | **Final integration test: OS dev dashboard** | Build the OS dev dashboard project: QEMU launch panel, serial console, GDB register display, build output panel. Save. Load. **Acceptance:** All panels connect to their respective backends correctly. | **High** |

---

*Total tasks: **510** across 14 sections. At an average of 3 hours per task, full completion represents approximately 1,530 development hours. The Critical + High priority subset (~430 tasks) delivers a production-quality CBuild suitable for daily OS development workflows. Medium and Low priority tasks add the polish and extensibility of a mature, independent developer tool.*
