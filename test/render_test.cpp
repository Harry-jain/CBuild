// T-115 / T-141 / T-145: Renderer safety and quality tests
#include <iostream>
#include <vector>
#include <cmath>
#include "../src/render/renderer.hpp"

int tests_passed = 0;
int tests_failed = 0;

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "FAIL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if (!((a) == (b))) { \
        std::cerr << "FAIL: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

int main() {
    std::cout << "Running extended render tests..." << std::endl;
    
    std::vector<uint32_t> fb(100 * 100);
    Renderer2D renderer(fb.data(), 100, 100);
    
    // T-104: Clear
    renderer.clear(Color::canvas_bg());
    ASSERT_TRUE(renderer.get_stats().pixels_drawn == 10000);
    
    // T-103: Out of bounds safety
    renderer.reset_stats();
    renderer.put_pixel(150, 50, Color::red());
    ASSERT_TRUE(renderer.get_stats().pixels_drawn == 0);
    renderer.put_pixel(-1, -1, Color::red());
    ASSERT_TRUE(renderer.get_stats().pixels_drawn == 0);
    
    // T-117: Frame lifecycle
    renderer.begin_frame(Color::canvas_bg());
    ASSERT_TRUE(renderer.get_stats().draw_calls == 1);
    renderer.end_frame();
    ASSERT_TRUE(renderer.get_stats().draw_calls == 2);
    
    // T-127: View transform round-trip
    ViewTransform vt(Vec2(100, 50), 2.0f);
    Vec2 original(30, 40);
    Vec2 screen = vt.canvas_to_screen(original);
    Vec2 back = vt.screen_to_canvas(screen);
    ASSERT_TRUE(std::abs(back.x - original.x) < 0.01f);
    ASSERT_TRUE(std::abs(back.y - original.y) < 0.01f);
    
    // T-129: measure_text_width
    float tw = renderer.measure_text_width("Hello");
    ASSERT_TRUE(tw > 0.0f);
    
    // T-135: text metrics
    ASSERT_TRUE(renderer.text_height() > 0.0f);
    ASSERT_TRUE(renderer.line_height() > renderer.text_height());
    
    // T-126: Nested scissor intersection
    renderer.push_scissor(Rect(10, 10, 80, 80));
    renderer.push_scissor(Rect(40, 40, 80, 80));
    renderer.reset_stats();
    renderer.put_pixel(45, 45, Color::red()); // inside intersection
    ASSERT_TRUE(renderer.get_stats().pixels_drawn == 1);
    renderer.reset_stats();
    renderer.put_pixel(15, 15, Color::red()); // outside inner scissor
    ASSERT_TRUE(renderer.get_stats().pixels_drawn == 0);
    renderer.pop_scissor();
    renderer.pop_scissor();
    
    // T-139: shutdown
    renderer.shutdown();
    ASSERT_TRUE(renderer.get_width() == 0);
    ASSERT_TRUE(renderer.get_height() == 0);
    
    std::cout << "Tests passed: " << tests_passed << "\nTests failed: " << tests_failed << std::endl;
    return tests_failed > 0 ? 1 : 0;
}
