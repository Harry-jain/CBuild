#pragma once
#include "vec2.hpp"
#include "rect.hpp"
#include <optional>
#include <cmath>
#include <algorithm>
#include <limits>

namespace geom {

inline Vec2 closest_point_on_segment(const Vec2& a, const Vec2& b, const Vec2& p) {
    Vec2 ab = b - a;
    float len_sq = ab.length_sq();
    if (len_sq < 0.00001f) return a;
    float t = (p - a).dot(ab) / len_sq;
    t = std::max(0.0f, std::min(1.0f, t));
    return a + ab * t;
}

inline float distance_to_segment(const Vec2& a, const Vec2& b, const Vec2& p) {
    return (p - closest_point_on_segment(a, b, p)).length();
}

inline std::optional<Vec2> line_intersection(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4) {
    float x1 = p1.x, y1 = p1.y;
    float x2 = p2.x, y2 = p2.y;
    float x3 = p3.x, y3 = p3.y;
    float x4 = p4.x, y4 = p4.y;

    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(den) < 0.00001f) {
        return std::nullopt;
    }

    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
    return Vec2(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
}

inline Vec2 screen_to_ndc(const Vec2& p, float w, float h) {
    if (w <= 0.0f || h <= 0.0f) return Vec2();
    return Vec2((p.x / w) * 2.0f - 1.0f, 1.0f - (p.y / h) * 2.0f);
}

inline float swept_aabb(const Rect& moving, const Vec2& vel, const Rect& stationary) {
    float entry_x, entry_y;
    float exit_x, exit_y;

    if (vel.x > 0.0f) {
        entry_x = stationary.x - moving.right();
        exit_x = stationary.right() - moving.x;
    } else {
        entry_x = stationary.right() - moving.x;
        exit_x = stationary.x - moving.right();
    }

    if (vel.y > 0.0f) {
        entry_y = stationary.y - moving.bottom();
        exit_y = stationary.bottom() - moving.y;
    } else {
        entry_y = stationary.bottom() - moving.y;
        exit_y = stationary.y - moving.bottom();
    }

    float entry_time_x = (std::abs(vel.x) < 0.00001f) ? -std::numeric_limits<float>::infinity() : entry_x / vel.x;
    float entry_time_y = (std::abs(vel.y) < 0.00001f) ? -std::numeric_limits<float>::infinity() : entry_y / vel.y;

    float exit_time_x = (std::abs(vel.x) < 0.00001f) ? std::numeric_limits<float>::infinity() : exit_x / vel.x;
    float exit_time_y = (std::abs(vel.y) < 0.00001f) ? std::numeric_limits<float>::infinity() : exit_y / vel.y;

    float entry_time = std::max(entry_time_x, entry_time_y);
    float exit_time = std::min(exit_time_x, exit_time_y);

    if (entry_time > exit_time || (entry_time_x < 0.0f && entry_time_y < 0.0f) || entry_time_x > 1.0f || entry_time_y > 1.0f) {
        return 1.0f;
    }

    return std::max(0.0f, entry_time);
}

inline Vec2 polar_to_cartesian(float angle_rad, float radius) {
    return Vec2(std::cos(angle_rad) * radius, std::sin(angle_rad) * radius);
}

} // namespace geom
