/**
 * @file platform.cpp
 * @brief Реализация платформенного слоя для Windows.
 *
 * Содержит конкретную реализацию функций, объявленных в platform.hpp,
 * включая создание окна, обработку сообщений, управление памятью,
 * консольный вывод и работу с таймером.
 */

#include <platform/platform.hpp>
#include <core/logger.hpp>
#include <core/te_memory.hpp>
#include <core/event.hpp>
#include <core/input.hpp>
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

//==============================================================================
// Внутренние структуры и переменные
//==============================================================================

/**
 * @struct internal_state
 * @brief Внутреннее состояние платформы (скрыто от заголовочного файла).
 */
typedef struct internal_state {
    HINSTANCE h_instance; /**< Дескриптор экземпляра приложения (HINSTANCE). */
    HWND hwnd;            /**< Дескриптор созданного окна. */
} internal_state;

/**
 * @var clock_frequency
 * @brief Частота таймера QueryPerformanceCounter (количество тиков в секунду).
 * @note Инвертированное значение, используется для пересчёта тиков в секунды.
 */
static f64 clock_frequency;

/**
 * @var start_time
 * @brief Начальное значение счётчика производительности (для точки отсчёта).
 */
static LARGE_INTEGER start_time;

//==============================================================================
// Вспомогательные функции (объявлены в этом файле)
//==============================================================================

/**
 * @brief Создаёт окно приложения (вспомогательная функция).
 * @param w_data Параметры окна (размеры, заголовок).
 * @param i_state Внутреннее состояние, в которое будет сохранён HWND.
 * @return true при успешном создании окна, false при ошибке.
 */
b8 platform_create_window(window_data* w_data, internal_state* i_state);

/**
 * @brief Оконная процедура (callback, вызываемый Windows для сообщений окна).
 * @param hwnd   Дескриптор окна.
 * @param msg    Код сообщения.
 * @param w_param Дополнительная информация (зависит от msg).
 * @param l_param Дополнительная информация (зависит от msg).
 * @return Результат обработки сообщения (обычно 0, если обработано).
 */
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

//==============================================================================
// Основные функции (реализация интерфейса из platform.hpp)
//==============================================================================

b8 platform_init(platform_state* p_state, window_data* w_data) {
    /**
     * @brief Инициализирует платформенный слой.
     * @param p_state Указатель на состояние платформы (будет заполнено).
     * @param w_data  Параметры окна.
     * @return true при успехе, false при ошибке.
     *
     * Алгоритм:
     * 1. Выделить память под internal_state (через te_memory_allocate).
     * 2. Получить HINSTANCE через GetModuleHandleA.
     * 3. Создать окно через platform_create_window.
     * 4. Инициализировать таймер (QueryPerformanceFrequency/Counter).
     * 5. При ошибке создания окна освободить выделенную память.
     */
    p_state->internal_state = te_memory_allocate(sizeof(internal_state), MEMORY_TAG_APPLICATION);
    internal_state* i_state = (internal_state*)p_state->internal_state;
    i_state->h_instance = GetModuleHandleA(nullptr);

    if (!platform_create_window(w_data, i_state)) {
        te_memory_free(i_state, sizeof(internal_state), MEMORY_TAG_APPLICATION);
        return false;
    }

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return true;
}

void platform_shutdown(platform_state* p_state) {
    /**
     * @brief Завершает работу платформенного слоя.
     * @param p_state Указатель на состояние платформы.
     *
     * Уничтожает окно (DestroyWindow) и освобождает память внутреннего состояния.
     */
    internal_state* state = (internal_state*)p_state->internal_state;
    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = nullptr;
    }
    te_memory_free(state, sizeof(internal_state), MEMORY_TAG_APPLICATION);
}

b8 platform_pump_messages(platform_state* p_state) {
    /**
     * @brief Обрабатывает все ожидающие сообщения от Windows.
     * @param p_state Указатель на состояние (не используется).
     * @return true (всегда, так как окно не закрыто; при получении WM_QUIT цикл прервётся иначе).
     *
     * Использует PeekMessage для неблокирующей обработки очереди сообщений.
     * TranslateMessage и DispatchMessage передают сообщения в оконную процедуру.
     */
    MSG message;
    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    return true;
}

//==============================================================================
// Функции управления памятью (низкоуровневые обёртки)
//==============================================================================

