#pragma once
#include "widget.hpp"

// T-171/172: PanelWidget with title bar and children clipping
class PanelWidget : public Widget {
public:
    PanelWidget(const std::string& id, const std::string& title = "Panel")
        : Widget("Panel", id), title_height_(24.0f) {
        label_ = title;
        rect_ = Rect(0, 0, 300, 200);
        style_.fill = Color::panel_bg();
        style_.border_color = Color::panel_border();
        style_.text_color = Color::text_primary();
        style_.corner_radius = 8.0f;
        style_.border_width = 1.0f;
    }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        r.draw_rect_rounded(rect_, style_.corner_radius, style_.fill);
        r.draw_rect_outline(rect_, style_.border_color, style_.border_width);
        
        // Title bar
        Color title_bg = style_.fill.tinted(1.3f);
        r.draw_rect_solid(Rect(rect_.x + 1, rect_.y + 1, rect_.w - 2, title_height_), title_bg);
        
        if (!label_.empty()) {
            float ty = rect_.y + (title_height_ - r.text_height()) * 0.5f;
            r.draw_text(Vec2(rect_.x + style_.padding, ty), label_, style_.text_color);
        }
        
        // Clip children to panel body
        Rect body(rect_.x, rect_.y + title_height_, rect_.w, rect_.h - title_height_);
        r.push_scissor(body);
        for (auto& child : children_) {
            child->draw(r);
        }
        r.pop_scissor();
    }
    
    WidgetPtr clone(const std::string& new_id) const override {
        auto w = std::make_shared<PanelWidget>(new_id, label_);
        w->rect_ = rect_; w->style_ = style_;
        return w;
    }

private:
    float title_height_;
};

// T-175/176: SliderWidget with drag and click-to-position
class SliderWidget : public Widget {
public:
    SliderWidget(const std::string& id, float min_val = 0.0f, float max_val = 1.0f)
        : Widget("Slider", id), min_(min_val), max_(max_val), value_(0.5f), dragging_(false) {
        rect_ = Rect(0, 0, 200, 24);
        style_.fill = Color::rgb(0x2a2a2a);
        style_.corner_radius = 4.0f;
    }
    
    float value() const { return value_; }
    void set_value(float v) { value_ = std::clamp(v, min_, max_); mark_dirty(); }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        // Track
        float track_h = 6.0f;
        float track_y = rect_.y + (rect_.h - track_h) * 0.5f;
        r.draw_rect_rounded(Rect(rect_.x, track_y, rect_.w, track_h), 3.0f, style_.fill);
        
        // Filled portion
        float frac = (value_ - min_) / (max_ - min_);
        float fill_w = rect_.w * frac;
        r.draw_rect_rounded(Rect(rect_.x, track_y, fill_w, track_h), 3.0f, Color::accent());
        
        // Thumb
        float thumb_x = rect_.x + fill_w;
        float thumb_r = 8.0f;
        r.draw_circle(Vec2(thumb_x, rect_.y + rect_.h * 0.5f), thumb_r, Color::white());
        r.draw_circle_outline(Vec2(thumb_x, rect_.y + rect_.h * 0.5f), thumb_r, Color::accent(), 2.0f);
    }
    
    bool on_mouse_press(Vec2 pos, int) override {
        dragging_ = true;
        update_value_from_pos(pos.x); return true;
    }
    bool on_mouse_release(Vec2, int) override { dragging_ = false; return true; }
    bool on_mouse_move(Vec2 pos, Vec2) override {
        if (dragging_) { update_value_from_pos(pos.x); return true; }
        return false;
    }
    
    WidgetPtr clone(const std::string& new_id) const override {
        auto w = std::make_shared<SliderWidget>(new_id, min_, max_);
        w->rect_ = rect_; w->style_ = style_; w->value_ = value_;
        return w;
    }

private:
    float min_, max_, value_;
    bool dragging_;
    
    void update_value_from_pos(float x) {
        float frac = std::clamp((x - rect_.x) / rect_.w, 0.0f, 1.0f);
        value_ = min_ + frac * (max_ - min_);
        if (on_value_changed) on_value_changed(std::to_string(value_));
        mark_dirty();
    }
};

