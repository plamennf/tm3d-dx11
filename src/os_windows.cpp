#ifdef OS_WINDOWS

#include "os.h"
#include "input.h"
#include "display.h"

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <string.h>

extern Key_Info key_infos[NUM_KEYS];

wchar_t *win32_utf8_to_utf16(char *string) {
    int required_size = MultiByteToWideChar(CP_UTF8, 0, string, -1, nullptr, 0);
    if (!required_size) return nullptr;

    wchar_t *result = new wchar_t[required_size];
    memset(result, 0, required_size * sizeof(wchar_t));
    
    MultiByteToWideChar(CP_UTF8, 0, string, -1, result, required_size);
    
    return result;
}

char *win32_utf16_to_utf8(wchar_t *string) {
    int required_size = WideCharToMultiByte(CP_UTF8, 0, string, -1, nullptr, 0, nullptr, nullptr);
    if (!required_size) return nullptr;

    char *result = new char[required_size];
    memset(result, 0, required_size * sizeof(char));
    
    WideCharToMultiByte(CP_UTF8, 0, string, -1, result, required_size, nullptr, nullptr);
    
    return result;
}

Time os_get_system_time() {
    Time result = {};
    
    SYSTEMTIME system_time = {};
    GetSystemTime(&system_time);

    result.hour = system_time.wHour;
    result.minute = system_time.wMinute;
    result.second = system_time.wSecond;

    return result;
}

Time os_get_local_time() {
    Time result = {};
    
    SYSTEMTIME local_time = {};
    GetLocalTime(&local_time);

    result.hour = local_time.wHour;
    result.minute = local_time.wMinute;
    result.second = local_time.wSecond;

    return result;    
}

char *os_read_entire_file(char *filepath, s64 *out_length) {
    wchar_t *wide_filepath = win32_utf8_to_utf16(filepath);
    defer { delete [] wide_filepath; };

    HANDLE file = CreateFileW(wide_filepath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        if (out_length) *out_length = 0;
        return nullptr;
    }
    defer { CloseHandle(file); };
    
    s64 length = 0;
    GetFileSizeEx(file, (LARGE_INTEGER *)&length);

    if (out_length) *out_length = length;

    char *result = new char[length + 1];
    memset(result, 0, (length + 1) * sizeof(char));

    DWORD num_bytes_read = 0;
    ReadFile(file, result, (DWORD)length, &num_bytes_read, nullptr);

    return result;
}

void os_poll_events() {
    for (int i = 0; i < NUM_KEYS; i++) {
        key_infos[i].was_down = key_infos[i].is_down;
        key_infos[i].changed = false;
    }
    
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

bool os_file_exists(char *filepath) {
    wchar_t *wide_filepath = win32_utf8_to_utf16(filepath);
    defer { delete [] wide_filepath; };
    
    DWORD attrib = GetFileAttributesW(wide_filepath);

    return (attrib != INVALID_FILE_ATTRIBUTES &&
            !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

double os_get_time() {
    s64 perf_count;
    QueryPerformanceCounter((LARGE_INTEGER *)&perf_count);

    s64 perf_freq;
    QueryPerformanceFrequency((LARGE_INTEGER *)&perf_freq);

    return (double)perf_count / (double)perf_freq;
}

void os_setcwd(char *_dir) {
    char *dir = copy_string(_dir);
    defer { delete [] dir; };
    
    for (char *at = dir; *at; at++) {
        if (at[0] == '/') {
            at[0] = '\\';
        }
    }

    wchar_t *wide_dir = win32_utf8_to_utf16(dir);
    defer { delete [] wide_dir; };
    
    SetCurrentDirectoryW(wide_dir);
}

char *os_get_path_to_executable() {
    wchar_t *wide_result = new wchar_t[MAX_PATH];
    memset(wide_result, 0, MAX_PATH * sizeof(wchar_t));
    GetModuleFileNameW(nullptr, wide_result, MAX_PATH);
    
    char *result = win32_utf16_to_utf8(wide_result);
    for (char *at = result; *at; at++) {
        if (at[0] == '\\') {
            at[0] = '/';
        }
    }
    return result;
}

void os_hide_cursor() {
    SetCursor(nullptr);

    HWND hwnd = (HWND) display_get_native_window();
    
    RECT clip_rect;
    GetClientRect(hwnd, &clip_rect);
    ClientToScreen(hwnd, (POINT *)&clip_rect.left);
    ClientToScreen(hwnd, (POINT *)&clip_rect.right);
    ClipCursor(&clip_rect);
}

void os_show_cursor() {
    HCURSOR cursor_handle = LoadCursorW(nullptr, IDC_ARROW);
    SetCursor(cursor_handle);
    ClipCursor(nullptr);
}

#endif
