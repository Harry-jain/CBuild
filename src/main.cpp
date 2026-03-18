#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include "platform/window.hpp"
#include "platform/events.hpp"
#include "render/renderer.hpp"
#include "widgets/widget_factory.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main() {
    Window window("CBuild IDE Workstation", 900, 650);
    
    WidgetFactory::instance().register_builtin_widgets();

    Renderer2D renderer(nullptr, 0, 0);
    
    // Load modern font (Windows system font)
    if (!renderer.load_font("C:\\Windows\\Fonts\\arial.ttf", 16.0f)) {
        std::cerr << "Warning: Could not load Arial font. Falling back to bitmap font." << std::endl;
    }

    double start_time = Window::get_time();
    int frame_count = 0;
    double fps = 0.0;
    
    // ── Create widgets ─────────────────────────────────────────────
    auto panel = std::make_shared<PanelWidget>("main_panel", "Widget Showcase");
    panel->set_rect(Rect(20, 20, 420, 600));
    
    auto btn1 = std::make_shared<ButtonWidget>("btn_primary", "Primary Action");
    btn1->set_rect(Rect(40, 65, 160, 36));
    
    auto btn2 = std::make_shared<ButtonWidget>("btn_secondary", "Secondary");
    btn2->set_rect(Rect(215, 65, 140, 36));
    btn2->style().fill = Color::rgb(0x374151);
    btn2->style().hover_fill = Color::rgb(0x4B5563);
    btn2->style().press_fill = Color::rgb(0x1F2937);
    
    auto input1 = std::make_shared<TextInputWidget>("input_name", "Enter your name...");
    input1->set_rect(Rect(40, 120, 360, 34));
    
    auto slider = std::make_shared<SliderWidget>("opacity_slider", 0.0f, 100.0f);
    slider->set_rect(Rect(40, 175, 360, 28));
    slider->set_value(65.0f);
    
    auto slider_label = std::make_shared<LabelWidget>("slider_lbl", "Opacity: 65%");
    slider_label->set_rect(Rect(40, 208, 360, 20));
    slider_label->style().text_color = Color::text_secondary();
    
    auto cb1 = std::make_shared<CheckboxWidget>("cb_dark", "Dark Mode");
    cb1->set_rect(Rect(40, 240, 180, 28));
    cb1->set_checked(true);
    
    auto cb2 = std::make_shared<CheckboxWidget>("cb_snap", "Enable Grid Snap");
    cb2->set_rect(Rect(230, 240, 180, 28));
    
    auto progress = std::make_shared<ProgressBarWidget>("pb_build", 0.73f);
    progress->set_rect(Rect(40, 285, 360, 20));
    
    auto progress_label = std::make_shared<LabelWidget>("pb_lbl", "Build Progress: 73%");
    progress_label->set_rect(Rect(40, 310, 360, 20));
    progress_label->style().text_color = Color::text_secondary();
    
    auto console = std::make_shared<ConsoleWidget>("build_console");
    console->set_rect(Rect(460, 20, 420, 600));
    console->append("[CBuild] Widget system initialized", Color::snap_center());
    console->append("[CBuild] 9 widget types registered", Color::snap_center());
    console->append("[CBuild] Software Renderer: ACTIVE", Color::snap_center());
    console->append("", Color::text_primary());
    console->append("--- Build Output ---", Color::accent());
    console->append("[1/5] Compiling main.cpp", Color::text_primary());
    console->append("[2/5] Compiling renderer.cpp", Color::text_primary());
    console->append("[3/5] Compiling widget.cpp", Color::text_primary());
    console->append("[4/5] Compiling window_win32.cpp", Color::text_primary());
    console->append("[5/5] Linking CBuild.exe", Color::text_primary());
    console->append("[OK] Build succeeded (0 errors)", Color::snap_center());
    console->append("", Color::text_primary());
    console->append("--- Click buttons to test! ---", Color::accent());

    // Collect all interactive widgets for event routing
    std::vector<std::shared_ptr<Widget>> widgets = {
        panel, btn1, btn2, input1, slider, slider_label,
        cb1, cb2, progress, progress_label, console
    };
    
    Widget* hovered_widget = nullptr;
    Widget* focused_widget = nullptr;
    bool mouse_pressed = false;
    
    // ── Wire slider callback ───────────────────────────────────────
    slider->on_value_changed = [&slider_label](const std::string& val) {
        int v = static_cast<int>(std::stof(val));
        slider_label->set_label("Opacity: " + std::to_string(v) + "%");
    };
    
    // ── Wire button callbacks ──────────────────────────────────────
    btn1->on_click = [&console, &progress]() {
        static float prog = 0.73f;
        prog = std::fmod(prog + 0.1f, 1.01f);
        progress->set_value(prog);
        console->append("[Action] Primary button clicked! Progress: " 
                        + std::to_string(static_cast<int>(prog * 100)) + "%", Color::snap_point());
    };
    
    btn2->on_click = [&console]() {
        console->append("[Action] Secondary button clicked!", Color::text_secondary());
    };
    
    cb1->on_value_changed = [&console](const std::string& val) {
        console->append("[Toggle] Dark Mode: " + val, Color::snap_track());
    };
    
    cb2->on_value_changed = [&console](const std::string& val) {
        console->append("[Toggle] Grid Snap: " + val, Color::snap_track());
    };

    // ── Event routing callbacks ────────────────────────────────────
    window.on_mouse_move = [&](const MouseMoveEvent& e) {
        Vec2 pos = e.position;
        Widget* new_hover = nullptr;
        
        // If a widget is being dragged, keep sending to it
        if (mouse_pressed && hovered_widget) {
            hovered_widget->on_mouse_move(pos, e.delta);
            return;
        }
        
        // Hit test back-to-front (last in vector = highest priority)
        for (int i = (int)widgets.size() - 1; i >= 0; --i) {
            if (widgets[i]->hit_test(pos)) {
                new_hover = widgets[i].get();
                break;
            }
        }
        
        if (new_hover != hovered_widget) {
            if (hovered_widget) hovered_widget->on_mouse_leave();
            hovered_widget = new_hover;
            if (hovered_widget) hovered_widget->on_mouse_enter();
        }
        
        if (hovered_widget) {
            hovered_widget->on_mouse_move(pos, e.delta);
        }
    };
    
    window.on_mouse_button = [&](const MouseButtonEvent& e) {
        Vec2 pos = e.position;
        if (e.pressed) {
            mouse_pressed = true;
            // Find widget under cursor
            Widget* target = nullptr;
            for (int i = (int)widgets.size() - 1; i >= 0; --i) {
                if (widgets[i]->hit_test(pos)) {
                    target = widgets[i].get();
                    break;
                }
            }
            
            // Focus management
            if (focused_widget && focused_widget != target) {
                focused_widget->on_unfocus();
            }
            if (target) {
                target->on_focus();
                target->on_mouse_press(pos, static_cast<int>(e.button));
                focused_widget = target;
                hovered_widget = target;
            } else {
                focused_widget = nullptr;
            }
        } else {
            mouse_pressed = false;
            if (hovered_widget) {
                hovered_widget->on_mouse_release(pos, static_cast<int>(e.button));
            }
        }
    };
    
    window.on_key = [&](const KeyEvent& e) {
        if (e.pressed && (e.key == Key::Escape || e.key == Key::Q)) {
            if (!focused_widget) { window.close(); return; }
        }
        if (e.pressed && e.key == Key::Escape && focused_widget) {
            focused_widget->on_unfocus();
            focused_widget = nullptr;
            return;
        }
        if (focused_widget) {
            focused_widget->on_key(static_cast<int>(e.key), e.pressed, e.ctrl, e.shift, e.alt);
        }
    };
    
    window.on_char = [&](const CharEvent& e) {
        if (focused_widget) {
            focused_widget->on_char(e.codepoint);
        }
    };
    
    window.on_scroll = [&](const ScrollEvent& e) {
        if (hovered_widget) {
            hovered_widget->on_scroll(e.x_offset, e.y_offset);
        }
    };

    // ── Main loop ──────────────────────────────────────────────────
    while (window.is_open()) {
        window.poll_events();

        uint32_t* fb = window.get_framebuffer();
        int w = window.get_width();
        int h = window.get_height();
        
        if (fb && w > 0 && h > 0) {
            renderer.update_buffer(fb, w, h);
            renderer.begin_frame(Color::canvas_bg());
            
            // Grid background
            renderer.draw_grid(Rect(0, 0, (float)w, (float)h), Vec2(0, 0), 1.0f, 20.0f, 100.0f);
            
            // Draw all widgets
            for (auto& widget : widgets) {
                widget->draw(renderer);
            }
            
            // FPS HUD
            frame_count++;
            double now = Window::get_time();
            if (now - start_time >= 1.0) {
                fps = frame_count / (now - start_time);
                frame_count = 0;
                start_time = now;
            }
            std::string hud = "FPS: " + std::to_string((int)fps)
                + "  Pixels: " + std::to_string(renderer.get_stats().pixels_drawn);
            renderer.draw_text(Vec2(10, (float)h - 20), hud, Color::snap_track());
            
            renderer.end_frame();
        }

        window.swap_buffers();
        
        #ifdef _WIN32
        Sleep(1);
        #else
        usleep(1000);
        #endif
    }

    renderer.shutdown();
    return 0;
}
