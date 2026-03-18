#pragma once
#include "widget.hpp"

// T-161/162/163: ButtonWidget with indigo fill, hover/press feedback
class ButtonWidget : public Widget {
public:
    ButtonWidget(const std::string& id, const std::string& label = "Button")
        : Widget("Button", id) {
        label_ = label;
        rect_ = Rect(0, 0, 120, 36);
        style_.fill = Color::accent();
        style_.hover_fill = Color::rgb(0x6366F1);
        style_.press_fill = Color::rgb(0x3730A3);
        style_.text_color = Color::white();
        style_.corner_radius = 8.0f;
        style_.border_width = 0.0f;
    }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        r.draw_rect_rounded(rect_, style_.corner_radius, effective_fill());
        if (!label_.empty()) {
            float tw = r.measure_text_width(label_);
            float tx = rect_.x + (rect_.w - tw) * 0.5f;
            float ty = rect_.y + (rect_.h - r.text_height()) * 0.5f;
            r.draw_text(Vec2(tx, ty), label_, style_.text_color);
        }
    }
    
    bool on_mouse_enter() override { state_.hovered = true; mark_dirty(); return true; }
    bool on_mouse_leave() override { state_.hovered = false; state_.pressed = false; mark_dirty(); return true; }
    bool on_mouse_press(Vec2, int) override { state_.pressed = true; mark_dirty(); return true; }
    bool on_mouse_release(Vec2, int) override {
        if (state_.pressed && on_click) on_click();
        state_.pressed = false; mark_dirty();
        return true;
    }
    
    WidgetPtr clone(const std::string& new_id) const override {
        auto w = std::make_shared<ButtonWidget>(new_id, label_);
        w->rect_ = rect_; w->style_ = style_;
        return w;
    }
};
