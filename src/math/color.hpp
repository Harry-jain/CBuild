#pragma once
#include <cstdint>
#include <algorithm>

struct Color {
    float r;
    float g;
    float b;
    float a;

    Color() : r(0), g(0), b(0), a(1.0f) {}
    Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

    static Color hex(uint32_t rgba) {
        return Color(
            ((rgba >> 24) & 0xFF) / 255.0f,
            ((rgba >> 16) & 0xFF) / 255.0f,
            ((rgba >> 8) & 0xFF) / 255.0f,
            (rgba & 0xFF) / 255.0f
        );
    }

    static Color rgb(uint32_t rgb_val) {
        return Color(
            ((rgb_val >> 16) & 0xFF) / 255.0f,
            ((rgb_val >> 8) & 0xFF) / 255.0f,
            (rgb_val & 0xFF) / 255.0f,
            1.0f
        );
    }

    Color tinted(float factor) const {
        return Color(
            std::max(0.0f, std::min(1.0f, r * factor)),
            std::max(0.0f, std::min(1.0f, g * factor)),
            std::max(0.0f, std::min(1.0f, b * factor)),
            a
        );
    }
    
    Color with_alpha(float new_a) const {
        return Color(r, g, b, new_a);
    }

    static Color lerp(const Color& from, const Color& to, float t) {
        return Color(
            from.r + (to.r - from.r) * t,
            from.g + (to.g - from.g) * t,
            from.b + (to.b - from.b) * t,
            from.a + (to.a - from.a) * t
        );
    }
    
    // Core design system palette
    static Color canvas_bg() { return rgb(0x14120b); } // User-requested background
    static Color panel_bg() { return rgb(0x1a1a1a); }
    static Color panel_border() { return rgb(0x333333); }
    static Color accent() { return rgb(0x4F46E5); }
    static Color accent_dim() { return rgb(0x3730A3); }
    static Color text_primary() { return rgb(0xF3F4F6); }
    static Color text_secondary() { return rgb(0x9CA3AF); }
    static Color snap_grid() { return hex(0x38BDF866); } 
    static Color snap_track() { return rgb(0x38BDF8); } 
    static Color snap_point() { return rgb(0xF97316); } 
    static Color snap_center() { return rgb(0x22C55E); } 
    static Color selection() { return hex(0x4F46E533); } 
    static Color selection_border() { return rgb(0x4F46E5); }
    static Color transparent() { return Color(0, 0, 0, 0); }
    static Color red() { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
    static Color blue() { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
    static Color white() { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
    static Color green() { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
    static Color grid_minor() { return Color(0.3f, 0.3f, 0.3f, 0.15f); }
    static Color grid_major() { return Color(0.4f, 0.4f, 0.4f, 0.35f); }
};
