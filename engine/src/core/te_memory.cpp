/**
 * @file te_memory.cpp
 * @brief Реализация подсистемы управления памятью.
 *
 * Содержит внутреннюю структуру для хранения статистики,
 * а также реализацию функций выделения, освобождения и работы с памятью.
 * Статистика доступна при включённом макросе TE_MEMORY_STATS_ENABLED.
 */

#include <core/te_memory.hpp>
#include <core/logger.hpp>
#include <platform/platform.hpp>
//TODO: свои строки 
#include <string.h>
#include <stdio.h>

namespace te_memory {

    /**
     * @struct MemoryStats
     * @brief Внутренняя структура для хранения статистики использования памяти.
     */
    struct MemoryStats {
        u64 total_allocated;                          ///< Общее количество выделенной памяти (в байтах).
        u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];  ///< Массив, где каждый элемент — объём памяти, выделенный под соответствующий тег.
    } stats;

    /**
     * @var memory_tag_strings
     * @brief Массив строковых представлений тегов для вывода в отчёте.
     *
     * Индекс в массиве соответствует значению перечисления MemoryTag.
     * Используется функцией get_memory_usage_str().
     */
    const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
        "UNKNOWN    ",  // MEMORY_TAG_UNKNOWN
        "ARRAY      ",  // MEMORY_TAG_ARRAY
        "DARRAY     ",  // MEMORY_TAG_DARRAY
        "DICT       ",  // MEMORY_TAG_DICT
        "RING_QUEUE ",  // MEMORY_TAG_RING_QUEUE
        "BST        ",  // MEMORY_TAG_BST
        "STRING     ",  // MEMORY_TAG_STRING
        "APPLICATION",  // MEMORY_TAG_APPLICATION
        "JOB        ",  // MEMORY_TAG_JOB
        "TEXTURE    ",  // MEMORY_TAG_TEXTURE
        "MAT_INST   ",  // MEMORY_TAG_MATERIAL_INSTANCE
        "RENDERER   ",  // MEMORY_TAG_RENDERER
        "GAME       ",  // MEMORY_TAG_GAME
        "TRANSFORM  ",  // MEMORY_TAG_TRANSFORM
        "ENTITY     ",  // MEMORY_TAG_ENTITY
        "ENTITY_NODE",  // MEMORY_TAG_ENTITY_NODE
        "SCENE      "   // MEMORY_TAG_SCENE
    };

} // namespace te_memory

using namespace te_memory;

//==============================================================================
// Инициализация и завершение
//==============================================================================

void memory_system_init() {
    /**
     * @brief Инициализирует подсистему памяти.
     *
     * Обнуляет всю внутреннюю статистику.
     */
    platform_zero_memory(&stats, sizeof(stats));
}

void memory_system_shutdown() {
    /**
     * @brief Завершает работу подсистемы памяти.
     *
     * Проверяет, не осталось ли неосвобождённой памяти,
     * и в случае обнаружения утечек выводит фатальную ошибку.
     */
    if (stats.total_allocated != 0) {
        TE_LOG_FATAL("Не вся память освобождена, какие-то теги не обнулились!: file->te_memory.cpp, func->memory_system_shutdown");
    }
}

//==============================================================================
// Выделение и освобождение памяти
//==============================================================================

void* te_memory_allocate(u64 size, MemoryTag tag) {
    /**
     * @brief Выделяет блок памяти заданного размера с указанным тегом.
     * @param size Размер блока в байтах.
     * @param tag  Категория памяти.
     * @return Указатель на выделенный блок или nullptr при ошибке.
     *
     * @note Если тег MEMORY_TAG_UNKNOWN, выдаётся предупреждение.
     * @note Выделенная память обнуляется.
     */
    if (tag == MEMORY_TAG_UNKNOWN) {
        TE_LOG_WARN("Выделяется память с MEMORY_TAG_UNKNOWN, поменяйте тег: file->te_memory.cpp, func->te_memory_allocate");
    }
    // TODO: Добавить поддержку выравнивания (alignment)
    void* block = platform_allocate_memory(size, false);
    if (!block) {
        TE_LOG_FATAL("Не удалось выделить память: file->te_memory.cpp, func->te_memory_allocate");
        return nullptr;
    }
#ifdef TE_MEMORY_STATS_ENABLED
    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;
#endif
    platform_zero_memory(block, size);
    return block;
}

