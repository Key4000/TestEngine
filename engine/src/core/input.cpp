/**
 * @file input.cpp
 * @brief Реализация системы ввода (клавиатура, мышь).
 *
 * Содержит внутренние структуры состояния, функции инициализации, обновления,
 * обработки событий от платформы и опроса состояния.
 * Проверки на выход за пределы массивов опущены для максимальной производительности.
 */

#include <core/input.hpp>         // Наш заголовок с объявлениями
#include <platform/platform.hpp>  // Для platform_zero_memory
#include <core/logger.hpp>        // Для логирования
#include <core/event.hpp>         // Для генерации событий
#include <string.h>               // Для memcpy (не используется напрямую)

//==============================================================================
// Внутренние структуры (скрыты от внешнего мира)
//==============================================================================

/**
 * @brief Состояние клавиатуры.
 *
 * Хранит массив флагов для всех возможных клавиш.
 */
typedef struct keyboard_state {
    b8 keys[KEY_MAX]; /**< true – клавиша нажата, false – отпущена */
} keyboard_state;

/**
 * @brief Состояние мыши.
 *
 * Содержит текущие координаты и массив флагов кнопок.
 */
typedef struct mouse_state {
    i16 x, y;                     /**< Текущие координаты мыши (относительно окна) */
    b8 buttons[MOUSE_BUTTON_MAX]; /**< true – кнопка нажата, false – отпущена */
} mouse_state;

/**
 * @brief Полное состояние системы ввода.
 *
 * Хранит текущее и предыдущее состояние клавиатуры и мыши.
 * Позволяет определять нажатия/отпускания путём сравнения.
 */
typedef struct input_state {
    keyboard_state keyboard_current;  /**< Текущее состояние клавиатуры */
    keyboard_state keyboard_previous; /**< Состояние клавиатуры в предыдущем кадре */
    mouse_state mouse_current;        /**< Текущее состояние мыши */
    mouse_state mouse_previous;       /**< Состояние мыши в предыдущем кадре */
} input_state;

//==============================================================================
// Статические переменные (внутреннее состояние)
//==============================================================================

/**
 * @brief Флаг, указывающий, инициализирована ли система ввода.
 */
static b8 is_initialized = false;

/**
 * @brief Главная структура состояния ввода.
 */
static input_state state = {};

//==============================================================================
// Инициализация и завершение
//==============================================================================

void input_init() {
    if (is_initialized) {
        TE_LOG_WARN("Попытка инициализировать систему ввода еще раз: file->input.cpp, func->input_init");
        return;
    }

    platform_zero_memory(&state, sizeof(state));
    is_initialized = true;
    TE_LOG_INFO("Система ввода инициализирована: file->input.cpp, func->input_init");
}

void input_shutdown() {
    if (!is_initialized) {
        TE_LOG_WARN("Попытка закрыть систему ввода, когда она не инициализирована: file->input.cpp, func->input_shutdown");
        return;
    }

    is_initialized = false;
    TE_LOG_INFO("Система ввода закрыта: file->input.cpp, func->input_shutdown");
}

//==============================================================================
// Обновление состояния (вызывается один раз в кадр)
//==============================================================================

void input_update(f64 delta_time) {
    if (!is_initialized) {
        TE_LOG_ERROR("input_update вызван до инициализации системы ввода: file->input.cpp, func->input_update");
        return;
    }

    // Копируем текущее состояние клавиатуры в предыдущее
    state.keyboard_previous = state.keyboard_current;

    // Копируем текущее состояние мыши (координаты и кнопки) в предыдущее
    state.mouse_previous = state.mouse_current;

    (void)delta_time;  // параметр пока не используется
}

//==============================================================================
// Обработка событий от платформы (вызывается из platform.cpp)
//==============================================================================

/**
 * @brief Обрабатывает событие нажатия/отпускания клавиши.
 * @param key    Код клавиши (предполагается корректным).
 * @param pressed true – нажата, false – отпущена.
 *
 * @note Проверка на выход за пределы массива опущена для производительности.
 */
void input_process_key(KeyCode key, b8 pressed) {
    if (!is_initialized) return;

    // Обновляем состояние только если оно действительно изменилось
    if (state.keyboard_current.keys[key] != pressed) {
        state.keyboard_current.keys[key] = pressed;

        EventContext context = {};
        context.data.key.key_code = key;
        context.data.key.modifiers = 0;

        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, nullptr, context);
    }
}

/**
 * @brief Обрабатывает событие нажатия/отпускания кнопки мыши.
 * @param button  Код кнопки (предполагается корректным).
 * @param pressed true – нажата, false – отпущена.
 *
 * @note Проверка на выход за пределы массива опущена для производительности.
 */
void input_process_button(MouseButton button, b8 pressed) {
    if (!is_initialized) return;

    if (state.mouse_current.buttons[button] != pressed) {
        state.mouse_current.buttons[button] = pressed;

        EventContext context = {};
        context.data.mouse_button.button = button;

        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, nullptr, context);
    }
}

/**
 * @brief Обрабатывает событие перемещения мыши.
 * @param x Новая координата X.
 * @param y Новая координата Y.
 */
void input_process_mouse_move(i16 x, i16 y) {
    if (!is_initialized) return;

    // Генерируем событие только при реальном изменении позиции
    if (state.mouse_current.x != x || state.mouse_current.y != y) {
        state.mouse_current.x = x;
        state.mouse_current.y = y;

        EventContext context = {};
        context.data.mouse_move.x = x;
        context.data.mouse_move.y = y;

        event_fire(EVENT_CODE_MOUSE_MOVED, nullptr, context);
    }
}

/**
 * @brief Обрабатывает событие прокрутки колёсика мыши.
 * @param z_delta Изменение положения колёсика (обычно ±120, но нормализуется до ±1).
 */
void input_process_mouse_wheel(i8 z_delta) {
    if (!is_initialized) return;

    EventContext context = {};
    context.data.mouse_wheel.delta = z_delta;

    event_fire(EVENT_CODE_MOUSE_WHEEL, nullptr, context);
}

//==============================================================================
// Функции опроса состояния клавиатуры
//==============================================================================

b8 input_is_key_down(KeyCode key) {
    if (!is_initialized) return false;
    return state.keyboard_current.keys[key] == true;
}

b8 input_is_key_up(KeyCode key) {
    if (!is_initialized) return true;
    return state.keyboard_current.keys[key] == false;
}

b8 input_was_key_down(KeyCode key) {
    if (!is_initialized) return false;
    return state.keyboard_previous.keys[key] == true;
}

b8 input_was_key_up(KeyCode key) {
    if (!is_initialized) return true;
    return state.keyboard_previous.keys[key] == false;
}

//==============================================================================
// Функции опроса состояния мыши – позиция
//==============================================================================

void input_get_mouse_position(i32* x, i32* y) {
    if (!is_initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
    if (!is_initialized) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}

//==============================================================================
// Функции опроса состояния мыши – кнопки
//==============================================================================

b8 input_is_mouse_button_down(MouseButton button) {
    if (!is_initialized) return false;
    return state.mouse_current.buttons[button] == true;
}

b8 input_is_mouse_button_up(MouseButton button) {
    if (!is_initialized) return true;
    return state.mouse_current.buttons[button] == false;
}

b8 input_was_mouse_button_down(MouseButton button) {
    if (!is_initialized) return false;
    return state.mouse_previous.buttons[button] == true;
}

b8 input_was_mouse_button_up(MouseButton button) {
    if (!is_initialized) return true;
    return state.mouse_previous.buttons[button] == false;
}