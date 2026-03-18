// Widget System Integration Tests (Section E)
#include <iostream>
#include <memory>
#include <vector>
#include "../src/render/renderer.hpp"
#include "../src/widgets/widget.hpp"
#include "../src/widgets/widget_factory.hpp"

int tests_passed = 0;
int tests_failed = 0;

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "FAIL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

int main() {
    std::cout << "Running Widget System tests..." << std::endl;
    
    std::vector<uint32_t> fb(100 * 100);
    Renderer2D renderer(fb.data(), 100, 100);
    
    // T-146: Identity
    ButtonWidget btn("btn_1", "Click Me");
    ASSERT_TRUE(btn.id() == "btn_1");
    ASSERT_TRUE(btn.type_name() == "Button");
    ASSERT_TRUE(btn.label() == "Click Me");
    btn.set_label("New Label");
    ASSERT_TRUE(btn.label() == "New Label");
    
    // T-147: Geometry
    btn.set_position(50, 100);
    ASSERT_TRUE(btn.rect().x == 50.0f);
    ASSERT_TRUE(btn.rect().y == 100.0f);
    btn.move_by(Vec2(10, 20));
    ASSERT_TRUE(btn.rect().x == 60.0f);
    
    // T-148: Style
    btn.style().fill = Color::red();
    ASSERT_TRUE(btn.style().fill.r == 1.0f);
    
    // T-149: Hit Test
    btn.set_position(10, 10);
    btn.set_size(50, 30);
    ASSERT_TRUE(btn.hit_test(Vec2(25, 25)));
    ASSERT_TRUE(!btn.hit_test(Vec2(0, 0)));
    btn.set_visible(false);
    ASSERT_TRUE(!btn.hit_test(Vec2(25, 25)));
    btn.set_visible(true);
    
    // T-151: Effective fill
    btn.state().pressed = true;
    Color ef = btn.effective_fill();
    ASSERT_TRUE(ef.r == btn.style().press_fill.r);
    btn.state().pressed = false;
    btn.state().hovered = true;
    ef = btn.effective_fill();
    ASSERT_TRUE(ef.r == btn.style().hover_fill.r);
    btn.state().hovered = false;
    
    // T-155: Children management
    auto child1 = std::make_shared<LabelWidget>("lbl_1", "Child 1");
    auto child2 = std::make_shared<LabelWidget>("lbl_2", "Child 2");
    PanelWidget panel("panel_1", "Test Panel");
    panel.add_child(child1);
    panel.add_child(child2);
    ASSERT_TRUE(panel.children().size() == 2);
    ASSERT_TRUE(child1->parent() == &panel);
    panel.remove_child("lbl_1");
    ASSERT_TRUE(panel.children().size() == 1);
    
    // T-156: Find by ID
    panel.add_child(std::make_shared<ButtonWidget>("nested_btn"));
    Widget* found = panel.find_by_id("nested_btn");
    ASSERT_TRUE(found != nullptr);
    ASSERT_TRUE(found->id() == "nested_btn");
    ASSERT_TRUE(panel.find_by_id("nonexistent") == nullptr);
    
    // T-157: Dirty flag propagation
    child2->clear_dirty();
    panel.clear_dirty();
    child2->mark_dirty();
    ASSERT_TRUE(child2->is_dirty());
    ASSERT_TRUE(panel.is_dirty());
    
    // T-153/154: Properties
    auto props = btn.get_properties();
    ASSERT_TRUE(props.count("id") > 0);
    ASSERT_TRUE(props.count("x") > 0);
    btn.set_property("x", "200");
    ASSERT_TRUE(btn.rect().x == 200.0f);
    
    // T-159: Serialize
    std::string json = btn.serialize();
    ASSERT_TRUE(json.find("\"type\":\"Button\"") != std::string::npos);
    ASSERT_TRUE(json.find("\"id\":\"btn_1\"") != std::string::npos);
    
    // T-186: WidgetFactory
    WidgetFactory::instance().register_builtin_widgets();
    auto created = WidgetFactory::instance().create("Button", "factory_btn");
    ASSERT_TRUE(created != nullptr);
    ASSERT_TRUE(created->type_name() == "Button");
    auto unknown = WidgetFactory::instance().create("Unknown", "u1");
    ASSERT_TRUE(unknown == nullptr);
    
    // T-187: Clone
    auto cloned = btn.clone("btn_clone");
    ASSERT_TRUE(cloned->id() == "btn_clone");
    ASSERT_TRUE(cloned->label() == btn.label());
    ASSERT_TRUE(cloned->rect().w == btn.rect().w);
    
    // T-189: Preferred size
    Vec2 pref = btn.measure_preferred_size(renderer);
    ASSERT_TRUE(pref.x > 0 && pref.y > 0);
    
    // Slider value tests
    SliderWidget slider("slider_1");
    slider.set_value(0.75f);
    ASSERT_TRUE(slider.value() == 0.75f);
    slider.set_value(1.5f);
    ASSERT_TRUE(slider.value() == 1.0f); // clamped
    
    // Checkbox toggle
    CheckboxWidget cb("cb_1");
    ASSERT_TRUE(!cb.checked());
    cb.on_mouse_press(Vec2(0,0), 0);
    ASSERT_TRUE(cb.checked());
    cb.on_mouse_press(Vec2(0,0), 0);
    ASSERT_TRUE(!cb.checked());
    
    // ProgressBar
    ProgressBarWidget pb("pb_1", 0.5f);
    ASSERT_TRUE(pb.value() == 0.5f);
    pb.set_value(1.5f);
    ASSERT_TRUE(pb.value() == 1.0f);
    
    // Console
    ConsoleWidget console("console_1");
    console.append("Line 1");
    console.append("Line 2", Color::red());
    
    // Visual draw test (must not crash)
    btn.draw(renderer);
    panel.draw(renderer);
    slider.draw(renderer);
    cb.draw(renderer);
    pb.draw(renderer);
    console.draw(renderer);
    
    std::cout << "Tests passed: " << tests_passed << "\nTests failed: " << tests_failed << std::endl;
    return tests_failed > 0 ? 1 : 0;
}
