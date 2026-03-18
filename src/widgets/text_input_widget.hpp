#pragma once
#include "widget.hpp"
#include "../platform/keycodes.hpp"
#include <cmath>

// T-166 to T-170: TextInputWidget with cursor, backspace, selection stubs
class TextInputWidget : public Widget {
public:
    TextInputWidget(const std::string& id, const std::string& placeholder = "Type here...")
        : Widget("TextInput", id), placeholder_(placeholder), cursor_pos_(0),
          cursor_blink_time_(0.0), sel_start_(-1), sel_end_(-1) {
        rect_ = Rect(0, 0, 200, 32);
        style_.fill = Color::rgb(0x1a1a2e);
        style_.border_color = Color::panel_border();
        style_.text_color = Color::text_primary();
        style_.corner_radius = 6.0f;
        style_.border_width = 1.0f;
    }
    
    const std::string& value() const { return value_; }
    void set_value(const std::string& v) { value_ = v; cursor_pos_ = (int)v.size(); mark_dirty(); }
    void set_placeholder(const std::string& p) { placeholder_ = p; }
    
    void draw(Renderer2D& r) const override {
        if (!visible_) return;
        
        Color border = state_.focused ? Color::accent() : style_.border_color;
        r.draw_rect_rounded(rect_, style_.corner_radius, style_.fill);
        r.draw_rect_outline(rect_, border, style_.border_width);
        
        float tx = rect_.x + style_.padding;
        float ty = rect_.y + (rect_.h - r.text_height()) * 0.5f;
        
        if (value_.empty()) {
            r.draw_text(Vec2(tx, ty), placeholder_, style_.text_color.with_alpha(0.4f));
        } else {
            r.draw_text(Vec2(tx, ty), value_, style_.text_color);
        }
        
        // Blinking cursor when focused
        if (state_.focused) {
            float phase = std::fmod(cursor_blink_time_, 1.0f);
            if (phase < 0.5f) {
                std::string before = value_.substr(0, cursor_pos_);
                float cx = tx + r.measure_text_width(before);
                r.draw_rect_solid(Rect(cx, rect_.y + 6, 1.5f, rect_.h - 12), style_.text_color);
            }
        }
    }
    
    bool on_focus() override { state_.focused = true; cursor_blink_time_ = 0; mark_dirty(); return true; }
    bool on_unfocus() override { state_.focused = false; mark_dirty(); return true; }
    
    bool on_mouse_press(Vec2, int) override { return true; }
    
    bool on_char(unsigned int codepoint) override {
        if (!state_.focused) return false;
        if (codepoint >= 32 && codepoint < 127) {
            value_.insert(value_.begin() + cursor_pos_, static_cast<char>(codepoint));
            cursor_pos_++;
            cursor_blink_time_ = 0;
            if (on_value_changed) on_value_changed(value_);
            mark_dirty();
            return true;
        }
        return false;
    }
    
    bool on_key(int key, bool pressed, bool ctrl, bool shift, bool alt) override {
        (void)alt; (void)shift;
        if (!state_.focused || !pressed) return false;
        
        if (key == static_cast<int>(Key::Backspace)) {
            if (!value_.empty() && cursor_pos_ > 0) {
                value_.erase(cursor_pos_ - 1, 1);
                cursor_pos_--;
                cursor_blink_time_ = 0;
                if (on_value_changed) on_value_changed(value_);
                mark_dirty();
            }
            return true;
        }
        if (key == static_cast<int>(Key::Left) && cursor_pos_ > 0) {
            cursor_pos_--; cursor_blink_time_ = 0; mark_dirty(); return true;
        }
        if (key == static_cast<int>(Key::Right) && cursor_pos_ < (int)value_.size()) {
            cursor_pos_++; cursor_blink_time_ = 0; mark_dirty(); return true;
        }
        if (ctrl && key == static_cast<int>(Key::A)) {
            sel_start_ = 0; sel_end_ = (int)value_.size(); mark_dirty(); return true;
        }
        return false;
    }
    
    void update(double dt) { cursor_blink_time_ += dt; }
    
    WidgetPtr clone(const std::string& new_id) const override {
        auto w = std::make_shared<TextInputWidget>(new_id, placeholder_);
        w->rect_ = rect_; w->style_ = style_; w->value_ = value_;
        return w;
    }

private:
    std::string value_;
    std::string placeholder_;
    int cursor_pos_;
    mutable double cursor_blink_time_;
    int sel_start_, sel_end_;
};
