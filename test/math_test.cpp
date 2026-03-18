#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "../src/math/vec2.hpp"
#include "../src/math/rect.hpp"
#include "../src/math/color.hpp"
#include "../src/math/mat3.hpp"
#include "../src/math/geom.hpp"

// Zero-dependency mini test framework
int tests_passed = 0;
int tests_failed = 0;

#define ASSERT_EQ(a, b) do { \
    if (!((a) == (b))) { \
        std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " (" << #a << " == " << #b << ")" << std::endl; \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

#define ASSERT_NEAR(a, b, epsilon) do { \
    if (std::abs((a) - (b)) > (epsilon)) { \
        std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " (" << #a << " near " << #b << ")" << std::endl; \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

#define ASSERT_TRUE(a) do { \
    if (!(a)) { \
        std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " (" << #a << ")" << std::endl; \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

void test_vec2() {
    Vec2 a(10, 5);
    Vec2 b(2, 2);
    
    Vec2 c = a + b;
    ASSERT_EQ(c.x, 12.0f);
    ASSERT_EQ(c.y, 7.0f);
    
    ASSERT_EQ(a.dot(b), 30.0f);
    
    Vec2 snapped = Vec2(13.7f, 7.2f).snapped(10.0f);
    ASSERT_EQ(snapped.x, 10.0f);
    ASSERT_EQ(snapped.y, 10.0f);
    
    Vec2 lerped = Vec2::lerp(Vec2(0,0), Vec2(10,10), 0.5f);
    ASSERT_EQ(lerped.x, 5.0f);
}

void test_rect() {
    Rect r(10, 10, 20, 20);
    ASSERT_EQ(r.right(), 30.0f);
    ASSERT_EQ(r.bottom(), 30.0f);
    
    ASSERT_TRUE(r.contains(Vec2(15, 15)));
    ASSERT_TRUE(!r.contains(Vec2(5, 5)));
    
    Rect o(25, 25, 10, 10);
    ASSERT_TRUE(r.intersects(o));
    
    Rect expanded = r.expanded(5.0f);
    ASSERT_EQ(expanded.x, 5.0f);
    ASSERT_EQ(expanded.w, 30.0f);
}

void test_color() {
    Color c = Color::hex(0xFF0000FF);
    ASSERT_EQ(c.r, 1.0f);
    ASSERT_EQ(c.g, 0.0f);
    
    Color rgb = Color::rgb(0x4F46E5);
    // 0x4F is 79/255 = 0.3098f
    ASSERT_NEAR(rgb.r, 0.3098f, 0.01f);
}

void test_mat3() {
    Mat3 t = Mat3::translation(10, 20);
    Vec2 p = t.transform_point(Vec2(0, 0));
    ASSERT_EQ(p.x, 10.0f);
    ASSERT_EQ(p.y, 20.0f);
}

void test_geom() {
    auto isect = geom::line_intersection(Vec2(0,0), Vec2(10,10), Vec2(10,0), Vec2(0,10));
    ASSERT_TRUE(isect.has_value());
    ASSERT_EQ(isect->x, 5.0f);
    ASSERT_EQ(isect->y, 5.0f);
    
    auto isect_parallel = geom::line_intersection(Vec2(0,0), Vec2(0,10), Vec2(10,0), Vec2(10,10));
    ASSERT_TRUE(!isect_parallel.has_value());
}

int main() {
    std::cout << "Running Math tests..." << std::endl;
    test_vec2();
    test_rect();
    test_color();
    test_mat3();
    test_geom();
    
    std::cout << "Tests passed: " << tests_passed << std::endl;
    std::cout << "Tests failed: " << tests_failed << std::endl;
    
    return tests_failed > 0 ? 1 : 0;
}