void* platform_allocate_memory(u64 size, b8 aligned) {
    /**
     * @brief Выделяет память через malloc.
     * @param size Размер в байтах.
     * @param aligned Флаг выравнивания (игнорируется).
     * @return Указатель на выделенный блок или NULL.
     */
    return malloc(size);
}

void platform_free_memory(void* block, b8 aligned) {
    /**
     * @brief Освобождает память через free.
     * @param block Указатель на блок.
     * @param aligned Флаг выравнивания (игнорируется).
     */
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    /**
     * @brief Заполняет блок нулями через memset.
     * @param block Указатель на блок.
     * @param size Размер в байтах.
     * @return Указатель на block.
     */
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
    /**
     * @brief Копирует память через memcpy (без перекрытия).
     * @param dest   Целевой блок.
     * @param source Исходный блок.
     * @param size   Количество байт.
     * @return Указатель на dest.
     */
    return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
    /**
     * @brief Заполняет блок памяти заданным значением через memset.
     * @param dest  Целевой блок.
     * @param value Значение (конвертируется в unsigned char).
     * @param size  Размер блока.
     * @return Указатель на dest.
     */
    return memset(dest, value, size);
}

void* platform_move_memory(void* dest, const void* source, u64 size) {
    /**
     * @brief Копирует память с учётом перекрытия через memmove.
     * @param dest   Целевой блок.
     * @param source Исходный блок.
     * @param size   Количество байт.
     * @return Указатель на dest.
     */
    return memmove(dest, source, size);
}

//==============================================================================
// Функции для работы с консолью
//==============================================================================

void platform_console_write(const char* message, u8 color) {
    /**
     * @brief Выводит сообщение в стандартный вывод с цветом.
     * @param message Строка для вывода.
     * @param color   Индекс цвета (0-5), соответствующий уровню логирования.
     */
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};  // Массив атрибутов цвета консоли.
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = nullptr;
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, nullptr);
}

void platform_console_write_error(const char* error_message, u8 color) {
    /**
     * @brief Выводит сообщение об ошибке в stderr с цветом.
     * @param error_message Строка ошибки.
     * @param color         Индекс цвета.
     */
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(error_message);
    u64 length = strlen(error_message);
    LPDWORD number_written = nullptr;
    WriteConsoleA(console_handle, error_message, (DWORD)length, number_written, nullptr);
}

//==============================================================================
// Функции для работы со временем
//==============================================================================

f64 platform_get_absolute_time() {
    /**
     * @brief Возвращает абсолютное время с высоким разрешением (в секундах).
     * @return Текущее время в секундах (монотонное).
     *
     * Использует QueryPerformanceCounter и сохранённую частоту.
     */
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms) {
    /**
     * @brief Приостанавливает выполнение потока на заданное количество миллисекунд.
     * @param ms Время задержки.
     */
    Sleep(ms);
}

//==============================================================================
// Вспомогательные функции (реализация)
//==============================================================================

