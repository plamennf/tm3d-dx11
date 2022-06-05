#ifdef OS_WINDOWS

#include "core.h"
#include "display.h"
#include "draw.h"
#include "input.h"

#define UNICODE
#define _UNICODE
#include <windows.h>

extern wchar_t *win32_utf8_to_utf16(char *string);

static bool is_open = true;
static int display_width;
static int display_height;
static HWND window_handle;
static WINDOWPLACEMENT window_placement = { sizeof(window_placement) };

extern Key_Info key_infos[NUM_KEYS];

static Key win32_vk_code_to_key(u32 vk_code) {
    switch (vk_code) {
    case VK_F11: return KEY_F11;
    case VK_ESCAPE: return KEY_ESCAPE;

    case VK_SPACE: return KEY_SPACE;
        
    case 'A': return KEY_A;
    case 'D': return KEY_D;
    case 'W': return KEY_W;
    case 'S': return KEY_S;
    }
    return KEY_UNKNOWN;
}

static LRESULT CALLBACK win32_window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch (msg) {
    case WM_CLOSE:
        is_open = false;
        break;

    case WM_SIZE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        
        display_width = rect.right - rect.left;
        display_height = rect.bottom - rect.top;
        
        extern bool draw_is_initted;
        if (draw_is_initted) {
            extern void resize_render_targets(int width, int height);
            resize_render_targets(display_width, display_height);
        }
        
        break;
    }

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP: {
        u32 vk_code = (u32)wparam;
        bool is_down = (lparam & (1 << 31)) == 0;

        Key key = win32_vk_code_to_key(vk_code);

        Key_Info *info = &key_infos[key];
        info->is_down = is_down;
        
        break;
    }
        
    default:
        result = DefWindowProcW(hwnd, msg, wparam, lparam);
        break;
    }
    
    return result;
}

void display_init(int width, int height, char *title) {
    WNDCLASSW wnd_class = {};
    wnd_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wnd_class.lpfnWndProc = win32_window_callback;
    wnd_class.hInstance = GetModuleHandleW(nullptr);
    wnd_class.hCursor = nullptr;
    wnd_class.lpszClassName = L"TM3DWin32WindowClass";
    ATOM result = RegisterClassW(&wnd_class);
    assert(result);

    DWORD window_style = WS_OVERLAPPEDWINDOW;

    RECT wr = { 100L, 100L, 100L + (LONG)width, 100L + (LONG)height };
    AdjustWindowRect(&wr, window_style, FALSE);

    int window_width = wr.right - wr.left;
    int window_height = wr.bottom - wr.top;

    wchar_t *wide_title = win32_utf8_to_utf16(title);
    defer { delete [] wide_title; };
    window_handle = CreateWindowExW(0, wnd_class.lpszClassName, wide_title, window_style,
                                    CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height,
                                    nullptr, nullptr, wnd_class.hInstance, nullptr);
    assert(window_handle);

    HMONITOR monitor_handle = MonitorFromWindow(window_handle, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFOEXW monitor_info = {};
    monitor_info.cbSize = sizeof(monitor_info);
    GetMonitorInfoW(monitor_handle, &monitor_info);

    int monitor_width_without_taskbar = monitor_info.rcWork.right - monitor_info.rcWork.left;
    int monitor_height_without_taskbar = monitor_info.rcWork.bottom - monitor_info.rcWork.top;

    int window_x = (monitor_width_without_taskbar - window_width) / 2;
    int window_y = (monitor_height_without_taskbar - window_height) / 2;

    SetWindowPos(window_handle, HWND_TOP, window_x, window_y, 0, 0, SWP_NOSIZE);

#ifdef _DEBUG
    ShowWindow(window_handle, SW_SHOWMAXIMIZED);
#else
    int show_code = SW_SHOWDEFAULT;
    STARTUPINFOW startup_info = {};
    GetStartupInfoW(&startup_info);
    if (startup_info.dwFlags & STARTF_USESHOWWINDOW) {
        show_code = startup_info.wShowWindow;
    }
    ShowWindow(window_handle, show_code);
#endif
}

int display_get_width() {
    return display_width;
}

int display_get_height() {
    return display_height;
}

bool display_is_open() {
    return is_open;
}

void *display_get_native_window() {
    return (void *)window_handle;
}

void display_toggle_fullscreen() {
    DWORD style = GetWindowLongW(window_handle, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(window_handle, &window_placement) &&
            GetMonitorInfoW(MonitorFromWindow(window_handle, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLongW(window_handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window_handle, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLongW(window_handle, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window_handle, &window_placement);
        SetWindowPos(window_handle, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

#endif
