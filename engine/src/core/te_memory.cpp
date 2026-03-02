#include <core/te_memory.hpp>
#include <core/logger.hpp>
#include <platform/platform.hpp>
// TODO: свои строки
#include <string.h>
#include <stdio.h>

namespace te_memory {
    struct MemoryStats {
        u64 total_allocated;                          // вся занятая память
        u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];  // занятая память разбита по тегам
    } stats;
    /*
     * Используется для красивого вывода в get_memory_usage_str().
     */
    const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] =
        {
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
            "MAT_INST   ",  // MEMORY_TAG_MATERIAL_INSTANCE (сокращённо)
            "RENDERER   ",  // MEMORY_TAG_RENDERER
            "GAME       ",  // MEMORY_TAG_GAME
            "TRANSFORM  ",  // MEMORY_TAG_TRANSFORM
            "ENTITY     ",  // MEMORY_TAG_ENTITY
            "ENTITY_NODE",  // MEMORY_TAG_ENTITY_NODE
            "SCENE      "   // MEMORY_TAG_SCENE
    };
}  // namespace te_memory
using namespace te_memory;
//-------------------------------------
void memory_system_init() {
    /*
     *  обнуляем массивы
     */
    platform_zero_memory(&stats, sizeof(stats));
}
//------------------------------------
void memory_system_shutdown() {
    if (stats.total_allocated != 0) {
        TE_LOG_FATAL("Не вся память освобождена , какие то теги не обнулились!: file->te_memory.cpp, func->memory_system_shutdown");
    }
}
//------------------------------------
void* te_memory_allocate(u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        TE_LOG_WARN("Выделяется память с MEMORY_TAG_UNKNOWN, поменяйте тег: file->te_memory.cpp, func->te_memory_allocate");
    }
    // TODO: Добавить поддержку выравнивания (alignment)
    void* block = platform_allocate_memory(size, false);
    if (!block) {
        TE_LOG_FATAL("Не удалось выделить память: file->te_memory.cpp, func->te_memory_allocate");
        return nullptr;
    }
    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;
    platform_zero_memory(block, size);
    return block;
}
//------------------------------------
void te_memory_free(void* block, u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        TE_LOG_WARN("Освобождается память с MEMORY_TAG_UNKNOWN, поменяйте тег: file->te_memory.cpp, func->te_system_free");
    }
    /*
     * обновляем статистику и освобождаем память и обнуляем её
     */
    if (block) {
        stats.total_allocated -= size;
        stats.tagged_allocations[tag] -= size;
        // TODO: Добавить поддержку выравнивания
        platform_free_memory(block, false);
    }
}
//------------------------------------
void* te_memory_zero(void* block, u64 size) {
    return platform_zero_memory(block, size);
}
//------------------------------------
void* te_memory_copy(void* dest, const void* source, u64 size) {
    return platform_copy_memory(dest, source, size);
}
//------------------------------------
void* te_memory_set(void* dest, i32 value, u64 size) {
    return platform_set_memory(dest, value, size);
}
//------------------------------------
char* get_memory_usage_str() {
    /*
     * Константы для преобразования единиц
     */
    const u64 gib = 1024 * 1024 * 1024;  // 1 гигабайт
    const u64 mib = 1024 * 1024;         // 1 мегабайт
    const u64 kib = 1024;                // 1 килобайт
    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);  // Текущая позиция в буфере

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";  // Шаблон для единиц: XiB, где X = B/K/M/G
        float amount = 1.0f;
        if (stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';  // Гигабайты
            amount = stats.tagged_allocations[i] / (float)gib;
        } else if (stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';  // Мегабайты
            amount = stats.tagged_allocations[i] / (float)mib;
        } else if (stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';  // Килобайты
            amount = stats.tagged_allocations[i] / (float)kib;
        } else {
            unit[0] = 'B';  // Байты
            unit[1] = 0;    // Обрезаем "iB", оставляем только "B"
            amount = (float)stats.tagged_allocations[i];
        }
        u64 remaining = sizeof(buffer) - offset;
        // Форматируем строку для текущего тега
        i32 length = snprintf(buffer + offset, remaining, "  %s: %.2f%s\n",
                              memory_tag_strings[i], amount, unit);
        offset += length;  // Сдвигаем позицию для следующей записи
    }

    u64 len = strlen(buffer) + 1;
    char* out_string = static_cast<char*>(te_memory_allocate(len, MEMORY_TAG_STRING));
    te_memory_copy(out_string, buffer, len);
    return out_string;
}
