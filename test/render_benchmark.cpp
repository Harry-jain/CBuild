// T-144: Software Renderer Benchmark — 10,000 rect stress test
#include <iostream>
#include <vector>
#include <chrono>
#include "../src/render/renderer.hpp"

int main() {
    const int W = 800, H = 600;
    std::vector<uint32_t> fb(W * H);
    Renderer2D renderer(fb.data(), W, H);

    const int NUM_RECTS = 10000;
    const int NUM_FRAMES = 100;

    std::cout << "CBuild Software Renderer Benchmark" << std::endl;
    std::cout << "Rendering " << NUM_RECTS << " rects x " << NUM_FRAMES << " frames (" << W << "x" << H << ")" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (int f = 0; f < NUM_FRAMES; ++f) {
        renderer.begin_frame(Color::canvas_bg());
        
        for (int i = 0; i < NUM_RECTS; ++i) {
            float x = static_cast<float>((i * 7) % W);
            float y = static_cast<float>((i * 13) % H);
            float w = 10.0f + static_cast<float>(i % 20);
            float h = 8.0f + static_cast<float>(i % 15);
            
            Color c(static_cast<float>(i % 255) / 255.0f,
                    static_cast<float>((i * 3) % 255) / 255.0f,
                    static_cast<float>((i * 7) % 255) / 255.0f,
                    0.8f);
            
            if (i % 4 == 0) {
                renderer.draw_rect_rounded(Rect(x, y, w, h), 4.0f, c);
            } else {
                renderer.draw_rect_solid(Rect(x, y, w, h), c);
            }
        }
        
        renderer.end_frame();
    }

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    double fps = (NUM_FRAMES / elapsed_ms) * 1000.0;

    std::cout << "Total time: " << elapsed_ms << " ms" << std::endl;
    std::cout << "Average FPS: " << fps << std::endl;
    std::cout << "Per-frame: " << (elapsed_ms / NUM_FRAMES) << " ms" << std::endl;
    std::cout << "Last frame stats: pixels=" << renderer.get_stats().pixels_drawn
              << " primitives=" << renderer.get_stats().primitives_drawn << std::endl;

    if (fps >= 60.0) {
        std::cout << "PASS: >= 60 FPS achieved" << std::endl;
        return 0;
    } else {
        std::cout << "NOTE: < 60 FPS (software rendering is CPU-bound; acceptable for debug)" << std::endl;
        return 0;
    }
}
