#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>
#include "../math/vec2.hpp"
#include "../math/color.hpp"

// ═══════════════════════════════════════════════════════════════════════════════
//  CPU-based TrueType Font Renderer
//  Uses stb_truetype to bake glyph bitmaps into a RAM atlas.
//  No OpenGL — glyphs are alpha-blitted directly into the software framebuffer.
// ═══════════════════════════════════════════════════════════════════════════════

struct GlyphInfo {
    int x0, y0, x1, y1;     // Atlas sub-rectangle
    float x_offset, y_offset; // Glyph offset from baseline
    float x_advance;          // Horizontal advance
};

class FontRenderer {
public:
    FontRenderer();
    ~FontRenderer();
    
    // Load a TrueType font from file path. Returns false on error.
    bool load(const std::string& path, float pixel_height);
    
    // Check if a valid font is loaded
    bool is_loaded() const { return loaded_; }
    
    // Render text into a framebuffer at the given position
    void draw_text(uint32_t* fb, int fb_w, int fb_h,
                   float x, float y, const std::string& text,
                   const Color& color) const;
    
    // Measure text dimensions
    float measure_width(const std::string& text) const;
    float ascent() const { return ascent_; }
    float descent() const { return descent_; }
    float line_height() const { return line_height_; }
    float pixel_height() const { return pixel_height_; }

private:
    bool loaded_;
    float pixel_height_;
    float scale_;
    float ascent_, descent_, line_height_;
    
    // Glyph atlas (greyscale bitmap baked by stb_truetype)
    std::vector<uint8_t> atlas_;
    int atlas_w_, atlas_h_;
    
    // Glyph info per ASCII codepoint [32..126]
    static constexpr int FIRST_CHAR = 32;
    static constexpr int NUM_CHARS = 95; // 32..126 inclusive
    GlyphInfo glyphs_[95];
    
    // Font file data (must stay alive while using stbtt functions)
    std::vector<uint8_t> font_data_;
    
    // Alpha-blend a single glyph bitmap onto the framebuffer
    void blit_glyph(uint32_t* fb, int fb_w, int fb_h,
                    int dst_x, int dst_y,
                    int glyph_idx,
                    uint32_t packed_color, float alpha) const;
};
