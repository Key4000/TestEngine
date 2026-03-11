/*
* система событий
*/
#pragma once
#include <defines.hpp> // для TE_API и базовых типов

/*
 * Диапазоны:
 *   0x0001-0x00FF - системные события движка
 *   0x0100-0xFFFF - пользовательские события приложения
 */
enum EventCode {
  EVENT_CODE_APPLICATION_QUIT = 0x01,
  EVENT_CODE_KEY_PRESSED = 0x02,
  EVENT_CODE_KEY_RELEASED = 0x03,
  EVENT_CODE_BUTTON_PRESSED = 0x04,
  EVENT_CODE_BUTTON_RELEASED = 0x05,
  EVENT_CODE_MOUSE_MOVED = 0x06,
  EVENT_CODE_MOUSE_WHEEL = 0x07,
  EVENT_CODE_RESIZED = 0x08,
  // ... при необходимости можно добавить
  MAX_EVENT_CODE = 0xFF
};

/*
 * Контекст события — универсальная структура для передачи данных.
 * Размер: 16 байт (128 бит), копируется по значению.
 */
struct EventContext {
  union {
    // Для клавиатуры
    struct {
      u16 key_code; 
      u16 modifiers;
    } key;
    // Для кнопок мыши
    struct {
      u8 button;
    } mouse_button;
    // Для колёсика мыши
    struct {
      i8 delta;
    } mouse_wheel;
    // Для движения мыши
    struct {
      i16 x;
      i16 y;
    } mouse_move;
    // Для изменения размера окна
    struct {
      u16 width;
      u16 height;
    } window_resize;
    // Для пользовательских данных (до 16 байт)
    u8 user_data[16];
    // Резерв
    u64 raw[2];
  } data;
};

/*
 * Тип указателя на функцию-обработчик события.
 */
typedef b8 (*PFN_on_event)(u16 code, void* sender, void* listener_inst, EventContext context);

/*
 * Инициализация и завершение системы событий.
 */
TE_API b8 event_init();
TE_API void event_shutdown();

/*
 * Регистрация / отмена регистрации обработчика.
 */
TE_API b8 event_register(u16 code, void* listener, PFN_on_event on_event);
TE_API b8 event_unregister(u16 code, void* listener, PFN_on_event on_event);

/*
 * Отправка события.
 */
TE_API b8 event_fire(u16 code, void* sender, EventContext context);