// T-177: CheckboxWidget
class CheckboxWidget : public Widget {
public:
    CheckboxWidget(const std::string& id, const std::string& label = "Option")
        : Widget("Checkbox", id), checked_(false) {
        label_ = label;
        rect_ = Rect(0, 0, 150, 24);
        style_.text_color = Color::text_primary();
    }
    
    bool checked() const { return checked_; }
    void set_checked(bool c) { checked_ = c; mark_dirty(); }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        float box_size = 16.0f;
        float bx = rect_.x;
        float by = rect_.y + (rect_.h - box_size) * 0.5f;
        
        Color box_fill = checked_ ? Color::accent() : Color::rgb(0x2a2a2a);
        r.draw_rect_rounded(Rect(bx, by, box_size, box_size), 3.0f, box_fill);
        r.draw_rect_outline(Rect(bx, by, box_size, box_size), Color::panel_border(), 1.0f);
        
        if (checked_) {
            // Checkmark as two lines
            r.draw_line(Vec2(bx + 3, by + 8), Vec2(bx + 7, by + 12), Color::white());
            r.draw_line(Vec2(bx + 7, by + 12), Vec2(bx + 13, by + 4), Color::white());
        }
        
        if (!label_.empty()) {
            float ty = rect_.y + (rect_.h - r.text_height()) * 0.5f;
            r.draw_text(Vec2(bx + box_size + 8, ty), label_, style_.text_color);
        }
    }
    
    bool on_mouse_press(Vec2, int) override {
        checked_ = !checked_;
        if (on_value_changed) on_value_changed(checked_ ? "true" : "false");
        mark_dirty();
        return true;
    }
    
    WidgetPtr clone(const std::string& new_id) const override {
        auto w = std::make_shared<CheckboxWidget>(new_id, label_);
        w->rect_ = rect_; w->checked_ = checked_;
        return w;
    }

private:
    bool checked_;
};

// T-178: DropdownWidget
class DropdownWidget : public Widget {
public:
    DropdownWidget(const std::string& id, const std::vector<std::string>& options = {})
        : Widget("Dropdown", id), options_(options), selected_index_(-1), open_(false) {
        rect_ = Rect(0, 0, 180, 28);
        style_.fill = Color::rgb(0x1a1a2e);
        style_.border_color = Color::panel_border();
        style_.text_color = Color::text_primary();
        style_.corner_radius = 4.0f;
        style_.border_width = 1.0f;
    }
    
    void set_options(const std::vector<std::string>& opts) { options_ = opts; mark_dirty(); }
    int selected_index() const { return selected_index_; }
    std::string selected_value() const { return (selected_index_ >= 0 && selected_index_ < (int)options_.size()) ? options_[selected_index_] : ""; }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        r.draw_rect_rounded(rect_, style_.corner_radius, style_.fill);
        r.draw_rect_outline(rect_, style_.border_color, style_.border_width);
        
        std::string display = selected_index_ >= 0 ? options_[selected_index_] : "Select...";
        float ty = rect_.y + (rect_.h - r.text_height()) * 0.5f;
        r.draw_text(Vec2(rect_.x + style_.padding, ty), display, style_.text_color);
        
        // Down arrow
        float ax = rect_.right() - 16;
        float ay = rect_.y + rect_.h * 0.5f;
        r.draw_line(Vec2(ax - 3, ay - 2), Vec2(ax, ay + 2), style_.text_color);
        r.draw_line(Vec2(ax, ay + 2), Vec2(ax + 3, ay - 2), style_.text_color);
        
