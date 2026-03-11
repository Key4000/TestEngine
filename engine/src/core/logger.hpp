/**
 * @file logger.hpp
 * @brief Система логирования с уровнями, фильтрацией и цветным выводом.
 *
 * Предоставляет уровни логирования (TRACE, DEBUG, INFO, WARN, ERROR, FATAL),
 * возможность фильтрации на этапе компиляции и во время выполнения,
 * а также макросы для удобного использования.
 */

#pragma once

#include <defines.hpp>  // для TE_API и базовых типов

//==============================================================================
// Уровни логирования
//==============================================================================

/**
 * @enum LogLevel
 * @brief Уровни логирования от наиболее критичного до наименее важного.
 */
enum class LogLevel {
    LL_FATAL, ///< Фатальные ошибки, после которых работа невозможна.
    LL_ERROR, ///< Ошибки, не приводящие к немедленному краху.
    LL_WARN,  ///< Предупреждения о потенциальных проблемах.
    LL_INFO,  ///< Информационные сообщения (например, о запуске).
    LL_DEBUG, ///< Отладочная информация (только в debug-сборке).
    LL_TRACE  ///< Подробнейшая трассировка (только в debug-сборке).
};

//==============================================================================
// Глобальные переменные и функции фильтрации
//==============================================================================

/**
 * @var glob_min_log_level
 * @brief Глобальный минимальный уровень логирования.
 *
 * Сообщения с уровнем ниже этого значения не выводятся.
 * Может быть изменён во время выполнения через te_set_log_level().
 */
extern LogLevel glob_min_log_level;

/**
 * @brief Устанавливает минимальный уровень логирования во время выполнения.
 * @param level Новый минимальный уровень.
 */
TE_API void te_set_log_level(LogLevel level);

//==============================================================================
// Основные функции логирования
//==============================================================================

/**
 * @brief Записывает сообщение в лог с указанным уровнем.
 * @param level   Уровень сообщения.
 * @param message Форматная строка (как в printf).
 * @param ...     Аргументы для форматной строки.
 */
TE_API void te_log_write(LogLevel level, const char* message, ...);

/**
 * @brief Инициализирует систему логирования (устанавливает UTF-8 в консоль).
 * @return true в случае успеха, false при ошибке.
 */
TE_API b8 log_init();

/**
 * @brief Завершает работу системы логирования (в текущей версии ничего не делает).
 */
void log_shutdown();

//==============================================================================
// Конфигурация включения уровней (compile-time фильтрация)
//==============================================================================

/**
 * @def LOG_TRACE_ENABLED
 * @brief Включает/отключает вывод TRACE-сообщений на этапе компиляции.
 * @details Если макрос определён как 1, TE_LOG_TRACE будет активен.
 *          В релизной сборке (NDEBUG) принудительно устанавливается в 0.
 */
#ifndef LOG_TRACE_ENABLED
#define LOG_TRACE_ENABLED 1
#endif

/**
 * @def LOG_DEBUG_ENABLED
 * @brief Включает/отключает вывод DEBUG-сообщений.
 */
#ifndef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 1
#endif

/**
 * @def LOG_INFO_ENABLED
 * @brief Включает/отключает вывод INFO-сообщений.
 */
#ifndef LOG_INFO_ENABLED
#define LOG_INFO_ENABLED 1
#endif

/**
 * @def LOG_WARN_ENABLED
 * @brief Включает/отключает вывод WARN-сообщений.
 */
#ifndef LOG_WARN_ENABLED
#define LOG_WARN_ENABLED 1
#endif

/**
 * @def LOG_ERROR_ENABLED
 * @brief Включает/отключает вывод ERROR-сообщений.
 */
#ifndef LOG_ERROR_ENABLED
#define LOG_ERROR_ENABLED 1
#endif

/**
 * @def LOG_FATAL_ENABLED
 * @brief Включает/отключает вывод FATAL-сообщений.
 */
#ifndef LOG_FATAL_ENABLED
#define LOG_FATAL_ENABLED 1
#endif

// В релизной сборке принудительно отключаем TRACE и DEBUG.
#ifdef NDEBUG
#undef LOG_TRACE_ENABLED
#define LOG_TRACE_ENABLED 0
#undef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 0
#endif

//==============================================================================
// Макросы логирования
//==============================================================================

/**
 * @def TE_LOG_TRACE(format, ...)
 * @brief Выводит сообщение уровня TRACE, если он включён.
 */
#if LOG_TRACE_ENABLED
#define TE_LOG_TRACE(format, ...) te_log_write(LogLevel::LL_TRACE, format, ##__VA_ARGS__)
#else
#define TE_LOG_TRACE(format, ...) ((void)0)
#endif

/**
 * @def TE_LOG_DEBUG(format, ...)
 * @brief Выводит сообщение уровня DEBUG, если он включён.
 */
#if LOG_DEBUG_ENABLED
#define TE_LOG_DEBUG(format, ...) te_log_write(LogLevel::LL_DEBUG, format, ##__VA_ARGS__)
#else
#define TE_LOG_DEBUG(format, ...) ((void)0)
#endif

/**
 * @def TE_LOG_INFO(format, ...)
 * @brief Выводит сообщение уровня INFO, если он включён.
 */
#if LOG_INFO_ENABLED
#define TE_LOG_INFO(format, ...) te_log_write(LogLevel::LL_INFO, format, ##__VA_ARGS__)
#else
#define TE_LOG_INFO(format, ...) ((void)0)
#endif

/**
 * @def TE_LOG_WARN(format, ...)
 * @brief Выводит сообщение уровня WARN, если он включён.
 */
#if LOG_WARN_ENABLED
#define TE_LOG_WARN(format, ...) te_log_write(LogLevel::LL_WARN, format, ##__VA_ARGS__)
#else
#define TE_LOG_WARN(format, ...) ((void)0)
#endif

/**
 * @def TE_LOG_ERROR(format, ...)
 * @brief Выводит сообщение уровня ERROR, если он включён.
 */
#if LOG_ERROR_ENABLED
#define TE_LOG_ERROR(format, ...) te_log_write(LogLevel::LL_ERROR, format, ##__VA_ARGS__)
#else
#define TE_LOG_ERROR(format, ...) ((void)0)
#endif

/**
 * @def TE_LOG_FATAL(format, ...)
 * @brief Выводит сообщение уровня FATAL, если он включён.
 */
#if LOG_FATAL_ENABLED
#define TE_LOG_FATAL(format, ...) te_log_write(LogLevel::LL_FATAL, format, ##__VA_ARGS__)
#else
#define TE_LOG_FATAL(format, ...) ((void)0)
#endif