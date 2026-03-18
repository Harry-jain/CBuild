#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../math/vec2.hpp"
#include "../math/rect.hpp"
#include "../math/color.hpp"
#include "font_renderer.hpp"

struct Scissor {
    Rect rect;
};

struct ViewTransform {
    Vec2 pan;
    float zoom;
    
    ViewTransform() : pan(0, 0), zoom(1.0f) {}
    ViewTransform(Vec2 p, float z) : pan(p), zoom(z) {}
    
    Vec2 canvas_to_screen(Vec2 p) const { return p * zoom + pan; }
    Vec2 screen_to_canvas(Vec2 p) const { return (p - pan) * (1.0f / zoom); }
};

struct RenderStats {
    uint32_t pixels_drawn;
    uint32_t primitives_drawn;
    uint32_t draw_calls;
    uint32_t rects_drawn;
    uint32_t lines_drawn;
    uint32_t circles_drawn;
    uint32_t text_calls;
};

class Renderer2D {
public:
    Renderer2D(uint32_t* framebuffer, int width, int height);
    
    void update_buffer(uint32_t* new_fb, int new_w, int new_h);
    void shutdown();

    // Frame lifecycle
    void begin_frame(const Color& clear_color);
    void end_frame();

    bool load_font(const std::string& path, float size);

    void clear(const Color& color);
    
    // ── Core pixel operations ─────────────────────────────────
    void put_pixel(int x, int y, const Color& color);
    void put_pixel_alpha(int x, int y, const Color& color, float alpha_override);
    
    // ── Rectangle primitives ──────────────────────────────────
    void draw_rect_solid(const Rect& rect, const Color& fill);
    void draw_rect_rounded(const Rect& rect, float radius, const Color& fill);
    void draw_rect(const Rect& rect, const Color& fill, float corner_r, float border_w, const Color& border_color);
    void draw_rect_outline(const Rect& rect, const Color& color, float thickness = 1.0f);
    void draw_rect_border(const Rect& rect, float radius, float border_w, const Color& border_color);
    
    // ── Line primitives ───────────────────────────────────────
    void draw_line(const Vec2& p1, const Vec2& p2, const Color& color);
    void draw_line_thick(const Vec2& p1, const Vec2& p2, const Color& color, float thickness);
    void draw_dashed_line(const Vec2& a, const Vec2& b, const Color& color, float dash_len, float gap_len);
    
    // ── Shape primitives ──────────────────────────────────────
    void draw_circle(const Vec2& center, float radius, const Color& fill);
    void draw_circle_outline(const Vec2& center, float radius, const Color& color, float thickness = 1.0f);
    void draw_crosshair(const Vec2& center, float size, const Color& color, float thickness = 1.0f);
    void draw_diamond(const Vec2& center, float size, const Color& fill, const Color& border_color);
    
    // ── Grid rendering ────────────────────────────────────────
    void draw_grid(const Rect& viewport, const Vec2& offset, float zoom, float minor_step, float major_step);
    
    // ── Text primitives ───────────────────────────────────────
    void draw_text(const Vec2& pos, const std::string& text, const Color& color, float scale = 2.0f);
    float measure_text_width(const std::string& text, float scale = 2.0f) const;
    float text_height(float scale = 2.0f) const;
    float line_height(float scale = 2.0f) const;

    // ── View transform ────────────────────────────────────────
    void set_view_transform(const Vec2& pan, float zoom);
    const ViewTransform& get_view_transform() const { return view_; }
    Vec2 canvas_to_screen(const Vec2& p) const { return view_.canvas_to_screen(p); }
    Vec2 screen_to_canvas(const Vec2& p) const { return view_.screen_to_canvas(p); }

    // ── Scissor clipping ──────────────────────────────────────
    void push_scissor(const Rect& rect);
    void pop_scissor();

    // ── Debug overlay ─────────────────────────────────────────
    void set_debug_overlay(bool enabled) { debug_overlay_ = enabled; }
    bool debug_overlay_enabled() const { return debug_overlay_; }

    // ── Statistics ────────────────────────────────────────────
    const RenderStats& get_stats() const { return stats_; }
    void reset_stats();

    int get_width() const { return width_; }
    int get_height() const { return height_; }

private:
    uint32_t* fb_;
    int width_;
    int height_;
    
    ViewTransform view_;
    RenderStats stats_;
    std::vector<Scissor> scissors_;
    FontRenderer font_;
    bool debug_overlay_;

    inline uint32_t color_to_uint32(const Color& c) const;
    inline uint32_t blend_fast(uint32_t bg, uint32_t fg, float alpha) const;
    
    bool clip_point(int x, int y) const;
    
    // Xiaolin Wu AA internals
    void plot_aa(float x, float y, float c, const Color& color);
    float fpart(float x) const { return x - std::floor(x); }
    float rfpart(float x) const { return 1.0f - fpart(x); }
    void draw_line_wu(float x0, float y0, float x1, float y1, const Color& color);
};
