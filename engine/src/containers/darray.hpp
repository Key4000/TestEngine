/*
* динамический массив
*/
#pragma once

#include <defines.hpp>

enum {
  DARRAY_CAPACITY,
  //сколько элементов вмещает массив(смещение 0)
  DARRAY_LENGTH,
  //сколько сейчас занято(смещение 8)
  DARRAY_STRIDE,
  //сколько байт весит один элемент(смещение 16)
  DARRAY_FIELD_LENGTH //(используется для проверок)
};

TE_API void* _darray_create(u64 capacity, u64 stride);
TE_API void _darray_destroy(void* array);
TE_API u64 _darray_field_get(void* array, u64 field);
TE_API void _darray_field_set(void* array, u64 field, u64 value);
TE_API void* _darray_resize(void* array);

TE_API void* _darray_push(void* array, const void* value_ptr);
TE_API void _darray_pop(void* array, void* dest);
TE_API void* _darray_pop_at(void* array, u64 index, void* dest);
TE_API void* _darray_insert_at(void* array, u64 index, void* value_ptr);
//------------------------------------
// Макросы для удобного использования с типами:
//------------------------------------
/*
 * Константы конфигурации:
 */
#define DARRAY_DEFAULT_CAPACITY 1   // Начальная ёмкость по умолчанию
#define DARRAY_RESIZE_FACTOR 2      // Коэффициент увеличения при перераспределении
/*
* Создаёт динамический массив для указанного типа
*/
#define darray_create(type) \
_darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))
/*
* Создаёт динамический массив с указанной начальной ёмкостью
*/
#define darray_reserve(type, capacity) \
_darray_create(capacity, sizeof(type))
/*
* Уничтожает массив
*/
#define darray_destroy(array) _darray_destroy(array);
/*
* Добавляет элемент в конец массива (с автоматическим определением типа)
*/
#define darray_push(array, value)           \
{                                       \
auto temp = value;         \
array = _darray_push(array, &temp); \
}
/*
* Удаляет элемент с конца массива
*/
#define darray_pop(array, value_ptr) \
_darray_pop(array, value_ptr)
/*
* Вставляет элемент по указанному индексу
*/
#define darray_insert_at(array, index, value)           \
{                                                   \
auto temp = value;                     \
array = _darray_insert_at(array, index, &temp); \
}
/*
* Удаляет элемент по указанному индексу
*/
#define darray_pop_at(array, index, value_ptr) \
_darray_pop_at(array, index, value_ptr)
/*
* Очищает массив (устанавливает length = 0, но не освобождает память)
*/
#define darray_clear(array) \
_darray_field_set(array, DARRAY_LENGTH, 0)
/*
* Получает текущую ёмкость массива
*/
#define darray_capacity(array) \
_darray_field_get(array, DARRAY_CAPACITY)
/*
* Получает текущее количество элементов
*/
#define darray_length(array) \
_darray_field_get(array, DARRAY_LENGTH)
/*
* Получает размер элемента в байтах
*/
#define darray_stride(array) \
_darray_field_get(array, DARRAY_STRIDE)
/*
* Устанавливает количество элементов (осторожно!)
*/
#define darray_length_set(array, value) \
_darray_field_set(array, DARRAY_LENGTH, value)