b8 platform_create_window(window_data* w_data, internal_state* i_state) {
    /**
     * @brief Создаёт окно Windows.
     * @param w_data Параметры окна.
     * @param i_state Внутреннее состояние (содержит HINSTANCE).
     * @return true при успехе, false при ошибке.
     *
     * Этапы:
     * - Регистрация класса окна (WNDCLASSA).
     * - Создание окна через CreateWindowExA.
     * - Отображение окна (ShowWindow).
     */
    // Регистрация класса окна
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
        MessageBoxA(nullptr, "Окно не удалось зарегистрировать: file->platform.cpp, func->platform_create_window", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Подготовка размеров окна с учётом неклиентской области
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

    // Создание окна
    HWND handle = CreateWindowExA(
        window_ex_style, "te_window_class", w_data->app_name,
        window_style, window_x, window_y,
        window_width, window_height,
        nullptr, nullptr, i_state->h_instance, nullptr);

    if (!handle) {
        MessageBoxA(nullptr, "Не удалось создать окно!: file->platform.cpp, func->platform_create_window", "Error!", MB_ICONEXCLAMATION | MB_OK);
        TE_LOG_FATAL("Не удалось создать окно!: file->platform.cpp, func->platform_create_window");
        return false;
    }

    i_state->hwnd = handle;

    // Отображение окна
    b32 should_activate = 1;  // TODO: сделать настраиваемым
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(i_state->hwnd, show_window_command_flags);

    return true;
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    /**
     * @brief Оконная процедура, обрабатывающая сообщения Windows.
     * @param hwnd   Дескриптор окна.
     * @param msg    Идентификатор сообщения.
     * @param w_param, l_param Дополнительные параметры.
     * @return Результат обработки (зависит от сообщения).
     *
     * Обрабатываемые сообщения:
     * - WM_ERASEBKGND: предотвращает мерцание фона.
     * - WM_CLOSE: инициирует событие QUIT и уничтожает окно.
     * - WM_DESTROY: завершает цикл сообщений (PostQuitMessage).
     * - WM_SIZE, WM_KEY*, WM_MOUSE*: заготовки для будущей обработки ввода.
     */
 switch (msg) 
 {
//------------------------------------------------------------------------------
// Обработка действий окна 
//------------------------------------------------------------------------------ 
  case WM_ERASEBKGND:
    return 1;  // Сообщаем, что фон стирать не нужно.

  case WM_CLOSE: 
  {
    EventContext context = {};
    event_fire(EVENT_CODE_APPLICATION_QUIT, nullptr, context);
    DestroyWindow(hwnd);
    return 0;
  }

  case WM_DESTROY:
  {
    PostQuitMessage(0);
    return 0;
  }

  case WM_SIZE:
  {
    // TODO: сгенерировать событие изменения размера окна
  }break;

//------------------------------------------------------------------------------
// Обработка кнопок клавиатуры 
//------------------------------------------------------------------------------
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP: 
  {
    // Определяем, было ли это нажатие или отпускание
    b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);

    // Извлекаем код клавиши (wParam содержит виртуальный код)
    u16 key_code = (u16)w_param;

    // Отладочный вывод (только в debug-сборке)
    #ifdef _DEBUG
    TE_LOG_DEBUG("Клавиша %s: код %d: file->platform.cpp, func->win32_process_message",
                 pressed ? "нажата" : "отпущена", key_code);
    #endif

    // Передаём событие в систему ввода
    input_process_key((KeyCode)key_code, pressed);
    break;
}
//------------------------------------------------------------------------------
// Обработка движения мыши и прокрутки колеса 
//------------------------------------------------------------------------------
  case WM_MOUSEMOVE: 
  {
    // Извлекаем координаты мыши из lParam
    i32 x = GET_X_LPARAM(l_param);
    i32 y = GET_Y_LPARAM(l_param);

    // Передаём их в систему ввода
    input_process_mouse_move((i16)x, (i16)y);
    break;
  } 

  case WM_MOUSEWHEEL: 
  {
    // Получаем сырую дельту (обычно ±120)
    i32 raw_delta = GET_WHEEL_DELTA_WPARAM(w_param);
    
    // Игнорируем нулевые значения (на всякий случай)
    if (raw_delta != 0) 
    {
      // Упрощаем до ±1
      i8 flattened = (raw_delta > 0) ? 1 : -1;
      // Передаём в систему ввода
      input_process_mouse_wheel(flattened);
    }
    break;
}

//------------------------------------------------------------------------------
// Обработка кнопок мыши (левая, правая, средняя)
//------------------------------------------------------------------------------
case WM_LBUTTONDOWN:
case WM_LBUTTONUP:
case WM_RBUTTONDOWN:
case WM_RBUTTONUP:
case WM_MBUTTONDOWN:
case WM_MBUTTONUP:
{
    // Определяем, было ли это нажатием (true) или отпусканием (false)
    b8 pressed = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN);

    // Определяем, какая именно кнопка сработала
    MouseButton button = MOUSE_BUTTON_MAX;
    if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) {
        button = MOUSE_BUTTON_LEFT;
    } else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) {
        button = MOUSE_BUTTON_RIGHT;
    } else { // WM_MBUTTONDOWN или WM_MBUTTONUP
        button = MOUSE_BUTTON_MIDDLE;
    }

    // Передаём событие в систему ввода (она сама проверит изменение и сгенерирует событие)
    if(button != MOUSE_BUTTON_MAX)
    {
      input_process_button(button, pressed);
    }
    break;
}
//------------------------------------------------------------------------------
// конец обработки кодов сообщений 
//------------------------------------------------------------------------------

return DefWindowProcA(hwnd, msg, w_param, l_param);
}