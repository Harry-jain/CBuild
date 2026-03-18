#include "widget.hpp"
#include "../render/renderer.hpp"

// ── T-152: Default draw ──────────────────────────────────────────
void Widget::draw(Renderer2D& r) const {
    if (!visible_) return;
    
    if (style_.corner_radius > 0.0f) {
        r.draw_rect_rounded(rect_, style_.corner_radius, effective_fill());
    } else {
        r.draw_rect_solid(rect_, effective_fill());
    }
    
    if (style_.border_width > 0.0f && style_.border_color.a > 0.01f) {
        r.draw_rect_outline(rect_, style_.border_color, style_.border_width);
    }

    if (!label_.empty()) {
        float tw = r.measure_text_width(label_);
        float tx = rect_.x + (rect_.w - tw) * 0.5f;
        float ty = rect_.y + (rect_.h - r.text_height()) * 0.5f;
        r.draw_text(Vec2(tx, ty), label_, style_.text_color);
    }
    
    for (auto& child : children_) {
        child->draw(r);
    }
}

// ── T-152: Selection overlay ─────────────────────────────────────
void Widget::draw_selection(Renderer2D& r) const {
    r.draw_rect_outline(rect_, Color::selection_border(), 1.5f);
    
    const float hs = 4.0f; // half-size of handle
    Vec2 handles[8] = {
        {rect_.x,                  rect_.y},                   // TL
        {rect_.x + rect_.w * 0.5f, rect_.y},                   // TC
        {rect_.right(),            rect_.y},                   // TR
        {rect_.right(),            rect_.y + rect_.h * 0.5f},  // MR
        {rect_.right(),            rect_.bottom()},            // BR
        {rect_.x + rect_.w * 0.5f, rect_.bottom()},            // BC
        {rect_.x,                  rect_.bottom()},            // BL
        {rect_.x,                  rect_.y + rect_.h * 0.5f},  // ML
    };
    
    for (auto& h : handles) {
        r.draw_rect_solid(Rect(h.x - hs, h.y - hs, hs * 2, hs * 2), Color::white());
        r.draw_rect_outline(Rect(h.x - hs, h.y - hs, hs * 2, hs * 2), Color::selection_border(), 1.0f);
    }
}

// ── T-188: Z-order operations ────────────────────────────────────
void Widget::bring_to_front() {
    if (!parent_) return;
    auto& siblings = parent_->children_;
    for (auto it = siblings.begin(); it != siblings.end(); ++it) {
        if (it->get() == this) {
            auto ptr = *it;
            siblings.erase(it);
            siblings.push_back(ptr);
            return;
        }
    }
}

void Widget::send_to_back() {
    if (!parent_) return;
    auto& siblings = parent_->children_;
    for (auto it = siblings.begin(); it != siblings.end(); ++it) {
        if (it->get() == this) {
            auto ptr = *it;
            siblings.erase(it);
            siblings.insert(siblings.begin(), ptr);
            return;
        }
    }
}

void Widget::bring_forward() {
    if (!parent_) return;
    auto& siblings = parent_->children_;
    for (size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i].get() == this && i + 1 < siblings.size()) {
            std::swap(siblings[i], siblings[i + 1]);
            return;
        }
    }
}

void Widget::send_backward() {
    if (!parent_) return;
    auto& siblings = parent_->children_;
    for (size_t i = 1; i < siblings.size(); ++i) {
        if (siblings[i].get() == this) {
            std::swap(siblings[i], siblings[i - 1]);
            return;
        }
    }
}

// ── T-189: Preferred Size ────────────────────────────────────────
Vec2 Widget::measure_preferred_size(Renderer2D& r) const {
    if (label_.empty()) return Vec2(rect_.w, rect_.h);
    float tw = r.measure_text_width(label_);
    float th = r.text_height();
    return Vec2(tw + style_.padding * 2.0f, th + style_.padding * 2.0f);
}
