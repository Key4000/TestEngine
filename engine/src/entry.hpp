/**
 * @file entry.hpp
 * @brief Точка входа в движок (скрытый main).
 *
 * Содержит только создание игры и запуск приложения. Вся инициализация
 * подсистем выполняется внутри конструктора Application.
 */

#pragma once

#include <defines.hpp>
#include <game_interface.hpp>
#include <return_codes.hpp>
#include <core/application.hpp>
#include <cstdio>

/**
 * @brief Функция создания игры, предоставляемая клиентским кодом (testbed).
 * @param game_inst Указатель на структуру Game, которую необходимо заполнить.
 * @return true при успехе, false при ошибке.
 */
extern b8 create_game(Game* game_inst);

/**
 * @brief Точка входа в приложение.
 * @return Код возврата (0 – успех, иное – ошибка).
 *
 * Последовательность действий:
 * 1. Вызов create_game для получения конфигурации игры.
 * 2. Создание объекта Application (все подсистемы инициализируются внутри).
 * 3. Проверка успешности инициализации через is_initialized().
 * 4. Запуск главного цикла через run().
 * 5. Возврат кода ошибки при необходимости.
 */
int main() {
    // Создаём экземпляр игры
    Game game_inst;
    if (!create_game(&game_inst)) {
        printf("Критическая ошибка: не удалось создать игру: file->entry.hpp, func->main\n");
        return static_cast<int>(ReturnCodes::ERROR_GAME_CREATE);
    }

    // Создаём и запускаем приложение
    Application app_engine(&game_inst);
    if (!app_engine.is_initialized()) {
        // Ошибка уже залогирована внутри конструктора Application
        return static_cast<int>(ReturnCodes::ERROR_APPLICATION_INIT);
    }

    if (!app_engine.run()) {
        return static_cast<int>(ReturnCodes::ERROR_ENGINE_APP_RUN);
    }

    return static_cast<int>(ReturnCodes::SUCCESS);
}