#ifdef _WIN32
#include "window.hpp"
#include <windows.h>
#include <windowsx.h>

struct WindowHandle {
    HWND hwnd;
};

static Key translate_key(WPARAM wParam) {
    if (wParam >= 'A' && wParam <= 'Z') return static_cast<Key>(static_cast<int>(Key::A) + (wParam - 'A'));
    if (wParam >= '0' && wParam <= '9') return static_cast<Key>(static_cast<int>(Key::Num0) + (wParam - '0'));
    switch (wParam) {
        case VK_ESCAPE: return Key::Escape;
        case VK_RETURN: return Key::Enter;
        case VK_TAB: return Key::Tab;
        case VK_SPACE: return Key::Space;
        case VK_BACK: return Key::Backspace;
        case VK_UP: return Key::Up;
        case VK_DOWN: return Key::Down;
        case VK_LEFT: return Key::Left;
        case VK_RIGHT: return Key::Right;
        case VK_SHIFT: return Key::Shift;
        case VK_CONTROL: return Key::Control;
        case VK_MENU: return Key::Alt;
        case VK_F1: return Key::F1;
        case VK_F2: return Key::F2;
        case VK_F3: return Key::F3;
        case VK_F4: return Key::F4;
        case VK_F5: return Key::F5;
        default: return Key::Unknown;
    }
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* win = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!win) return DefWindowProc(hwnd, uMsg, wParam, lParam);

    switch (uMsg) {
        case WM_CLOSE:
            win->_internal_close();
            return 0;
        case WM_SIZE:
            win->_internal_set_size(LOWORD(lParam), HIWORD(lParam));
            if (win->on_resize) {
                ResizeEvent e{LOWORD(lParam), HIWORD(lParam)};
                win->on_resize(e);
            }
            return 0;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            if (win->on_key) {
                KeyEvent e;
                e.key = translate_key(wParam);
                e.pressed = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);
                e.alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
                e.ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                e.shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                win->on_key(e);
            }
            break;
        }
        case WM_CHAR:
        {
            if (win->on_char) {
                CharEvent e;
                e.codepoint = static_cast<unsigned int>(wParam);
                win->on_char(e);
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            Vec2 pos((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            Vec2 old = win->get_mouse_position();
            win->_internal_update_mouse(pos);
            if (win->on_mouse_move) {
                MouseMoveEvent e;
                e.position = pos;
                e.delta = pos - old;
                win->on_mouse_move(e);
            }
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            if (win->on_mouse_button) {
                MouseButtonEvent e;
                if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP) e.button = MouseButton::Left;
                else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP) e.button = MouseButton::Right;
                else e.button = MouseButton::Middle;

                e.pressed = (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN);
                e.position = win->get_mouse_position();
                win->on_mouse_button(e);
            }
            break;
        }
        case WM_MOUSEWHEEL:
        {
            if (win->on_scroll) {
                ScrollEvent e;
                e.x_offset = 0;
                e.y_offset = (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
                win->on_scroll(e);
            }
            break;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window::Window(const std::string& title, int width, int height)
    : width_(width), height_(height), is_open_(true)
{
    handle_ = new WindowHandle();
    
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    static bool class_registered = false;
    if (!class_registered) {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = "CBuildWindow";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassEx(&wc);
        class_registered = true;
    }

    handle_->hwnd = CreateWindowEx(
        0, "CBuildWindow", title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, nullptr
    );

    SetWindowLongPtr(handle_->hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(handle_->hwnd, SW_SHOW);
    UpdateWindow(handle_->hwnd);

    // Default clear to #14120B (A, R=0x14, G=0x12, B=0x0B -> 0xFF14120B)
    framebuffer_.resize(width * height, 0xFF14120B);
}

Window::~Window() {
    if (handle_->hwnd) {
        DestroyWindow(handle_->hwnd);
    }
    delete handle_;
}

bool Window::is_open() const { return is_open_; }
void Window::_internal_close() { is_open_ = false; }
void Window::close() { is_open_ = false; }

void Window::_internal_set_size(int w, int h) {
    width_ = w;
    height_ = h;
    framebuffer_.resize(w * h, 0xFF14120B);
}

void Window::_internal_update_mouse(const Vec2& pos) {
    mouse_pos_ = pos;
}

void Window::poll_events() {
    MSG msg;
    while (PeekMessage(&msg, handle_->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

uint32_t* Window::get_framebuffer() {
    return framebuffer_.empty() ? nullptr : framebuffer_.data();
}

void Window::swap_buffers() {
    if (width_ <= 0 || height_ <= 0 || framebuffer_.empty()) return;
    
    HDC hdc = GetDC(handle_->hwnd);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width_;
    bmi.bmiHeader.biHeight = -height_; // Top-down parsing
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    SetDIBitsToDevice(
        hdc, 0, 0, width_, height_,
        0, 0, 0, height_,
        framebuffer_.data(), &bmi, DIB_RGB_COLORS
    );

    ReleaseDC(handle_->hwnd, hdc);
}

double Window::get_time() {
    static LARGE_INTEGER freq;
    static bool init = false;
    if (!init) { QueryPerformanceFrequency(&freq); init = true; }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return static_cast<double>(now.QuadPart) / static_cast<double>(freq.QuadPart);
}

bool Window::is_key_down(Key key) const {
    int vk = 0;
    if (key >= Key::A && key <= Key::Z) vk = 'A' + (static_cast<int>(key) - static_cast<int>(Key::A));
    else if (key == Key::Shift) vk = VK_SHIFT;
    else if (key == Key::Control) vk = VK_CONTROL;
    else if (key == Key::Alt) vk = VK_MENU;
    
    if (vk == 0) return false;
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool Window::is_mouse_button_down(MouseButton button) const {
    int vk = 0;
    if (button == MouseButton::Left) vk = VK_LBUTTON;
    else if (button == MouseButton::Right) vk = VK_RBUTTON;
    else if (button == MouseButton::Middle) vk = VK_MBUTTON;
    
    if (vk == 0) return false;
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void Window::set_title(const std::string& title) {
    SetWindowText(handle_->hwnd, title.c_str());
}

void Window::set_cursor(CursorShape shape) {
    LPCSTR def = IDC_ARROW;
    if (shape == CursorShape::IBeam) def = IDC_IBEAM;
    else if (shape == CursorShape::Crosshair) def = IDC_CROSS;
    else if (shape == CursorShape::Hand) def = IDC_HAND;
    else if (shape == CursorShape::ResizeEW) def = IDC_SIZEWE;
    else if (shape == CursorShape::ResizeNS) def = IDC_SIZENS;
    SetCursor(LoadCursor(nullptr, def));
}

std::string Window::get_clipboard() const {
    if (!OpenClipboard(handle_->hwnd)) return "";
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) { CloseClipboard(); return ""; }
    char* pszText = static_cast<char*>(GlobalLock(hData));
    std::string text(pszText);
    GlobalUnlock(hData);
    CloseClipboard();
    return text;
}

void Window::set_clipboard(const std::string& text) {
    if (!OpenClipboard(handle_->hwnd)) return;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    char* dest = static_cast<char*>(GlobalLock(hMem));
    std::copy(text.begin(), text.end(), dest);
    dest[text.size()] = '\0';
    GlobalUnlock(hMem);
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}
#endif
