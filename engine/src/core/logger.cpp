/**
 * @file logger.cpp
 * @brief Реализация системы логирования.
 *
 * Содержит функции для фильтрации, форматирования и вывода сообщений в консоль
 * с поддержкой цветов и уровней логирования.
 */

#include <core/logger.hpp>
#include <core/asserts.hpp>
#include <platform/platform.hpp>
#include <cstdio>     // fprintf, vfprintf, stderr
#include <cstdarg>    // va_list, va_start, va_end
#include <windows.h>  // UTF-8 in console

//==============================================================================
// Глобальная переменная для run-time фильтрации
//==============================================================================

/**
 * @var glob_min_log_level
 * @brief Глобальный минимальный уровень логирования.
 *
 * Сообщения с уровнем ниже этого значения игнорируются.
 */
LogLevel glob_min_log_level = LogLevel::LL_FATAL;

/**
 * @brief Устанавливает минимальный уровень логирования во время выполнения.
 * @param level Новый минимальный уровень.
 */
void te_set_log_level(LogLevel level) {
    glob_min_log_level = level;
}

//==============================================================================
// Вспомогательная функция: преобразование уровня в строку
//==============================================================================

/**
 * @brief Преобразует уровень логирования в строку с квадратными скобками.
 * @param level Уровень логирования.
 * @return Указатель на статическую строку (например, "[FATAL]").
 */
static const char* level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::LL_TRACE:
            return "[TRACE]";
        case LogLevel::LL_DEBUG:
            return "[DEBUG]";
        case LogLevel::LL_INFO:
            return "[INFO]";
        case LogLevel::LL_WARN:
            return "[WARN]";
        case LogLevel::LL_ERROR:
            return "[ERROR]";
        case LogLevel::LL_FATAL:
            return "[FATAL]";
        default:
            return "[UNKNOWN]";
    }
}

//==============================================================================
// Основная функция логирования
//==============================================================================

/**
 * @brief Записывает форматированное сообщение в лог.
 * @param level   Уровень сообщения.
 * @param message Форматная строка (printf-стиль).
 * @param ...     Аргументы для форматной строки.
 *
 * Алгоритм:
 * 1. Проверка run-time фильтрации (если уровень ниже минимального — выход).
 * 2. Форматирование сообщения в буфер.
 * 3. Добавление префикса с уровнем.
 * 4. Определение цвета в зависимости от уровня.
 * 5. Вывод в консоль (ошибки — в stderr, остальное — в stdout).
 */
void te_log_write(LogLevel level, const char* message, ...) {
    b8 is_error = level < LogLevel::LL_WARN;
    if (level < glob_min_log_level) {
        return;  // run-time фильтрация
    }

    const i32 msg_length = 32000;
    char out_message[msg_length];

    memset(out_message, 0, sizeof(out_message));
    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, msg_length, message, arg_ptr);
    va_end(arg_ptr);

    char out_message2[msg_length];
    const char* cur_level = level_to_string(level);
    sprintf(out_message2, "%s%s\n", cur_level, out_message);

    u8 color_index;
    switch (level) {
        case LogLevel::LL_FATAL:
            color_index = 0;
            break;
        case LogLevel::LL_ERROR:
            color_index = 1;
            break;
        case LogLevel::LL_WARN:
            color_index = 2;
            break;
        case LogLevel::LL_INFO:
            color_index = 3;
            break;
        case LogLevel::LL_DEBUG:
            color_index = 4;
            break;
        case LogLevel::LL_TRACE:
            color_index = 5;
            break;
        default:
            color_index = 2;
    }

    if (is_error) {
        platform_console_write_error(out_message2, color_index);
    } else {
        platform_console_write(out_message2, color_index);
    }
}

//==============================================================================
// Инициализация и завершение работы системы логирования
//==============================================================================

/**
 * @brief Инициализирует систему логирования.
 * @return true, если успешно, иначе false.
 *
 * В текущей реализации устанавливает кодировку консоли UTF-8.
 */
b8 log_init() {
    SetConsoleOutputCP(CP_UTF8);
    return true;
}

/**
 * @brief Завершает работу системы логирования.
 * @note В текущей версии ничего не делает (заглушка).
 */
void log_shutdown() {
    // TODO: дописать позже (например, закрытие файла лога)
}

//==============================================================================
// Функция обработки утверждений (вызывается из макросов asserts.hpp)
//==============================================================================

/**
 * @brief Вызывается при срабатывании утверждения.
 * @param expr Выражение, которое не выполнилось.
 * @param msg  Дополнительное сообщение.
 * @param name Имя файла.
 * @param line Номер строки.
 *
 * Логирует фатальную ошибку с подробностями об утверждении.
 */
void te_assert_fail(const char* expr, const char* msg, const char* name, i32 line) {
    te_log_write(LogLevel::LL_FATAL,
                 "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n",
                 expr, msg, name, line);
}