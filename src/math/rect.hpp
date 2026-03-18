#pragma once
#include "vec2.hpp"
#include <algorithm>

struct Rect {
    float x;
    float y;
    float w;
    float h;

    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

    Vec2 position() const { return Vec2(x, y); }
    Vec2 size() const { return Vec2(w, h); }
    
    Vec2 center() const { return Vec2(x + w * 0.5f, y + h * 0.5f); }
    Vec2 top_left() const { return Vec2(x, y); }
    Vec2 top_right() const { return Vec2(x + w, y); }
    Vec2 bottom_left() const { return Vec2(x, y + h); }
    Vec2 bottom_right() const { return Vec2(x + w, y + h); }
    
    Vec2 top_center() const { return Vec2(x + w * 0.5f, y); }
    Vec2 bottom_center() const { return Vec2(x + w * 0.5f, y + h); }
    Vec2 left_center() const { return Vec2(x, y + h * 0.5f); }
    Vec2 right_center() const { return Vec2(x + w, y + h * 0.5f); }
    
    float right() const { return x + w; }
    float bottom() const { return y + h; }

    bool contains(const Vec2& p) const {
        return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
    }

    bool intersects(const Rect& o) const {
        return !(o.x > right() || o.right() < x || o.y > bottom() || o.bottom() < y);
    }
    
    Rect expanded(float amount) const {
        return Rect(x - amount, y - amount, w + amount * 2.0f, h + amount * 2.0f);
    }
    
    Rect merged(const Rect& o) const {
        float min_x = std::min(x, o.x);
        float min_y = std::min(y, o.y);
        float max_r = std::max(right(), o.right());
        float max_b = std::max(bottom(), o.bottom());
        return Rect(min_x, min_y, max_r - min_x, max_b - min_y);
    }
    
    Rect translated(const Vec2& delta) const {
        return Rect(x + delta.x, y + delta.y, w, h);
    }
    
    Vec2 clamp(const Vec2& p) const {
        float cx = std::max(x, std::min(p.x, right()));
        float cy = std::max(y, std::min(p.y, bottom()));
        return Vec2(cx, cy);
    }
};
