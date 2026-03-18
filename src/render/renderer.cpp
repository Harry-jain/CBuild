#include "renderer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include "../../vendor/stb/stb_easy_font.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  Construction / Lifecycle
// ═══════════════════════════════════════════════════════════════════════════════

Renderer2D::Renderer2D(uint32_t* framebuffer, int width, int height)
    : fb_(framebuffer), width_(width), height_(height), debug_overlay_(false) {
    reset_stats();
}

void Renderer2D::update_buffer(uint32_t* new_fb, int new_w, int new_h) {
    fb_ = new_fb;
    width_ = new_w;
    height_ = new_h;
}

bool Renderer2D::load_font(const std::string& path, float size) {
    return font_.load(path, size);
}

void Renderer2D::shutdown() {
    fb_ = nullptr;
    width_ = 0;
    height_ = 0;
    scissors_.clear();
    reset_stats();
    view_ = ViewTransform();
}

void Renderer2D::reset_stats() {
    std::memset(&stats_, 0, sizeof(RenderStats));
}

void Renderer2D::begin_frame(const Color& clear_color) {
    reset_stats();
    clear(clear_color);
    stats_.draw_calls++;
}

void Renderer2D::end_frame() {
    // In software rendering, frame is complete once all draws are done.
    // This is a logical boundary for layer ordering guarantees:
    // Callers must draw in order: background → shapes → lines → text.
    // No deferred batching needed since we write directly to the framebuffer.
    stats_.draw_calls++;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Core Pixel Operations
// ═══════════════════════════════════════════════════════════════════════════════

inline uint32_t Renderer2D::color_to_uint32(const Color& c) const {
    uint8_t a = static_cast<uint8_t>(std::clamp(c.a * 255.0f, 0.0f, 255.0f));
    uint8_t r = static_cast<uint8_t>(std::clamp(c.r * 255.0f, 0.0f, 255.0f));
    uint8_t g = static_cast<uint8_t>(std::clamp(c.g * 255.0f, 0.0f, 255.0f));
    uint8_t b = static_cast<uint8_t>(std::clamp(c.b * 255.0f, 0.0f, 255.0f));
    return (a << 24) | (r << 16) | (g << 8) | b;
}

inline uint32_t Renderer2D::blend_fast(uint32_t bg, uint32_t fg, float alpha) const {
    if (alpha <= 0.001f) return bg;
    if (alpha >= 0.999f) return fg;
    
    uint32_t a_uint = static_cast<uint32_t>(alpha * 256);
    uint32_t ia_uint = 256 - a_uint;
    
    uint32_t rb = (((bg & 0xFF00FF) * ia_uint) + ((fg & 0xFF00FF) * a_uint)) >> 8;
    uint32_t g  = (((bg & 0x00FF00) * ia_uint) + ((fg & 0x00FF00) * a_uint)) >> 8;
    
    return (rb & 0xFF00FF) | (g & 0x00FF00) | 0xFF000000;
}

bool Renderer2D::clip_point(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
    if (!scissors_.empty()) {
        const Rect& s = scissors_.back().rect;
        if (static_cast<float>(x) < s.x || static_cast<float>(x) > s.right() ||
            static_cast<float>(y) < s.y || static_cast<float>(y) > s.bottom()) return false;
    }
    return true;
}

void Renderer2D::put_pixel(int x, int y, const Color& color) {
    if (color.a <= 0.001f) return;
    if (!clip_point(x, y)) return;
    
    int index = y * width_ + x;
    if (color.a >= 0.999f) {
        fb_[index] = color_to_uint32(color);
    } else {
        fb_[index] = blend_fast(fb_[index], color_to_uint32(color), color.a);
    }
    stats_.pixels_drawn++;
}

void Renderer2D::put_pixel_alpha(int x, int y, const Color& color, float alpha_override) {
    if (alpha_override <= 0.001f) return;
    if (!clip_point(x, y)) return;
    int index = y * width_ + x;
    
    uint32_t fg = color_to_uint32(color);
    float final_alpha = color.a * alpha_override;
    if (final_alpha >= 0.999f) {
        fb_[index] = fg;
    } else {
        fb_[index] = blend_fast(fb_[index], fg, final_alpha);
    }
    stats_.pixels_drawn++;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Clear
// ═══════════════════════════════════════════════════════════════════════════════

void Renderer2D::clear(const Color& color) {
    if (!fb_ || width_ <= 0 || height_ <= 0) return;
    uint32_t c = color_to_uint32(color);
    std::fill(fb_, fb_ + (width_ * height_), c);
    stats_.primitives_drawn++;
    stats_.pixels_drawn += static_cast<uint32_t>(width_ * height_);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Rectangle Primitives
// ═══════════════════════════════════════════════════════════════════════════════

void Renderer2D::draw_rect_solid(const Rect& rect, const Color& fill) {
    if (!fb_ || fill.a <= 0.001f) return;
    
    int start_x = static_cast<int>(std::max(0.0f, rect.x));
    int start_y = static_cast<int>(std::max(0.0f, rect.y));
    int end_x = static_cast<int>(std::min(static_cast<float>(width_) - 1.0f, rect.right()));
    int end_y = static_cast<int>(std::min(static_cast<float>(height_) - 1.0f, rect.bottom()));
    
    if (!scissors_.empty()) {
        const Rect& s = scissors_.back().rect;
        start_x = std::max(start_x, static_cast<int>(s.x));
        start_y = std::max(start_y, static_cast<int>(s.y));
        end_x = std::min(end_x, static_cast<int>(s.right()));
        end_y = std::min(end_y, static_cast<int>(s.bottom()));
    }
    if (start_x > end_x || start_y > end_y) return;

    uint32_t c32 = color_to_uint32(fill);
    if (fill.a >= 0.999f) {
        for (int y = start_y; y <= end_y; ++y) {
            int row = y * width_;
            for (int x = start_x; x <= end_x; ++x) {
                fb_[row + x] = c32;
            }
        }
    } else {
        for (int y = start_y; y <= end_y; ++y) {
            int row = y * width_;
            for (int x = start_x; x <= end_x; ++x) {
                fb_[row + x] = blend_fast(fb_[row + x], c32, fill.a);
            }
        }
    }
    stats_.primitives_drawn++;
    stats_.rects_drawn++;
    stats_.pixels_drawn += static_cast<uint32_t>((end_x - start_x + 1) * (end_y - start_y + 1));
}

// SDF Rounded Rectangle — produces hyper-quality sub-pixel AA corners
void Renderer2D::draw_rect_rounded(const Rect& rect, float radius, const Color& fill) {
    if (!fb_ || fill.a <= 0.001f) {
        return;
    }
    if (radius <= 0.0f) {
        draw_rect_solid(rect, fill);
        return;
    }
    float max_r = std::min(rect.w, rect.h) * 0.5f;
    radius = std::min(radius, max_r);
    
    int start_x = static_cast<int>(std::max(0.0f, rect.x));
    int start_y = static_cast<int>(std::max(0.0f, rect.y));
    int end_x = static_cast<int>(std::min(static_cast<float>(width_) - 1.0f, rect.right()));
    int end_y = static_cast<int>(std::min(static_cast<float>(height_) - 1.0f, rect.bottom()));
    
    if (!scissors_.empty()) {
        const Rect& s = scissors_.back().rect;
        start_x = std::max(start_x, static_cast<int>(s.x));
        start_y = std::max(start_y, static_cast<int>(s.y));
        end_x = std::min(end_x, static_cast<int>(s.right()));
        end_y = std::min(end_y, static_cast<int>(s.bottom()));
    }
    if (start_x > end_x || start_y > end_y) return;
    
    uint32_t c32 = color_to_uint32(fill);
    Vec2 center = rect.center();
    Vec2 half = rect.size() * 0.5f;

    for (int y = start_y; y <= end_y; ++y) {
        int row = y * width_;
        for (int x = start_x; x <= end_x; ++x) {
            Vec2 p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            Vec2 d = Vec2(std::abs(p.x - center.x), std::abs(p.y - center.y)) - half + Vec2(radius, radius);
            
            float dist;
            if (d.x < 0.0f && d.y < 0.0f) dist = std::max(d.x, d.y) - radius;
            else dist = Vec2(std::max(d.x, 0.0f), std::max(d.y, 0.0f)).length() - radius;
            
            float alpha = 1.0f - std::clamp(dist + 0.5f, 0.0f, 1.0f);
            if (alpha > 0.0f) {
                fb_[row + x] = blend_fast(fb_[row + x], c32, fill.a * alpha);
                stats_.pixels_drawn++;
            }
        }
    }
    stats_.primitives_drawn++;
    stats_.rects_drawn++;
}

// T-118: Unified draw_rect with fill + border + corner radius
void Renderer2D::draw_rect(const Rect& rect, const Color& fill, float corner_r,
                           float border_w, const Color& border_color) {
    if (corner_r > 0.0f) {
        draw_rect_rounded(rect, corner_r, fill);
        if (border_w > 0.0f && border_color.a > 0.001f) {
            draw_rect_border(rect, corner_r, border_w, border_color);
        }
    } else {
        draw_rect_solid(rect, fill);
        if (border_w > 0.0f && border_color.a > 0.001f) {
            draw_rect_outline(rect, border_color, border_w);
        }
    }
}

// T-119: Four-edge outline with seamless corners (no overlap at joins)
void Renderer2D::draw_rect_outline(const Rect& rect, const Color& color, float thickness) {
    if (!fb_ || color.a <= 0.001f || thickness <= 0.0f) return;
    float t = thickness;
    // Top edge (full width)
    draw_rect_solid(Rect(rect.x, rect.y, rect.w, t), color);
    // Bottom edge (full width)
    draw_rect_solid(Rect(rect.x, rect.bottom() - t, rect.w, t), color);
    // Left edge (exclude corners)
    draw_rect_solid(Rect(rect.x, rect.y + t, t, rect.h - 2.0f * t), color);
    // Right edge (exclude corners)
    draw_rect_solid(Rect(rect.right() - t, rect.y + t, t, rect.h - 2.0f * t), color);
    stats_.rects_drawn++;
}

// T-118 helper: SDF border ring for rounded rects
void Renderer2D::draw_rect_border(const Rect& rect, float radius, float border_w,
                                   const Color& border_color) {
    if (!fb_ || border_color.a <= 0.001f || border_w <= 0.0f) return;
    float max_r = std::min(rect.w, rect.h) * 0.5f;
    radius = std::min(radius, max_r);
    
    int sx = static_cast<int>(std::max(0.0f, rect.x - 1.0f));
    int sy = static_cast<int>(std::max(0.0f, rect.y - 1.0f));
    int ex = static_cast<int>(std::min(static_cast<float>(width_) - 1.0f, rect.right() + 1.0f));
    int ey = static_cast<int>(std::min(static_cast<float>(height_) - 1.0f, rect.bottom() + 1.0f));
    
    if (!scissors_.empty()) {
        const Rect& s = scissors_.back().rect;
        sx = std::max(sx, static_cast<int>(s.x));
        sy = std::max(sy, static_cast<int>(s.y));
        ex = std::min(ex, static_cast<int>(s.right()));
        ey = std::min(ey, static_cast<int>(s.bottom()));
    }
    if (sx > ex || sy > ey) return;
    
    uint32_t c32 = color_to_uint32(border_color);
    Vec2 center = rect.center();
    Vec2 half = rect.size() * 0.5f;

    for (int y = sy; y <= ey; ++y) {
        int row = y * width_;
        for (int x = sx; x <= ex; ++x) {
            Vec2 p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            Vec2 dd = Vec2(std::abs(p.x - center.x), std::abs(p.y - center.y)) - half + Vec2(radius, radius);
            
            float dist;
            if (dd.x < 0.0f && dd.y < 0.0f) dist = std::max(dd.x, dd.y) - radius;
            else dist = Vec2(std::max(dd.x, 0.0f), std::max(dd.y, 0.0f)).length() - radius;
            
            // Border ring: visible between -border_w and 0 from the edge
            float outer_alpha = 1.0f - std::clamp(dist + 0.5f, 0.0f, 1.0f);
            float inner_alpha = 1.0f - std::clamp(dist + border_w + 0.5f, 0.0f, 1.0f);
            float ring = outer_alpha - inner_alpha;
            
            if (ring > 0.001f) {
                fb_[row + x] = blend_fast(fb_[row + x], c32, border_color.a * ring);
                stats_.pixels_drawn++;
            }
        }
    }
    stats_.primitives_drawn++;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Line Primitives
// ═══════════════════════════════════════════════════════════════════════════════

void Renderer2D::plot_aa(float x, float y, float c, const Color& color) {
    put_pixel_alpha(static_cast<int>(x), static_cast<int>(y), color, c);
}

// Xiaolin Wu's Anti-Aliased Line Algorithm
void Renderer2D::draw_line_wu(float x0, float y0, float x1, float y1, const Color& color) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
    
    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

    float xend = std::round(x0);
    float yend = y0 + gradient * (xend - x0);
    float xgap = rfpart(x0 + 0.5f);
    int xpxl1 = static_cast<int>(xend);
    int ypxl1 = static_cast<int>(std::floor(yend));
    
    if (steep) {
        plot_aa(static_cast<float>(ypxl1),   static_cast<float>(xpxl1), rfpart(yend) * xgap, color);
        plot_aa(static_cast<float>(ypxl1+1), static_cast<float>(xpxl1), fpart(yend)  * xgap, color);
    } else {
        plot_aa(static_cast<float>(xpxl1), static_cast<float>(ypxl1),   rfpart(yend) * xgap, color);
        plot_aa(static_cast<float>(xpxl1), static_cast<float>(ypxl1+1), fpart(yend)  * xgap, color);
    }
    float intery = yend + gradient;

    xend = std::round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    int xpxl2 = static_cast<int>(xend);
    int ypxl2 = static_cast<int>(std::floor(yend));
    if (steep) {
        plot_aa(static_cast<float>(ypxl2),   static_cast<float>(xpxl2), rfpart(yend) * xgap, color);
        plot_aa(static_cast<float>(ypxl2+1), static_cast<float>(xpxl2), fpart(yend)  * xgap, color);
    } else {
        plot_aa(static_cast<float>(xpxl2), static_cast<float>(ypxl2),   rfpart(yend) * xgap, color);
        plot_aa(static_cast<float>(xpxl2), static_cast<float>(ypxl2+1), fpart(yend)  * xgap, color);
    }
    
    if (steep) {
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
            int iy = static_cast<int>(std::floor(intery));
            plot_aa(static_cast<float>(iy),   static_cast<float>(x), rfpart(intery), color);
            plot_aa(static_cast<float>(iy+1), static_cast<float>(x), fpart(intery),  color);
            intery += gradient;
        }
    } else {
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
            int iy = static_cast<int>(std::floor(intery));
            plot_aa(static_cast<float>(x), static_cast<float>(iy),   rfpart(intery), color);
            plot_aa(static_cast<float>(x), static_cast<float>(iy+1), fpart(intery),  color);
            intery += gradient;
        }
    }
}

void Renderer2D::draw_line(const Vec2& p1, const Vec2& p2, const Color& color) {
    if (!fb_ || color.a <= 0.001f) return;
    float dx = std::abs(p2.x - p1.x);
    float dy = std::abs(p2.y - p1.y);
    if (dx < 0.5f && dy < 0.5f) return; // degenerate

    if (dx < 0.5f || dy < 0.5f) {
        // Fast axis-aligned path
        float left   = std::min(p1.x, p2.x);
        float right  = std::max(p1.x, p2.x);
        float top    = std::min(p1.y, p2.y);
        float bottom = std::max(p1.y, p2.y);
        draw_rect_solid(Rect(left, top, std::max(right - left, 1.0f), std::max(bottom - top, 1.0f)), color);
    } else {
        draw_line_wu(p1.x, p1.y, p2.x, p2.y, color);
    }
    stats_.lines_drawn++;
    stats_.primitives_drawn++;
}

// T-120: Thick lines by drawing multiple parallel Wu lines with sub-pixel offsets
void Renderer2D::draw_line_thick(const Vec2& p1, const Vec2& p2, const Color& color, float thickness) {
    if (!fb_ || color.a <= 0.001f || thickness <= 0.0f) return;
    if (thickness <= 1.0f) { draw_line(p1, p2, color); return; }
    
    Vec2 dir = (p2 - p1);
    float len = dir.length();
    if (len < 0.001f) return;
    dir = dir * (1.0f / len);
    // Perpendicular direction
    Vec2 perp(-dir.y, dir.x);
    
    float half_t = thickness * 0.5f;
    int steps = static_cast<int>(std::ceil(thickness));
    for (int i = 0; i <= steps; ++i) {
        float t = -half_t + (thickness * static_cast<float>(i) / static_cast<float>(steps));
        Vec2 offset = perp * t;
        draw_line(p1 + offset, p2 + offset, color);
    }
}

// T-121: Dashed line parametric walker
void Renderer2D::draw_dashed_line(const Vec2& a, const Vec2& b, const Color& color,
                                   float dash_len, float gap_len) {
    if (!fb_ || color.a <= 0.001f) return;
    Vec2 dir = b - a;
    float total_len = dir.length();
    if (total_len < 0.001f) return;
    
    Vec2 unit = dir * (1.0f / total_len);
    float segment = dash_len + gap_len;
    float cursor = 0.0f;
    
    while (cursor < total_len) {
        float dash_end = std::min(cursor + dash_len, total_len);
        Vec2 p1 = a + unit * cursor;
        Vec2 p2 = a + unit * dash_end;
        draw_line(p1, p2, color);
        cursor += segment;
    }
    stats_.lines_drawn++;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Shape Primitives
// ═══════════════════════════════════════════════════════════════════════════════

// SDF circle fill with sub-pixel AA
void Renderer2D::draw_circle(const Vec2& center, float radius, const Color& fill) {
    if (!fb_ || fill.a <= 0.001f || radius <= 0.0f) return;
    Rect bounding(center.x - radius - 1.0f, center.y - radius - 1.0f,
                  radius * 2.0f + 2.0f, radius * 2.0f + 2.0f);
    
    int sx = static_cast<int>(std::max(0.0f, bounding.x));
    int sy = static_cast<int>(std::max(0.0f, bounding.y));
    int ex = static_cast<int>(std::min(static_cast<float>(width_) - 1.0f, bounding.right()));
    int ey = static_cast<int>(std::min(static_cast<float>(height_) - 1.0f, bounding.bottom()));
    
    if (!scissors_.empty()) {
        const Rect& s = scissors_.back().rect;
        sx = std::max(sx, static_cast<int>(s.x));
        sy = std::max(sy, static_cast<int>(s.y));
        ex = std::min(ex, static_cast<int>(s.right()));
        ey = std::min(ey, static_cast<int>(s.bottom()));
    }
    
    uint32_t c32 = color_to_uint32(fill);
    for (int y = sy; y <= ey; ++y) {
        int row = y * width_;
        for (int x = sx; x <= ex; ++x) {
            Vec2 p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            float dist = p.distance_to(center) - radius;
            float alpha = 1.0f - std::clamp(dist + 0.5f, 0.0f, 1.0f);
            if (alpha > 0.0f) {
                fb_[row + x] = blend_fast(fb_[row + x], c32, fill.a * alpha);
                stats_.pixels_drawn++;
            }
        }
    }
    stats_.primitives_drawn++;
    stats_.circles_drawn++;
}

// T-122: Outline circle (ring) using SDF distance band
void Renderer2D::draw_circle_outline(const Vec2& center, float radius, const Color& color,
                                      float thickness) {
    if (!fb_ || color.a <= 0.001f || radius <= 0.0f) return;
    float outer_r = radius + thickness * 0.5f;
    float inner_r = radius - thickness * 0.5f;
    
    Rect bounding(center.x - outer_r - 1.0f, center.y - outer_r - 1.0f,
                  outer_r * 2.0f + 2.0f, outer_r * 2.0f + 2.0f);
    
    int sx = static_cast<int>(std::max(0.0f, bounding.x));
    int sy = static_cast<int>(std::max(0.0f, bounding.y));
    int ex = static_cast<int>(std::min(static_cast<float>(width_) - 1.0f, bounding.right()));
    int ey = static_cast<int>(std::min(static_cast<float>(height_) - 1.0f, bounding.bottom()));
    
    if (!scissors_.empty()) {
        const Rect& s = scissors_.back().rect;
        sx = std::max(sx, static_cast<int>(s.x));
        sy = std::max(sy, static_cast<int>(s.y));
        ex = std::min(ex, static_cast<int>(s.right()));
        ey = std::min(ey, static_cast<int>(s.bottom()));
    }
    
    uint32_t c32 = color_to_uint32(color);
    for (int y = sy; y <= ey; ++y) {
        int row = y * width_;
        for (int x = sx; x <= ex; ++x) {
            Vec2 p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            float dist = p.distance_to(center);
            float outer_a = 1.0f - std::clamp(dist - outer_r + 0.5f, 0.0f, 1.0f);
            float inner_a = 1.0f - std::clamp(dist - inner_r + 0.5f, 0.0f, 1.0f);
            float ring = outer_a - inner_a;
            if (ring > 0.001f) {
                fb_[row + x] = blend_fast(fb_[row + x], c32, color.a * ring);
                stats_.pixels_drawn++;
            }
        }
    }
    stats_.primitives_drawn++;
    stats_.circles_drawn++;
}

// T-123: Perfectly centered crosshair
void Renderer2D::draw_crosshair(const Vec2& center, float size, const Color& color, float thickness) {
    if (!fb_ || color.a <= 0.001f) return;
    float half = size * 0.5f;
    float ht = thickness * 0.5f;
    // Horizontal arm
    draw_rect_solid(Rect(center.x - half, center.y - ht, size, thickness), color);
    // Vertical arm
    draw_rect_solid(Rect(center.x - ht, center.y - half, thickness, size), color);
    stats_.primitives_drawn++;
}

// T-124: Diamond (45° rotated square) via scanline fill
void Renderer2D::draw_diamond(const Vec2& center, float size, const Color& fill,
                               const Color& border_color) {
    if (!fb_) return;
    float half = size * 0.5f;
    
    int sy = static_cast<int>(std::max(0.0f, center.y - half - 1.0f));
    int ey = static_cast<int>(std::min(static_cast<float>(height_) - 1.0f, center.y + half + 1.0f));
    
    for (int y = sy; y <= ey; ++y) {
        float py = static_cast<float>(y) + 0.5f;
        float dy = std::abs(py - center.y);
        float span = half - dy; // width at this scanline row
        if (span <= 0.0f) continue;
        
        int sx_row = static_cast<int>(std::max(0.0f, center.x - span));
        int ex_row = static_cast<int>(std::min(static_cast<float>(width_) - 1.0f, center.x + span));
        int row = y * width_;
        
        uint32_t fill_c = color_to_uint32(fill);
        uint32_t bord_c = color_to_uint32(border_color);
        
        for (int x = sx_row; x <= ex_row; ++x) {
            if (!clip_point(x, y)) continue;
            float px = static_cast<float>(x) + 0.5f;
            float dist = std::abs(px - center.x) + dy - half; // Manhattan diamond SDF
            
            float fill_alpha = 1.0f - std::clamp(dist + 0.5f, 0.0f, 1.0f);
            float border_alpha = fill_alpha * (1.0f - std::clamp(-dist - 1.0f, 0.0f, 1.0f));
            
            if (border_alpha > 0.001f && border_color.a > 0.001f) {
                fb_[row + x] = blend_fast(fb_[row + x], bord_c, border_color.a * border_alpha);
            }
            if (fill_alpha > border_alpha && fill.a > 0.001f) {
                fb_[row + x] = blend_fast(fb_[row + x], fill_c, fill.a * (fill_alpha - border_alpha));
            }
            stats_.pixels_drawn++;
        }
    }
    stats_.primitives_drawn++;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Grid Rendering (T-125)
// ═══════════════════════════════════════════════════════════════════════════════

void Renderer2D::draw_grid(const Rect& viewport, const Vec2& offset, float zoom,
                            float minor_step, float major_step) {
    if (!fb_ || zoom <= 0.001f) return;
    
    float scaled_minor = minor_step * zoom;
    
    // Adaptive: hide minor grid when lines are too dense
    bool draw_minor = (scaled_minor >= 4.0f);
    
    Color grid_minor_color(0.3f, 0.3f, 0.3f, 0.15f);
    Color grid_major_color(0.4f, 0.4f, 0.4f, 0.35f);
    
    // Compute range in canvas space
    float canvas_left   = (viewport.x - offset.x) / zoom;
    float canvas_right  = (viewport.right() - offset.x) / zoom;
    float canvas_top    = (viewport.y - offset.y) / zoom;
    float canvas_bottom = (viewport.bottom() - offset.y) / zoom;

    // Vertical lines
    if (draw_minor) {
        float start = std::floor(canvas_left / minor_step) * minor_step;
        for (float cx = start; cx <= canvas_right; cx += minor_step) {
            float sx = cx * zoom + offset.x;
            if (sx >= viewport.x && sx <= viewport.right()) {
                int ix = static_cast<int>(sx);
                if (ix >= 0 && ix < width_) {
                    int y_start = std::max(0, static_cast<int>(viewport.y));
                    int y_end = std::min(height_ - 1, static_cast<int>(viewport.bottom()));
                    for (int y = y_start; y <= y_end; ++y) {
                        put_pixel(ix, y, grid_minor_color);
                    }
                }
            }
        }
    }
    
    // Major vertical lines
    {
        float start = std::floor(canvas_left / major_step) * major_step;
        for (float cx = start; cx <= canvas_right; cx += major_step) {
            float sx = cx * zoom + offset.x;
            if (sx >= viewport.x && sx <= viewport.right()) {
                int ix = static_cast<int>(sx);
                if (ix >= 0 && ix < width_) {
                    int y_start = std::max(0, static_cast<int>(viewport.y));
                    int y_end = std::min(height_ - 1, static_cast<int>(viewport.bottom()));
                    for (int y = y_start; y <= y_end; ++y) {
                        put_pixel(ix, y, grid_major_color);
                    }
                }
            }
        }
    }
    
    // Horizontal lines
    if (draw_minor) {
        float start = std::floor(canvas_top / minor_step) * minor_step;
        for (float cy = start; cy <= canvas_bottom; cy += minor_step) {
            float sy = cy * zoom + offset.y;
            if (sy >= viewport.y && sy <= viewport.bottom()) {
                int iy = static_cast<int>(sy);
                if (iy >= 0 && iy < height_) {
                    int x_start = std::max(0, static_cast<int>(viewport.x));
                    int x_end = std::min(width_ - 1, static_cast<int>(viewport.right()));
                    for (int x = x_start; x <= x_end; ++x) {
                        put_pixel(x, iy, grid_minor_color);
                    }
                }
            }
        }
    }
    
    // Major horizontal lines
    {
        float start = std::floor(canvas_top / major_step) * major_step;
        for (float cy = start; cy <= canvas_bottom; cy += major_step) {
            float sy = cy * zoom + offset.y;
            if (sy >= viewport.y && sy <= viewport.bottom()) {
                int iy = static_cast<int>(sy);
                if (iy >= 0 && iy < height_) {
                    int x_start = std::max(0, static_cast<int>(viewport.x));
                    int x_end = std::min(width_ - 1, static_cast<int>(viewport.right()));
                    for (int x = x_start; x <= x_end; ++x) {
                        put_pixel(x, iy, grid_major_color);
                    }
                }
            }
        }
    }
    stats_.primitives_drawn++;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Text Primitives
// ═══════════════════════════════════════════════════════════════════════════════

void Renderer2D::draw_text(const Vec2& pos, const std::string& text, const Color& color, float scale) {
    if (!fb_ || color.a <= 0.0f || text.empty()) return;
    
    if (font_.is_loaded()) {
        font_.draw_text(fb_, width_, height_, pos.x, pos.y, text, color);
    } else {
        // Fallback to stb_easy_font if no TTF loaded
        char buffer[99999];
        int num_quads = stb_easy_font_print(0, 0, const_cast<char*>(text.c_str()),
                                             nullptr, buffer, sizeof(buffer));
        
        float* vertices = reinterpret_cast<float*>(buffer);
        for (int i = 0; i < num_quads; i++) {
            int off = i * 16;
            float x0 = vertices[off + 0], y0 = vertices[off + 1];
            float x1 = vertices[off + 4], y1 = vertices[off + 5];
            float x2 = vertices[off + 8], y2 = vertices[off + 9];
            float x3 = vertices[off + 12], y3 = vertices[off + 13];
            
            float xmin = std::min({x0, x1, x2, x3}) * scale;
            float xmax = std::max({x0, x1, x2, x3}) * scale;
            float ymin = std::min({y0, y1, y2, y3}) * scale;
            float ymax = std::max({y0, y1, y2, y3}) * scale;
            
            draw_rect_solid(Rect(pos.x + xmin, pos.y + ymin, xmax - xmin, ymax - ymin), color);
        }
    }
    stats_.text_calls++;
}

float Renderer2D::measure_text_width(const std::string& text, float scale) const {
    if (text.empty()) return 0.0f;
    if (font_.is_loaded()) return font_.measure_width(text);
    return stb_easy_font_width(const_cast<char*>(text.c_str())) * scale;
}

float Renderer2D::text_height(float scale) const { 
    if (font_.is_loaded()) return font_.pixel_height();
    return 7.0f * scale; 
}

float Renderer2D::line_height(float scale) const { 
    if (font_.is_loaded()) return font_.line_height();
    return 10.0f * scale; 
}

// ═══════════════════════════════════════════════════════════════════════════════
//  View Transform (T-127 / T-128)
// ═══════════════════════════════════════════════════════════════════════════════

void Renderer2D::set_view_transform(const Vec2& pan, float zoom) {
    view_.pan = pan;
    view_.zoom = std::max(0.01f, zoom);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Scissor Clipping
// ═══════════════════════════════════════════════════════════════════════════════

void Renderer2D::push_scissor(const Rect& rect) {
    if (!scissors_.empty()) {
        // Intersect with parent scissor for nested clipping
        const Rect& parent = scissors_.back().rect;
        float nx = std::max(rect.x, parent.x);
        float ny = std::max(rect.y, parent.y);
        float nr = std::min(rect.right(), parent.right());
        float nb = std::min(rect.bottom(), parent.bottom());
        scissors_.push_back({Rect(nx, ny, std::max(0.0f, nr - nx), std::max(0.0f, nb - ny))});
    } else {
        scissors_.push_back({rect});
    }
}

void Renderer2D::pop_scissor() {
    if (!scissors_.empty()) {
        scissors_.pop_back();
    }
}
