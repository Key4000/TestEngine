/**
 * @file return_codes.hpp
 * @brief Коды возврата, используемые в движке.
 * 
 * Содержит только коды, которые реально применяются в текущей версии.
 * При добавлении новых ошибок раскомментируйте соответствующие блоки
 * или допишите новые значения в конец списка.
 */

#pragma once

/**
 * @enum ReturnCodes
 * @brief Коды возврата функций движка.
 */
enum class ReturnCodes {
    //==========================================================================
    // Успешное завершение
    //==========================================================================
    SUCCESS = 0, ///< Операция выполнена успешно.

    //==========================================================================
    // Ошибки инициализации
    //==========================================================================
    ERROR_LOG_INIT = 1,           ///< Ошибка инициализации системы логирования.
    ERROR_GAME_CREATE = 2,         ///< Ошибка создания экземпляра игры (create_game вернула false).
    ERROR_MISSING_GAME_FUNC = 3,   ///< Отсутствуют обязательные функции игры.
    ERROR_ENGINE_APP_RUN = 4,      ///< Ошибка выполнения главного цикла приложения.

    //==========================================================================
    // Зарезервированные коды для будущего использования
    // (раскомментируйте по мере необходимости)
    //==========================================================================
    // ERROR_MEMORY_SYSTEM_INIT = 5,
    // ERROR_EVENT_SYSTEM_INIT = 6,
    // ERROR_PLATFORM_INIT = 7,
    // ERROR_APPLICATION_INIT = 8,
    // ERROR_GAME_STATE_ALLOCATION = 10,
    // ERROR_MEMORY_ALLOCATION_FAILED = 20,
    // ERROR_MEMORY_FREE_FAILED = 21,
    // ERROR_EVENT_REGISTER_DUPLICATE = 30,
    // ERROR_EVENT_UNREGISTER_NOT_FOUND = 31,
    // ERROR_EVENT_SYSTEM_NOT_INITIALIZED = 32,
    // ERROR_DARRAY_CREATE_FAILED = 40,
    // ERROR_DARRAY_RESIZE_FAILED = 41,
    // ERROR_DARRAY_INDEX_OUT_OF_BOUNDS = 42,
    // ERROR_PLATFORM_WINDOW_CREATION_FAILED = 50,
    // ERROR_PLATFORM_WINDOW_CLASS_REGISTRATION_FAILED = 51,
    // ERROR_UNKNOWN = 99
};