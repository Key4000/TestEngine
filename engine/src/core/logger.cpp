/*
 *  Реализация системы логирования
 *  1. run-time фильтрация
 *  2. compile-time фильтрация
 *  3. вспомогательная функция для приведения уровня в строку
 *  4. основная функция логирования
 */
#include <core/logger.hpp>
#include <core/asserts.hpp>
#include <platform/platform.hpp>
#include <cstdio>     // fprintf, vfprintf, stderr
#include <cstdarg>    // va_list, va_start, va_end
#include <windows.h>  //UTF-8 in console

/*
 *  run-time фильтрация
 */
LogLevel glob_min_log_level = LogLevel::LL_FATAL;
void te_set_log_level(LogLevel level) {
    glob_min_log_level = level;
}

/*
 * вспомогательная функция для приведения уровня в строку
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

/*
 *  основная функция логирования
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

/*
 * функция инициализации и завершения логировария
 */
b8 log_init() {
    SetConsoleOutputCP(CP_UTF8);
    return true;
}

void log_shutdown() {
    // TODO: дописать позже
}

/*
 * функция обработки условий(asserts)(обьявление в asserts.hpp)
 */
void te_assert_fail(const char* expr, const char* msg, const char* name, i32 line) {
    // Сообщение об ошибке будет выведено в лог с уровнем LOG_LEVEL_FATAL, что
    // означает фатальную ошибку.
    te_log_write(LogLevel::LL_FATAL,
                 "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n",
                 expr, msg, name, line);
}