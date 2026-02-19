/*
 *  Система логирования
 *  Уровни: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
 *  В релизной сборке (NDEBUG) TRACE и DEBUG принудительно отключаются
*  Возможность фильтрации уровней логирования в ren-time и compile-time
 */

#pragma once

#include <defines.hpp>  // для TE_API, типов

// Уровни логирования
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

//run-time фильтрация
extern LogLevel glob_min_log_level;
TE_API void te_set_log_level(LogLevel level);


/*
*  основная функция логирования и 
*  дополнительные функции инициализации
*/

TE_API void te_log_write(LogLevel level, const char* format, ...);

b8 initialize_logging();
void shutdown_logging();

// -------------------- Конфигурация включения уровней --------------------
/*
* compile-time фильтрация - 
* Каждый уровень можно включить
* /отключить через LOG_*_ENABLED
*
* Пользователь может определить эти  
* макросы до включения заголовка,
* чтобы переопределить значения по 
* умолчанию.
*/


#ifndef LOG_TRACE_ENABLED
#define LOG_TRACE_ENABLED 1
#endif

#ifndef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 1
#endif

#ifndef LOG_INFO_ENABLED
#define LOG_INFO_ENABLED 1
#endif

#ifndef LOG_WARN_ENABLED
#define LOG_WARN_ENABLED 1
#endif

#ifndef LOG_ERROR_ENABLED
#define LOG_ERROR_ENABLED 1
#endif

#ifndef LOG_FATAL_ENABLED
#define LOG_FATAL_ENABLED 1
#endif

// В релизной сборке (NDEBUG определён) принудительно отключаем TRACE и DEBUG.
#ifdef NDEBUG
#undef LOG_TRACE_ENABLED
#define LOG_TRACE_ENABLED 0
#undef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 0
#endif

// -------------------- Макросы логирования --------------------
// Каждый макрос активен только если соответствующий LEVEL_ENABLED == 1

#if LOG_TRACE_ENABLED
#define TE_LOG_TRACE(format, ...) te_log_write(LogLevel::TRACE, format, ##__VA_ARGS__)
#else
#define TE_LOG_TRACE(format, ...) ((void)0)
#endif

#if LOG_DEBUG_ENABLED
#define TE_LOG_DEBUG(format, ...) te_log_write(LogLevel::DEBUG, format, ##__VA_ARGS__)
#else
#define TE_LOG_DEBUG(format, ...) ((void)0)
#endif

#if LOG_INFO_ENABLED
#define TE_LOG_INFO(format, ...) te_log_write(LogLevel::INFO, format, ##__VA_ARGS__)
#else
#define TE_LOG_INFO(format, ...) ((void)0)
#endif

#if LOG_WARN_ENABLED
#define TE_LOG_WARN(format, ...) te_log_write(LogLevel::WARN, format, ##__VA_ARGS__)
#else
#define TE_LOG_WARN(format, ...) ((void)0)
#endif

#if LOG_ERROR_ENABLED
#define TE_LOG_ERROR(format, ...) te_log_write(LogLevel::ERROR, format, ##__VA_ARGS__)
#else
#define TE_LOG_ERROR(format, ...) ((void)0)
#endif

#if LOG_FATAL_ENABLED
#define TE_LOG_FATAL(format, ...) te_log_write(LogLevel::FATAL, format, ##__VA_ARGS__)
#else
#define TE_LOG_FATAL(format, ...) ((void)0)
#endif