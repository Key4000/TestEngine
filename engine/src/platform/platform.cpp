/*
 * Реализация интерфейса платформенного
 * слоя
 */

#include <platform/platform.hpp>
#include <core/logger.hpp>

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct internal_state {
    HINSTANCE h_instance;  // экземпляр приложения
    HWND hwnd;             // дескриптор окна
} internal_state;

static f64 clock_frequency;  // частота таймера
static LARGE_INTEGER start_time;

b8 platform_create_window(window_data* w_data, internal_state* i_state);
// оконная процедура(это callback которую windows вызывает на каждое сообщение для окна
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

//------------------------------------
//     Основные функции
//------------------------------------
b8 platform_init(platform_state* p_state, window_data* w_data) {
    /*
     *
     */
    p_state->internal_state = malloc(sizeof(internal_state));
    internal_state* i_state = (internal_state*)p_state->internal_state;
    i_state->h_instance = GetModuleHandleA(nullptr);
    /*
     * создаем окно
     */
    if (!platform_create_window(w_data, i_state)) {
        return false;
    }
    /*
     * инициализация таймера
     */
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return true;
}

void platform_shutdown(platform_state* p_state) {
    internal_state* state = (internal_state*)p_state->internal_state;
    /*
     * удаляем окно
     */
    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = nullptr;
    }
};

b8 platform_pump_messages(platform_state* p_state) {
    MSG message;

    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
        // Преобразует сообщения клавиатуры
        // WM_KEYDOWN/WM_KEYUP → WM_CHAR
        TranslateMessage(&message);
        // передает сообщение в вашу оконную процедуру(win32_procces_message)
        DispatchMessageA(&message);
    }

    return true;
};
//------------------------------------
//     Функции управления памяти
//------------------------------------
void* platform_allocate_memory(u64 size, b8 aligned) {
    return malloc(size);
}
void platform_free_memory(void* block, b8 aligned) {
    free(block);
}
void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}
void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}
void* platform_set_memory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}
//------------------------------------
//   Функции для работы с консолью
//------------------------------------
void platform_console_write(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};

    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = nullptr;

    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, nullptr);
}
void platform_console_write_error(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};

    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = nullptr;

    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, nullptr);
}
//------------------------------------
//  Функции для работы со временем
//------------------------------------
f64 platform_get_absolute_time() {
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}
void platform_sleep(u64 ms) {
    Sleep(ms);
}
//------------------------------------
//  Вспомагательные функции
//------------------------------------
b8 platform_create_window(window_data* w_data, internal_state* i_state) {
    /*
     * Настройка и регистрация
     * класса окна
     */
    HICON icon = LoadIcon(i_state->h_instance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = i_state->h_instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = "te_window_class";
    if (!RegisterClassA(&wc)) {
        MessageBoxA(nullptr, "Окно неудалось зарегестрировать: file->platform.cpp, func->platform_create_window", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    /*
     * создаем окно
     */
    u32 client_x = w_data->x;
    u32 client_y = w_data->y;
    u32 client_width = w_data->width;
    u32 client_height = w_data->height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 window_ex_style = WS_EX_APPWINDOW;
    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);
    window_x += border_rect.left;
    window_y += border_rect.top;
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(
        window_ex_style, "te_window_class", w_data->app_name,
        window_style, window_x, window_y,
        window_width, window_height,
        nullptr, nullptr, i_state->h_instance, nullptr);
    if (handle == nullptr) {
        MessageBoxA(nullptr, "Неудалось создать окно!: file->platform.cpp, func->platform_create_window", "Error!", MB_ICONEXCLAMATION | MB_OK);
        TE_LOG_FATAL("Неудалось создать окно!: file->platform.cpp, func->platform_create_window");
        return false;
    } else {
        i_state->hwnd = handle;
    }
    /* Код отображение
     *  и активацией окна
     */
    b32 should_activate = 1;  // TODO: если окно не должно принимать ввод должно быть false
    // если окно активировано, то показать и активировать, если нет, то показать, но не активировать
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(i_state->hwnd, show_window_command_flags);

    return true;
};

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to prevent flicker.
            return 1;
        case WM_CLOSE:
            // TODO: Fire an event for the application to quit.
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            // Get the updated size.
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;

            // TODO: Fire an event for window resize.
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            // b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            // TODO: input processing

        } break;
        case WM_MOUSEMOVE: {
            // Mouse move
            // i32 x_position = GET_X_LPARAM(l_param);
            // i32 y_position = GET_Y_LPARAM(l_param);
            // TODO: input processing.
        } break;
        case WM_MOUSEWHEEL: {
            // i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            // if (z_delta != 0) {
            //     // Flatten the input to an OS-independent (-1, 1)
            //     z_delta = (z_delta < 0) ? -1 : 1;
            //     // TODO: input processing.
            // }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            // b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            //  TODO: input processing.
        } break;
    }
    return DefWindowProcA(hwnd, msg, w_param, l_param);
}