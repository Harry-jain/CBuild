#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <algorithm>
#include <sstream>
#include "../math/vec2.hpp"
#include "../math/rect.hpp"
#include "../math/color.hpp"

class Renderer2D;
class Widget;
using WidgetPtr = std::shared_ptr<Widget>;

// ═══════════════════════════════════════════════════════════════════════════════
//  Style & State (T-148)
// ═══════════════════════════════════════════════════════════════════════════════

struct WidgetStyle {
    Color fill         = Color::panel_bg();
    Color hover_fill   = Color::rgb(0x2a2a2a);
    Color press_fill   = Color::rgb(0x3a3a3a);
    Color border_color = Color::panel_border();
    Color text_color   = Color::text_primary();
    float corner_radius = 0.0f;
    float border_width  = 0.0f;
    float font_size     = 1.0f;
    float padding       = 8.0f;
};

struct WidgetState {
    bool hovered   = false;
    bool pressed   = false;
    bool focused   = false;
    bool selected  = false;
    bool disabled  = false;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  Text alignment (T-164)
// ═══════════════════════════════════════════════════════════════════════════════

enum class Align { Left, Center, Right };

// ═══════════════════════════════════════════════════════════════════════════════
//  Widget Base Class (T-146 to T-160, T-187 to T-189)
// ═══════════════════════════════════════════════════════════════════════════════

class Widget {
public:
    Widget(const std::string& type_name, const std::string& id)
        : type_name_(type_name), id_(id), visible_(true), locked_(false), dirty_(true), parent_(nullptr) {}
    
    virtual ~Widget() = default;

    // ── T-146: Identity ──────────────────────────────────────────
    const std::string& id() const { return id_; }
    const std::string& type_name() const { return type_name_; }
    const std::string& label() const { return label_; }
    void set_label(const std::string& l) { label_ = l; mark_dirty(); }

    // ── T-147: Geometry ──────────────────────────────────────────
    const Rect& rect() const { return rect_; }
    void set_position(float x, float y) { rect_.x = x; rect_.y = y; mark_dirty(); }
    void set_position(Vec2 p) { rect_.x = p.x; rect_.y = p.y; mark_dirty(); }
    void set_size(float w, float h) { rect_.w = w; rect_.h = h; mark_dirty(); }
    void set_rect(const Rect& r) { rect_ = r; mark_dirty(); }
    void move_by(const Vec2& delta) { rect_.x += delta.x; rect_.y += delta.y; mark_dirty(); }

    // ── T-148: Style & State ─────────────────────────────────────
    WidgetStyle& style() { return style_; }
    const WidgetStyle& style() const { return style_; }
    WidgetState& state() { return state_; }
    const WidgetState& state() const { return state_; }

    bool visible() const { return visible_; }
    void set_visible(bool v) { visible_ = v; mark_dirty(); }
    bool locked() const { return locked_; }
    void set_locked(bool l) { locked_ = l; }

    // ── T-149: Hit Test ──────────────────────────────────────────
    virtual bool hit_test(Vec2 p) const { return visible_ && rect_.contains(p); }

    // ── T-150: Virtual Event Handlers ─────────────────────────────
    virtual bool on_mouse_enter() { return false; }
    virtual bool on_mouse_leave() { return false; }
    virtual bool on_mouse_press(Vec2 pos, int button) { (void)pos; (void)button; return false; }
    virtual bool on_mouse_release(Vec2 pos, int button) { (void)pos; (void)button; return false; }
    virtual bool on_mouse_move(Vec2 pos, Vec2 delta) { (void)pos; (void)delta; return false; }
    virtual bool on_scroll(float dx, float dy) { (void)dx; (void)dy; return false; }
    virtual bool on_key(int key, bool pressed, bool ctrl, bool shift, bool alt) {
        (void)key; (void)pressed; (void)ctrl; (void)shift; (void)alt; return false;
    }
    virtual bool on_char(unsigned int codepoint) { (void)codepoint; return false; }
    virtual bool on_focus() { state_.focused = true; mark_dirty(); return false; }
    virtual bool on_unfocus() { state_.focused = false; mark_dirty(); return false; }

    // ── T-151: Effective Fill ─────────────────────────────────────
    Color effective_fill() const {
        if (state_.disabled) return style_.fill.with_alpha(0.5f);
        if (state_.pressed)  return style_.press_fill;
        if (state_.hovered)  return style_.hover_fill;
        return style_.fill;
    }