        if (open_) {
            float item_h = 24.0f;
            for (int i = 0; i < (int)options_.size(); ++i) {
                float iy = rect_.bottom() + i * item_h;
                Color bg = (i == selected_index_) ? Color::accent().with_alpha(0.3f) : Color::rgb(0x1a1a2e);
                r.draw_rect_solid(Rect(rect_.x, iy, rect_.w, item_h), bg);
                r.draw_rect_outline(Rect(rect_.x, iy, rect_.w, item_h), style_.border_color, 0.5f);
                r.draw_text(Vec2(rect_.x + style_.padding, iy + (item_h - r.text_height()) * 0.5f),
                           options_[i], style_.text_color);
            }
        }
    }
    
    bool on_mouse_press(Vec2 pos, int) override {
        if (open_) {
            float item_h = 24.0f;
            for (int i = 0; i < (int)options_.size(); ++i) {
                float iy = rect_.bottom() + i * item_h;
                if (pos.y >= iy && pos.y < iy + item_h) {
                    selected_index_ = i;
                    if (on_value_changed) on_value_changed(options_[i]);
                    break;
                }
            }
            open_ = false;
        } else {
            open_ = true;
        }
        mark_dirty();
        return true;
    }

private:
    std::vector<std::string> options_;
    int selected_index_;
    mutable bool open_;
};

// T-180: ProgressBarWidget
class ProgressBarWidget : public Widget {
public:
    ProgressBarWidget(const std::string& id, float value = 0.0f)
        : Widget("ProgressBar", id), value_(value) {
        rect_ = Rect(0, 0, 200, 20);
        style_.fill = Color::rgb(0x2a2a2a);
        style_.corner_radius = 4.0f;
    }
    
    float value() const { return value_; }
    void set_value(float v) { value_ = std::clamp(v, 0.0f, 1.0f); mark_dirty(); }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        r.draw_rect_rounded(rect_, style_.corner_radius, style_.fill);
        float fill_w = rect_.w * value_;
        if (fill_w > 1.0f) {
            r.draw_rect_rounded(Rect(rect_.x, rect_.y, fill_w, rect_.h),
                               style_.corner_radius, Color::accent());
        }
    }
    
    WidgetPtr clone(const std::string& new_id) const override {
        auto w = std::make_shared<ProgressBarWidget>(new_id, value_);
        w->rect_ = rect_; w->style_ = style_;
        return w;
    }

private:
    float value_;
};

// T-185: ConsoleWidget — append-only colored text log
class ConsoleWidget : public Widget {
public:
    struct ConsoleLine { std::string text; Color color; };
    
    ConsoleWidget(const std::string& id)
        : Widget("Console", id), scroll_offset_(0.0f), max_lines_(500) {
        rect_ = Rect(0, 0, 400, 200);
        style_.fill = Color::rgb(0x0a0a0a);
        style_.corner_radius = 4.0f;
        style_.border_width = 1.0f;
        style_.border_color = Color::panel_border();
    }
    
    void append(const std::string& text, const Color& color = Color::text_primary()) {
        lines_.push_back({text, color});
        if ((int)lines_.size() > max_lines_) lines_.erase(lines_.begin());
        auto_scroll();
        mark_dirty();
    }
    
    void clear_lines() { lines_.clear(); scroll_offset_ = 0; mark_dirty(); }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        r.draw_rect_rounded(rect_, style_.corner_radius, style_.fill);
        r.draw_rect_outline(rect_, style_.border_color, style_.border_width);
        
        r.push_scissor(Rect(rect_.x + 2, rect_.y + 2, rect_.w - 4, rect_.h - 4));
        float lh = r.line_height();
        float y = rect_.y + 4 - scroll_offset_;
        for (auto& line : lines_) {
            if (y + lh > rect_.y && y < rect_.bottom()) {
                r.draw_text(Vec2(rect_.x + 6, y), line.text, line.color);
            }
            y += lh;
        }
        r.pop_scissor();
    }
    
    bool on_scroll(float, float dy) override {
        scroll_offset_ -= dy * 20.0f;
        float max_scroll = std::max(0.0f, (float)lines_.size() * 10.0f - rect_.h + 8.0f);
        scroll_offset_ = std::clamp(scroll_offset_, 0.0f, max_scroll);
        mark_dirty();
        return true;
    }

private:
    std::vector<ConsoleLine> lines_;
    float scroll_offset_;
    int max_lines_;
    
    void auto_scroll() {
        float total = (float)lines_.size() * 10.0f;
        if (total > rect_.h - 8.0f) scroll_offset_ = total - rect_.h + 8.0f;
    }
};
