#include "core/event.hpp"
#include "core/te_memory.hpp"
#include "containers/darray.hpp"
#include "core/logger.hpp"

/*
 * Структура зарегистрированного события.
 */
struct RegisteredEvent {
  //Хранит связку "слушатель + функция-обработчик".
  void* listener;
  PFN_on_event callback;
};
/*
 * Запись для кода события.
 */
struct EventCodeEntry {
  // динамический массив зарегистрированных обработчиков.
  RegisteredEvent* events;
};
/*
 * Состояние системы событий.
 */
struct EventSystemState {
  // Таблица зарегистрированных событий, индексированная по коду события
  EventCodeEntry registered[MAX_EVENT_CODE];
};
/*
 * Внутреннее состояние системы событий.
 */
static b8 is_initialized = false; // Флаг инициализации системы
static EventSystemState state; // Экземпляр состояния системы
//-----------------------------------
b8 event_init() {
  if (is_initialized) {
    return false;
  }
  // Сначала сбрасываем флаг (на случай ошибки)
  is_initialized = false;

  // Обнуляем всю память состояния
  te_memory_zero(&state, sizeof(state));

  is_initialized = true;
  return true;
}
//-----------------------------------
void event_shutdown() {
  // Проходим по всем возможным кодам событий
  for (u16 i = 0; i < MAX_EVENT_CODE; ++i) {
    if (state.registered[i].events) {
      // Уничтожаем динамический массив обработчиков
      darray_destroy(state.registered[i].events);
      state.registered[i].events = nullptr;
    }
  }
  is_initialized = false;
}
//-----------------------------------
/*
 * Параметры:
 *   code      - код события для прослушивания
 *   listener  - указатель на объект-слушатель
 *   on_event  - функция-обработчик
 *
 * Возвращает:
 *   true  - успешная регистрация
 *   false - система не инициализирована, дубликат или ошибка
 */
b8 event_register(u16 code, void* listener, PFN_on_event on_event) {
  // 1. Проверяем, инициализирована ли система событий
  if (!is_initialized) {
    TE_LOG_ERROR("Попытка регистрации события при неинициализированной системе: file->event.cpp,func->event_register");
    return false;
  }

  // 2. Проверяем, что код события не выходит за допустимые пределы
  if (code >= MAX_EVENT_CODE) {
    TE_LOG_ERROR("Код события %d выходит за пределы (макс %d): file->event.cpp,func->event_register", code, MAX_EVENT_CODE - 1);
    return false;
  }

  // 3. Если для этого кода ещё нет массива обработчиков — создаём его
  if (!state.registered[code].events) {
    state.registered[code].events = darray_create(RegisteredEvent);
    if (!state.registered[code].events) {
      TE_LOG_FATAL("Не удалось выделить память для массива событий: file->event.cpp,func->event_register");
      return false;
    }
  }

  // 4. Проверяем, нет ли уже такой же регистрации (одинаковый listener и callback)
  u64 count = darray_length(state.registered[code].events);
  for (u64 i = 0; i < count; ++i) {
    RegisteredEvent* reg = &state.registered[code].events[i];
    if (reg->listener == listener && reg->callback == on_event) {
      TE_LOG_WARN("Попытка повторной регистрации одного и того же обработчика: file->event.cpp,func->event_register");
      return false; // Дубликат — не регистрируем повторно
    }
  }

  // 5. Создаём структуру новой регистрации
  RegisteredEvent new_event;
  new_event.listener = listener;
  new_event.callback = on_event;

  // 6. Добавляем её в массив через darray_push
  darray_push(state.registered[code].events, new_event);

  return true;
}
//-----------------------------------
b8 event_unregister(u16 code, void* listener, PFN_on_event on_event) {
  // 1. Проверка инициализации
  if (!is_initialized) {
    TE_LOG_ERROR("Попытка отмены регистрации при неинициализированной системе: file->event.cpp,func->event_unregister");
    return false;
  }

  // 2. Проверка кода
  if (code >= MAX_EVENT_CODE) {
    TE_LOG_ERROR("Код события %d выходит за пределы: file->event.cpp,func->event_unregister", code);
    return false;
  }

  // 3. Если для этого кода нет обработчиков, нечего отменять
  if (!state.registered[code].events) {
    TE_LOG_WARN("Попытка отмены регистрации для кода без обработчиков: file->event.cpp,func->event_unregister");
    return false;
  }

  // 4. Ищем точное совпадение listener + callback
  u64 count = darray_length(state.registered[code].events);
  for (u64 i = 0; i < count; ++i) {
    RegisteredEvent* reg = &state.registered[code].events[i];
    if (reg->listener == listener && reg->callback == on_event) {
      // Нашли — удаляем элемент по индексу
      RegisteredEvent popped;
      darray_pop_at(state.registered[code].events, i, &popped);
      TE_LOG_DEBUG("Обработчик события удалён: file->event.cpp,func->event_unregister");
      return true;
    }
  }

  // 5. Совпадение не найдено
  TE_LOG_WARN("Попытка отмены несуществующей регистрации: file->event.cpp,func->event_unregister");
  return false;
}
//-----------------------------------
b8 event_fire(u16 code, void* sender, EventContext context) {
  // 1. Проверка инициализации
  if (!is_initialized) {
    TE_LOG_ERROR("Попытка отправки события при неинициализированной системе: file->event.cpp,func->event_fire");
    return false;
  }

  // 2. Проверка кода
  if (code >= MAX_EVENT_CODE) {
    TE_LOG_ERROR("Код события %d выходит за пределы: file->event.cpp,func->event_fire", code);
    return false;
  }

  // 3. Если для этого кода нет обработчиков, событие не обработано
  if (!state.registered[code].events) {
    return false;
  }

  // 4. Получаем текущее количество обработчиков
  u64 count = darray_length(state.registered[code].events);

  // 5. Проходим по всем обработчикам
  for (u64 i = 0; i < count; ++i) {
    RegisteredEvent* reg = &state.registered[code].events[i];

    // Вызываем колбэк; если он возвращает true, прерываем цепочку
    if (reg->callback(code, sender, reg->listener, context)) {
      return true; // Событие обработано, дальнейшая рассылка не требуется
    }
  }

  // 6. Ни один обработчик не вернул true
  return false;
}