    // ── T-152: Draw (virtual) ────────────────────────────────────
    virtual void draw(Renderer2D& r) const;
    virtual void draw_selection(Renderer2D& r) const;

    // ── T-153: Get Properties ────────────────────────────────────
    virtual std::map<std::string, std::string> get_properties() const {
        std::map<std::string, std::string> m;
        m["id"] = id_;
        m["label"] = label_;
        m["x"] = std::to_string(rect_.x);
        m["y"] = std::to_string(rect_.y);
        m["width"] = std::to_string(rect_.w);
        m["height"] = std::to_string(rect_.h);
        m["corner_radius"] = std::to_string(style_.corner_radius);
        m["border_width"] = std::to_string(style_.border_width);
        m["font_size"] = std::to_string(style_.font_size);
        m["visible"] = visible_ ? "true" : "false";
        m["locked"] = locked_ ? "true" : "false";
        return m;
    }

    // ── T-154: Set Property ──────────────────────────────────────
    virtual void set_property(const std::string& key, const std::string& value) {
        if      (key == "x")             rect_.x = std::stof(value);
        else if (key == "y")             rect_.y = std::stof(value);
        else if (key == "width")         rect_.w = std::stof(value);
        else if (key == "height")        rect_.h = std::stof(value);
        else if (key == "corner_radius") style_.corner_radius = std::stof(value);
        else if (key == "border_width")  style_.border_width = std::stof(value);
        else if (key == "font_size")     style_.font_size = std::stof(value);
        else if (key == "visible")       visible_ = (value == "true");
        else if (key == "locked")        locked_ = (value == "true");
        else if (key == "label")         label_ = value;
        mark_dirty();
    }

    // ── T-155: Children Management ───────────────────────────────
    void add_child(WidgetPtr child) {
        child->parent_ = this;
        children_.push_back(std::move(child));
        mark_dirty();
    }

    void remove_child(const std::string& child_id) {
        children_.erase(
            std::remove_if(children_.begin(), children_.end(),
                [&](const WidgetPtr& c) { return c->id() == child_id; }),
            children_.end()
        );
        mark_dirty();
    }

    const std::vector<WidgetPtr>& children() const { return children_; }
    Widget* parent() const { return parent_; }

    // ── T-156: DFS Find ──────────────────────────────────────────
    Widget* find_by_id(const std::string& target_id) {
        if (id_ == target_id) return this;
        for (auto& c : children_) {
            Widget* found = c->find_by_id(target_id);
            if (found) return found;
        }
        return nullptr;
    }

    // ── T-157: Dirty Flag Propagation ────────────────────────────
    void mark_dirty() {
        dirty_ = true;
        if (parent_) parent_->mark_dirty();
    }
    bool is_dirty() const { return dirty_; }
    void clear_dirty() { dirty_ = false; }

    // ── T-158: Callbacks ─────────────────────────────────────────
    std::function<void()> on_click;
    std::function<void(const std::string&)> on_value_changed;

    // ── T-159: Serialize ─────────────────────────────────────────
    virtual std::string serialize() const {
        auto props = get_properties();
        std::ostringstream ss;
        ss << "{\"type\":\"" << type_name_ << "\"";
        for (auto& [k, v] : props) {
            ss << ",\"" << k << "\":\"" << v << "\"";
        }
        ss << "}";
        return ss.str();
    }

    // ── T-160: Deserialize ───────────────────────────────────────
    void deserialize(const std::map<std::string, std::string>& props) {
        for (auto& [k, v] : props) {
            set_property(k, v);
        }
    }

    // ── T-187: Clone ─────────────────────────────────────────────
    virtual WidgetPtr clone(const std::string& new_id) const {
        auto w = std::make_shared<Widget>(type_name_, new_id);
        w->label_ = label_;
        w->rect_ = rect_;
        w->style_ = style_;
        w->visible_ = visible_;
        w->locked_ = locked_;
        return w;
    }

    // ── T-188: Z-order ───────────────────────────────────────────
    void bring_forward();
    void send_backward();
    void bring_to_front();
    void send_to_back();

    // ── T-189: Preferred Size ────────────────────────────────────
    virtual Vec2 measure_preferred_size(Renderer2D& r) const;

protected:
    std::string type_name_;
    std::string id_;
    std::string label_;
    Rect rect_;
    WidgetStyle style_;
    WidgetState state_;
    bool visible_;
    bool locked_;
    bool dirty_;
    Widget* parent_;
    std::vector<WidgetPtr> children_;
};
