#define STB_TRUETYPE_IMPLEMENTATION
#include "../../vendor/stb/stb_truetype.h"
#include "font_renderer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

FontRenderer::FontRenderer()
    : loaded_(false), pixel_height_(0), scale_(0),
      ascent_(0), descent_(0), line_height_(0),
      atlas_w_(0), atlas_h_(0) {
    std::memset(glyphs_, 0, sizeof(glyphs_));
}

FontRenderer::~FontRenderer() = default;

bool FontRenderer::load(const std::string& path, float pixel_height) {
    // Read the font file
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    font_data_.resize(file_size);
    size_t read = fread(font_data_.data(), 1, file_size, f);
    fclose(f);
    if (static_cast<long>(read) != file_size) return false;
    
    // Initialize stb_truetype
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, font_data_.data(), stbtt_GetFontOffsetForIndex(font_data_.data(), 0))) {
        return false;
    }
    
    pixel_height_ = pixel_height;
    scale_ = stbtt_ScaleForPixelHeight(&font, pixel_height);
    
    // Get vertical metrics
    int asc, desc, lg;
    stbtt_GetFontVMetrics(&font, &asc, &desc, &lg);
    ascent_ = static_cast<float>(asc) * scale_;
    descent_ = static_cast<float>(desc) * scale_;
    line_height_ = (static_cast<float>(asc - desc + lg)) * scale_;
    
    // Bake the glyph atlas using stb_truetype's built-in baker
    // Use a large enough atlas for good quality
    atlas_w_ = 512;
    atlas_h_ = 512;
    atlas_.resize(atlas_w_ * atlas_h_);
    
    stbtt_bakedchar baked_chars[NUM_CHARS];
    int result = stbtt_BakeFontBitmap(font_data_.data(), 0, pixel_height,
                                       atlas_.data(), atlas_w_, atlas_h_,
                                       FIRST_CHAR, NUM_CHARS, baked_chars);
    if (result <= 0) {
        // Try larger atlas
        atlas_w_ = 1024;
        atlas_h_ = 1024;
        atlas_.resize(atlas_w_ * atlas_h_);
        result = stbtt_BakeFontBitmap(font_data_.data(), 0, pixel_height,
                                       atlas_.data(), atlas_w_, atlas_h_,
                                       FIRST_CHAR, NUM_CHARS, baked_chars);
    }
    
    // Store glyph info
    for (int i = 0; i < NUM_CHARS; ++i) {
        glyphs_[i].x0 = baked_chars[i].x0;
        glyphs_[i].y0 = baked_chars[i].y0;
        glyphs_[i].x1 = baked_chars[i].x1;
        glyphs_[i].y1 = baked_chars[i].y1;
        glyphs_[i].x_offset = baked_chars[i].xoff;
        glyphs_[i].y_offset = baked_chars[i].yoff;
        glyphs_[i].x_advance = baked_chars[i].xadvance;
    }
    
    loaded_ = true;
    return true;
}

void FontRenderer::blit_glyph(uint32_t* fb, int fb_w, int fb_h,
                               int dst_x, int dst_y,
                               int glyph_idx,
                               uint32_t packed_color, float alpha) const {
    const GlyphInfo& g = glyphs_[glyph_idx];
    int gw = g.x1 - g.x0;
    int gh = g.y1 - g.y0;
    
    for (int row = 0; row < gh; ++row) {
        int fy = dst_y + row;
        if (fy < 0 || fy >= fb_h) continue;
        
        for (int col = 0; col < gw; ++col) {
            int fx = dst_x + col;
            if (fx < 0 || fx >= fb_w) continue;
            
            // Sample from atlas
            uint8_t glyph_alpha = atlas_[(g.y0 + row) * atlas_w_ + (g.x0 + col)];
            if (glyph_alpha == 0) continue;
            
            float a = (static_cast<float>(glyph_alpha) / 255.0f) * alpha;
            
            uint32_t dst_idx = fy * fb_w + fx;
            uint32_t bg = fb[dst_idx];
            
            if (a >= 0.999f) {
                fb[dst_idx] = packed_color | 0xFF000000;
            } else {
                // Fast alpha-blend
                uint32_t a_uint = static_cast<uint32_t>(a * 256);
                uint32_t ia_uint = 256 - a_uint;
                uint32_t rb = (((bg & 0xFF00FF) * ia_uint) + ((packed_color & 0xFF00FF) * a_uint)) >> 8;
                uint32_t g  = (((bg & 0x00FF00) * ia_uint) + ((packed_color & 0x00FF00) * a_uint)) >> 8;
                fb[dst_idx] = (rb & 0xFF00FF) | (g & 0x00FF00) | 0xFF000000;
            }
        }
    }
}

void FontRenderer::draw_text(uint32_t* fb, int fb_w, int fb_h,
                              float x, float y, const std::string& text,
                              const Color& color) const {
    if (!loaded_ || text.empty()) return;
    
    uint32_t packed = ((static_cast<uint32_t>(color.r * 255) & 0xFF) << 16) |
                      ((static_cast<uint32_t>(color.g * 255) & 0xFF) << 8) |
                      (static_cast<uint32_t>(color.b * 255) & 0xFF);
    
    float cursor_x = x;
    float baseline_y = y + ascent_;
    
    for (char ch : text) {
        int ci = static_cast<int>(ch) - FIRST_CHAR;
        if (ci < 0 || ci >= NUM_CHARS) continue;
        
        const GlyphInfo& g = glyphs_[ci];
        int dx = static_cast<int>(std::round(cursor_x + g.x_offset));
        int dy = static_cast<int>(std::round(baseline_y + g.y_offset));
        
        blit_glyph(fb, fb_w, fb_h, dx, dy, ci, packed, color.a);
        cursor_x += g.x_advance;
    }
}

float FontRenderer::measure_width(const std::string& text) const {
    if (!loaded_ || text.empty()) return 0.0f;
    float w = 0.0f;
    for (char ch : text) {
        int ci = static_cast<int>(ch) - FIRST_CHAR;
        if (ci >= 0 && ci < NUM_CHARS) {
            w += glyphs_[ci].x_advance;
        }
    }
    return w;
}
