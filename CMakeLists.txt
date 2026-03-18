cmake_minimum_required(VERSION 3.20)
project(CBuild VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# T-026: Feature Flags
option(CBUILD_STATIC_MUSL "Build statically against musl-libc on Linux" OFF)
option(CBUILD_ENABLE_DOCKER "Enable Docker integration backend" ON)
option(CBUILD_ENABLE_OPENGL "Build with OpenGL (DISABLED BY BLUEPRINT)" OFF)

# T-032: ccache integration
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# T-022: clang-tidy integration
find_program(CLANG_TIDY_PROGRAM clang-tidy)
if(CLANG_TIDY_PROGRAM)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_PROGRAM};-checks=bugprone-*,modernize-*")
endif()

# Compiler warnings and reproducible builds
# T-012, T-031
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    add_compile_options(-Wall -Wextra -Wpedantic -ffile-prefix-map=${CMAKE_SOURCE_DIR}=. -Wno-unused-parameter)
endif()

# Debug/Release configs (T-013, T-014)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT MSVC)
        add_compile_options(-fsanitize=address -fsanitize=undefined -g)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /DNDEBUG)
    else()
        add_compile_options(-O3 -DNDEBUG -march=native)
    endif()
endif()

if(CBUILD_STATIC_MUSL AND NOT MSVC AND NOT APPLE)
    add_compile_options(-static -static-libgcc -static-libstdc++)
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

# Variables
if(WIN32)
    set(SOURCE_FILES src/main.cpp src/platform/window_win32.cpp src/render/renderer.cpp src/render/font_renderer.cpp src/widgets/widget.cpp)
else()
    set(SOURCE_FILES src/main.cpp src/render/renderer.cpp src/render/font_renderer.cpp src/widgets/widget.cpp)
endif()

# Main Target
add_executable(CBuild ${SOURCE_FILES})
target_include_directories(CBuild PRIVATE vendor/stb)

if(WIN32)
    target_link_libraries(CBuild PRIVATE ws2_32 user32 gdi32 shell32)
elseif(APPLE)
    target_link_libraries(CBuild PRIVATE "-framework Cocoa" "-framework CoreVideo")
else()
    find_package(Threads REQUIRED)
    if(NOT CBUILD_STATIC_MUSL)
        find_package(X11 REQUIRED)
        target_include_directories(CBuild PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(CBuild PRIVATE Threads::Threads ${X11_LIBRARIES})
    else()
        target_link_libraries(CBuild PRIVATE Threads::Threads -lX11)
    endif()
endif()

# T-015: Asset copy post-build command
add_custom_command(TARGET CBuild POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:CBuild>/assets
)

# T-024: ASAN integration test executable
add_executable(asan_test test/asan_test.cpp)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    target_compile_options(asan_test PRIVATE -fsanitize=address)
    target_link_options(asan_test PRIVATE -fsanitize=address)
endif()

# T-063: Math integration test executable
add_executable(math_test test/math_test.cpp)

# T-115: Renderer bounds integration test
add_executable(render_test test/render_test.cpp src/render/renderer.cpp src/render/font_renderer.cpp)

# T-186: Widget system integration test
add_executable(widget_test test/widget_test.cpp src/render/renderer.cpp src/render/font_renderer.cpp src/widgets/widget.cpp)

# T-144: Renderer performance benchmark
add_executable(render_benchmark test/render_benchmark.cpp src/render/renderer.cpp src/render/font_renderer.cpp)

# T-023: Valgrind target
add_custom_target(valgrind
    COMMAND valgrind --leak-check=full $<TARGET_FILE:CBuild>
    DEPENDS CBuild
    COMMENT "Running Valgrind on CBuild"
)

# T-028: AppImage packaging target
add_custom_target(package-appimage
    COMMAND echo "Placeholder for linuxdeploy execution"
    COMMENT "Building Linux AppImage"
)

# T-029, T-030: CPack setup
include(InstallRequiredSystemLibraries)
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${CBuild_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CBuild_VERSION_MINOR}")
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ")
endif()
include(CPack)

# T-016: CMake build summary message
message(STATUS "=== CBuild Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Static MUSL (Linux): ${CBUILD_STATIC_MUSL}")
message(STATUS "============================")
