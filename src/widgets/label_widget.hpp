#pragma once
#include "widget.hpp"

// T-164/165: LabelWidget with alignment and transparent background
class LabelWidget : public Widget {
public:
    LabelWidget(const std::string& id, const std::string& text = "Label")
        : Widget("Label", id), align_(Align::Left) {
        label_ = text;
        rect_ = Rect(0, 0, 200, 20);
        style_.fill = Color::transparent();
        style_.text_color = Color::text_primary();
    }
    
    void set_align(Align a) { align_ = a; mark_dirty(); }
    Align align() const { return align_; }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        if (style_.fill.a > 0.01f) {
            r.draw_rect_solid(rect_, style_.fill);
        }
        if (!label_.empty()) {
            float tw = r.measure_text_width(label_);
            float tx;
            if (align_ == Align::Center) tx = rect_.x + (rect_.w - tw) * 0.5f;
            else if (align_ == Align::Right) tx = rect_.right() - tw - style_.padding;
            else tx = rect_.x + style_.padding;
            float ty = rect_.y + (rect_.h - r.text_height()) * 0.5f;
            r.draw_text(Vec2(tx, ty), label_, style_.text_color);
        }
    }
    
    WidgetPtr clone(const std::string& new_id) const override {
        auto w = std::make_shared<LabelWidget>(new_id, label_);
        w->rect_ = rect_; w->style_ = style_; w->align_ = align_;
        return w;
    }

private:
    Align align_;
};