void te_memory_free(void* block, u64 size, MemoryTag tag) {
    /**
     * @brief Освобождает ранее выделенный блок памяти.
     * @param block Указатель на блок (может быть nullptr).
     * @param size  Размер блока в байтах (должен совпадать с указанным при выделении).
     * @param tag   Категория памяти (должна совпадать с указанной при выделении).
     *
     * @note Если тег MEMORY_TAG_UNKNOWN, выдаётся предупреждение.
     * @note Если block == nullptr, функция ничего не делает.
     */
    if (tag == MEMORY_TAG_UNKNOWN) {
        TE_LOG_WARN("Освобождается память с MEMORY_TAG_UNKNOWN, поменяйте тег: file->te_memory.cpp, func->te_memory_free");
    }
    if (block) {
#ifdef TE_MEMORY_STATS_ENABLED
        stats.total_allocated -= size;
        stats.tagged_allocations[tag] -= size;
#endif
        // TODO: Добавить поддержку выравнивания
        platform_free_memory(block, false);
    }
}

//==============================================================================
// Функции манипуляции памятью
//==============================================================================

void* te_memory_zero(void* block, u64 size) {
    /**
     * @brief Заполняет блок памяти нулями.
     * @param block Указатель на блок.
     * @param size  Размер блока в байтах.
     * @return Указатель на блок.
     */
    return platform_zero_memory(block, size);
}

void* te_memory_copy(void* dest, const void* source, u64 size) {
    /**
     * @brief Копирует данные из одного блока в другой (без перекрытия).
     * @param dest   Указатель на целевой блок.
     * @param source Указатель на исходный блок.
     * @param size   Количество байт для копирования.
     * @return Указатель на dest.
     */
    return platform_copy_memory(dest, source, size);
}

void* te_memory_set(void* dest, i32 value, u64 size) {
    /**
     * @brief Заполняет блок памяти заданным значением.
     * @param dest  Указатель на блок.
     * @param value Значение (интерпретируется как unsigned char).
     * @param size  Размер блока в байтах.
     * @return Указатель на dest.
     */
    return platform_set_memory(dest, value, size);
}

void* te_memory_move(void* dest, const void* source, u64 size) {
    /**
     * @brief Копирует данные с учётом возможного перекрытия областей.
     * @param dest   Указатель на целевой блок.
     * @param source Указатель на исходный блок.
     * @param size   Количество байт для копирования.
     * @return Указатель на dest.
     */
    return platform_move_memory(dest, source, size);
}

//==============================================================================
// Статистика использования памяти
//==============================================================================

#ifdef TE_MEMORY_STATS_ENABLED

char* get_memory_usage_str() {
    /**
     * @brief Формирует строку с отчётом об использовании памяти по тегам.
     * @return Указатель на динамически выделенную строку.
     *
     * @note Вызывающий обязан освободить возвращённую строку с помощью te_memory_free().
     *
     * Формат отчёта:
     *   System memory use (tagged):
     *     UNKNOWN    : 0.00B
     *     ARRAY      : 1.23KiB
     *     ...
     */
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;
    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.tagged_allocations[i] / (float)gib;
        } else if (stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.tagged_allocations[i] / (float)mib;
        } else if (stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.tagged_allocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0; // убираем "iB"
            amount = (float)stats.tagged_allocations[i];
        }
        u64 remaining = sizeof(buffer) - offset;
        i32 length = snprintf(buffer + offset, remaining, "  %s: %.2f%s\n",
                              memory_tag_strings[i], amount, unit);
        offset += length;
    }

    u64 len = strlen(buffer) + 1;
    char* out_string = static_cast<char*>(te_memory_allocate(len, MEMORY_TAG_STRING));
    te_memory_copy(out_string, buffer, len);
    return out_string;
}

#endif // TE_MEMORY_STATS_ENABLED