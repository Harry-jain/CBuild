#pragma once
#include <cmath>

struct Vec2 {
    float x;
    float y;

    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
    
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return x != o.x || y != o.y; }

    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
    float cross(const Vec2& o) const { return x * o.y - y * o.x; }
    float length_sq() const { return x * x + y * y; }
    float length() const { return std::sqrt(length_sq()); }
    
    Vec2 normalized() const {
        float len = length();
        if (len < 0.000001f) return Vec2();
        return *this / len;
    }
    
    float distance_sq_to(const Vec2& o) const { return (*this - o).length_sq(); }
    float distance_to(const Vec2& o) const { return (*this - o).length(); }
    
    Vec2 snapped(float grid_size) const {
        if (grid_size <= 0.000001f) return *this;
        return Vec2(std::round(x / grid_size) * grid_size, std::round(y / grid_size) * grid_size);
    }
    
    static Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
        return a + (b - a) * t;
    }
};
