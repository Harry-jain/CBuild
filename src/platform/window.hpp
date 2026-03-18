#pragma once
#include <string>
#include <functional>
#include <cstdint>
#include <vector>
#include "../math/vec2.hpp"
#include "events.hpp"

// Forward declaration to hide OS payload (HWND/Display*)
struct WindowHandle;

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    bool is_open() const;
    void close();
    void poll_events();
    
    // Core software rendering mapping
    void swap_buffers();
    uint32_t* get_framebuffer();
    
    int get_width() const { return width_; }
    int get_height() const { return height_; }
    float get_aspect_ratio() const { return (float)width_ / (float)height_; }

    // Live state
    bool is_key_down(Key key) const;
    bool is_mouse_button_down(MouseButton button) const;
    Vec2 get_mouse_position() const { return mouse_pos_; }

    // High resolution timer
    static double get_time();

    // OS interactions
    void set_title(const std::string& title);
    void set_cursor(CursorShape shape);
    std::string get_clipboard() const;
    void set_clipboard(const std::string& text);

    // Callbacks mapping
    std::function<void(const KeyEvent&)> on_key;
    std::function<void(const CharEvent&)> on_char;
    std::function<void(const MouseMoveEvent&)> on_mouse_move;
    std::function<void(const MouseButtonEvent&)> on_mouse_button;
    std::function<void(const ScrollEvent&)> on_scroll;
    std::function<void(const ResizeEvent&)> on_resize;

private:
    WindowHandle* handle_;
    int width_;
    int height_;
    bool is_open_;
    std::vector<uint32_t> framebuffer_;
    Vec2 mouse_pos_;

public:
    // Internal methods accessed by static OS callbacks
    void _internal_set_size(int w, int h);
    void _internal_close();
    void _internal_update_mouse(const Vec2& pos);
};
