#pragma once
#include "../math/vec2.hpp"
#include "keycodes.hpp"

struct KeyEvent {
    Key key;
    bool pressed;
    bool alt;
    bool ctrl;
    bool shift;
};

struct CharEvent {
    unsigned int codepoint;
};

struct MouseMoveEvent {
    Vec2 position;
    Vec2 delta;
};

struct MouseButtonEvent {
    MouseButton button;
    bool pressed;
    Vec2 position;
};

struct ScrollEvent {
    float x_offset;
    float y_offset;
};

struct ResizeEvent {
    int width;
    int height;
};
