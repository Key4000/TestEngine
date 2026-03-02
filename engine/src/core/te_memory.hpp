/*
* это своя подсистема управления памятью
* , для отслеживания вообще всей работы 
* памяти 
*/
#pragma once 

#include <defines.hpp>

typedef enum MemoryTag{
    MEMORY_TAG_UNKNOWN,          // Для временного использования
    MEMORY_TAG_ARRAY,            // Обычные массивы
    MEMORY_TAG_DARRAY,           // Динамические массивы
    MEMORY_TAG_DICT,             // Словари/хэш-таблицы
    MEMORY_TAG_RING_QUEUE,       // Кольцевые буферы
    MEMORY_TAG_BST,              // Бинарные деревья поиска
    MEMORY_TAG_STRING,           // Строки
    MEMORY_TAG_APPLICATION,      // Данные приложения
    MEMORY_TAG_JOB,              // Задачи/работы (job system)
    MEMORY_TAG_TEXTURE,          // Текстуры
    MEMORY_TAG_MATERIAL_INSTANCE,// Материалы
    MEMORY_TAG_RENDERER,         // Рендерер
    MEMORY_TAG_GAME,             // Данные игры
    MEMORY_TAG_TRANSFORM,        // Трансформации объектов
    MEMORY_TAG_ENTITY,           // Сущности
    MEMORY_TAG_ENTITY_NODE,      // Узлы сущностей (иерархия)
    MEMORY_TAG_SCENE,            // Сцены

    MEMORY_TAG_MAX_TAGS          // Маркер конца (для массивов)
} MemoryTag;
/*
* функции инициализации/завершения системы памяти
*/
TE_API void memory_system_init();
TE_API void memory_system_shutdown();
/*
* основные функции выделения/освобождения с тегом
*/
TE_API void* te_memory_allocate(u64 size, MemoryTag tag);
TE_API void te_memory_free(void* block, u64 size, MemoryTag tag);
/*
* функции манипуляций с памтью 
*/
TE_API void* te_memory_zero(void* block, u64 size);
TE_API void* te_memory_copy(void* dest, const void* source, u64 size);
TE_API void* te_memory_set(void* dest, i32 number, u64 size);
/*
* статистика используемой памяти
* ВНИМАНИЕ: возвращает указатель на динамически выделенную строку.
* Вызывающий ОБЯЗАН освободить её с помощью te_memory_free().
*/
TE_API char* get_memory_usage_str();

