/**
 * @file darray.cpp
 * @brief Реализация динамического массива (C-стиль) с метаданными перед данными.
 *
 * Содержит функции для создания, уничтожения, изменения размера,
 * добавления, удаления и вставки элементов. Все операции с памятью
 * выполняются через подсистему te_memory.
 */

#include <core/darray.hpp>
#include <core/te_memory.hpp>
#include <core/logger.hpp>

//==============================================================================
// Внутренние утилиты
//==============================================================================

/**
 * @brief Вычисляет размер заголовка метаданных в байтах.
 */
#define HEADER_SIZE() (DARRAY_FIELD_LENGTH * sizeof(u64))

//==============================================================================
// Создание и уничтожение
//==============================================================================

void* _darray_create(u64 capacity, u64 stride) {
    /**
     * @brief Создаёт новый динамический массив.
     * @param capacity Начальная ёмкость.
     * @param stride   Размер элемента.
     * @return Указатель на область данных или NULL при ошибке.
     *
     * Алгоритм:
     * 1. header_size = DARRAY_FIELD_LENGTH * sizeof(u64)
     * 2. array_size = capacity * stride
     * 3. Выделить память: header_size + array_size, тег MEMORY_TAG_DARRAY.
     * 4. Обнулить всю память.
     * 5. Заполнить метаданные: capacity, length = 0, stride.
     * 6. Вернуть указатель на начало данных (после метаданных).
     */
    u64 header_size = HEADER_SIZE();
    u64 array_size = capacity * stride;
    u64* new_array = te_memory_allocate(header_size + array_size, MEMORY_TAG_DARRAY);
    if (!new_array) return nullptr;

    te_memory_zero(new_array, header_size + array_size);
    new_array[DARRAY_CAPACITY] = capacity;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = stride;
    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

void _darray_destroy(void* array) {
    /**
     * @brief Уничтожает массив и освобождает память.
     * @param array Указатель на данные массива.
     *
     * Алгоритм:
     * 1. Получить указатель на начало метаданных: (u64*)array - DARRAY_FIELD_LENGTH.
     * 2. Вычислить полный размер блока: header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE].
     * 3. Освободить память через te_memory_free.
     */
    if (!array) return;
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = HEADER_SIZE();
    u64 all_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    te_memory_free(header, all_size, MEMORY_TAG_DARRAY);
}

//==============================================================================
// Доступ к метаданным
//==============================================================================

u64 _darray_field_get(void* array, u64 field) {
    /**
     * @brief Возвращает значение поля метаданных.
     * @param array Указатель на данные.
     * @param field Индекс поля.
     * @return Значение поля.
     */
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darray_field_set(void* array, u64 field, u64 value) {
    /**
     * @brief Устанавливает значение поля метаданных.
     * @param array Указатель на данные.
     * @param field Индекс поля.
     * @param value Новое значение.
     */
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

//==============================================================================
// Изменение размера
//==============================================================================

void* _darray_resize(void* array) {
    /**
     * @brief Увеличивает ёмкость массива в DARRAY_RESIZE_FACTOR раз.
     * @param array Указатель на данные.
     * @return Новый указатель на данные (или старый при ошибке).
     *
     * Алгоритм:
     * 1. Получить текущие длину, размер элемента и ёмкость.
     * 2. Создать новый массив с ёмкостью DARRAY_RESIZE_FACTOR * старая_ёмкость.
     * 3. Скопировать все существующие элементы.
     * 4. Установить длину нового массива равной старой длине.
     * 5. Уничтожить старый массив.
     * 6. Вернуть указатель на данные нового массива.
     */
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    u64 old_capacity = darray_capacity(array);

    void* temp = _darray_create(DARRAY_RESIZE_FACTOR * old_capacity, stride);
    if (!temp) {
        TE_LOG_FATAL("Не удалось увеличить массив: не хватило памяти");
        return array;
    }

    te_memory_copy(temp, array, length * stride);
    _darray_field_set(temp, DARRAY_LENGTH, length);
    _darray_destroy(array);
    return temp;
}

//==============================================================================
// Добавление и удаление элементов
//==============================================================================

void* _darray_push(void* array, const void* value_ptr) {
    /**
     * @brief Добавляет элемент в конец массива.
     * @param array     Указатель на данные.
     * @param value_ptr Указатель на значение для копирования.
     * @return Новый указатель на данные (может измениться при реаллокации).
     *
     * Алгоритм:
     * 1. Получить текущую длину и размер элемента.
     * 2. Если длина + 1 == ёмкость, вызвать _darray_resize.
     * 3. Вычислить адрес нового элемента: array + длина * stride.
     * 4. Скопировать данные.
     * 5. Увеличить длину на 1.
     */
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    if (length + 1 == darray_capacity(array)) {
        array = _darray_resize(array);
        length = darray_length(array); // обновить длину после ресайза (на всякий случай)
    }

    u64 addr = (u64)array + (length * stride);
    te_memory_copy((void*)addr, value_ptr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _darray_pop(void* array, void* dest) {
    /**
     * @brief Удаляет последний элемент и, если нужно, сохраняет его.
     * @param array Указатель на данные.
     * @param dest  Указатель для сохранения удалённого элемента (может быть NULL).
     *
     * Алгоритм:
     * 1. Получить длину и размер элемента.
     * 2. Вычислить адрес последнего элемента.
     * 3. Если dest != NULL, скопировать данные.
     * 4. Уменьшить длину на 1.
     */
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (length == 0) return;

    u64 addr = (u64)array + ((length - 1) * stride);
    if (dest) {
        te_memory_copy(dest, (void*)addr, stride);
    }
    _darray_field_set(array, DARRAY_LENGTH, length - 1);
}

void* _darray_pop_at(void* array, u64 index, void* dest) {
    /**
     * @brief Удаляет элемент по индексу и, если нужно, сохраняет его.
     * @param array Указатель на данные.
     * @param index Индекс удаляемого элемента.
     * @param dest  Указатель для сохранения значения (может быть NULL).
     * @return Указатель на данные (обычно не меняется).
     *
     * Алгоритм:
     * 1. Проверить, что index < длины.
     * 2. Если dest != NULL, скопировать удаляемый элемент.
     * 3. Если index не последний, сдвинуть элементы после index влево.
     * 4. Уменьшить длину.
     */
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (index >= length) {
        TE_LOG_ERROR("Индекс выходит за пределы массива: %i, index: %i", length, index);
        return array;
    }

    u64 addr = (u64)array;
    if (dest) {
        te_memory_copy(dest, (void*)(addr + index * stride), stride);
    }

    if (index != length - 1) {
        te_memory_move(
            (void*)(addr + index * stride),
            (void*)(addr + (index + 1) * stride),
            stride * (length - index - 1));
    }

    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    return array;
}

void* _darray_insert_at(void* array, u64 index, void* value_ptr) {
    /**
     * @brief Вставляет элемент по указанному индексу.
     * @param array     Указатель на данные.
     * @param index     Индекс для вставки.
     * @param value_ptr Указатель на значение.
     * @return Новый указатель на данные (может измениться при реаллокации).
     *
     * Алгоритм:
     * 1. Проверить, что index < длины.
     * 2. Если длина + 1 == ёмкость, вызвать _darray_resize.
     * 3. Если index не последний, сдвинуть элементы вправо.
     * 4. Скопировать новое значение на освободившееся место.
     * 5. Увеличить длину.
     */
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (index > length) {
        TE_LOG_ERROR("Индекс выходит за пределы массива: %i, index: %i", length, index);
        return array;
    }

    if (length + 1 == darray_capacity(array)) {
        array = _darray_resize(array);
        length = darray_length(array); // обновить после ресайза
    }

    u64 addr = (u64)array;
    if (index != length) {
        te_memory_move(
            (void*)(addr + (index + 1) * stride),
            (void*)(addr + index * stride),
            stride * (length - index));
    }

    te_memory_copy((void*)(addr + index * stride), value_ptr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}