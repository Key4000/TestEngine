/**
 * @file entry.hpp
 * @brief Скрытая точка входа в движок.
 *
 * Этот файл содержит функцию `main`, которая инициализирует все системы движка
 * (логирование, память, события), создаёт экземпляр игры через функцию `create_game`,
 * проверяет корректность игрового интерфейса, запускает главный цикл приложения
 * и выполняет завершение. Игра не должна содержать собственную функцию `main` —
 * вся инициализация скрыта внутри движка.
 */

#pragma once

#include <defines.hpp>
#include <game_interface.hpp>
#include <return_codes.hpp>
#include <core/asserts.hpp>
#include <core/logger.hpp>
#include <core/application.hpp>
#include <core/te_memory.hpp>
#include <core/event.hpp>
#include <cstdio>
// #include <cstdlib>

/**
 * @brief Функция создания игры, предоставляемая клиентским кодом (testbed).
 * @param game_inst Указатель на структуру Game, которую необходимо заполнить.
 * @return true при успешном создании, false при ошибке.
 *
 * Эта функция должна быть реализована в проекте игры. Она заполняет переданную
 * структуру Game: задаёт конфигурацию окна (app_config) и указатели на игровые
 * функции (initialize, update, render, on_resize, shutdown). Также может выделить
 * память под внутреннее состояние игры и сохранить его в поле state.
 */
extern b8 create_game(Game* game_inst);

/**
 * @brief Точка входа в приложение.
 * @return Код возврата (0 при успехе, иное при ошибке).
 *
 * Последовательность действий:
 * 1. Инициализация системы логирования (`log_init`). При неудаче возвращает ERROR_LOG_INIT.
 * 2. Инициализация подсистемы памяти (`memory_system_init`).
 * 3. Инициализация системы событий (`event_init`).
 * 4. Создание экземпляра игры (`create_game`). При неудаче возвращает ERROR_GAME_CREATE.
 * 5. Проверка наличия всех обязательных функций игры (initialize, update, render, on_resize, shutdown).
 *    Если какая-то отсутствует — возвращает ERROR_MISSING_GAME_FUNC.
 * 6. Создание объекта Application и запуск главного цикла (`app_engine.run()`).
 *    При ошибке выполнения цикла возвращает ERROR_ENGINE_APP_RUN.
 * 7. Завершение работы систем (event_shutdown, memory_system_shutdown).
 * 8. Возврат SUCCESS.
 */
int main() {
    if (!log_init()) {
        printf("Системе логирования не удалось инициализироваться: file->entry.hpp, func->main");
        return static_cast<int>(ReturnCodes::ERROR_LOG_INIT);
    }
    memory_system_init();
    event_init();

    Game game_inst;
    if (!create_game(&game_inst)) {
        TE_LOG_FATAL("Не удалось создать игру: file->entry.hpp , func -> main");
        return static_cast<int>(ReturnCodes::ERROR_GAME_CREATE);
    }

    // Проверка наличия всех обязательных функций
    if (!game_inst.initialize) TE_LOG_ERROR("движок не видит initialize: file->entry.hpp, func->main ");
    if (!game_inst.update) TE_LOG_ERROR("движок не видит update: file->entry.hpp, func->main ");
    if (!game_inst.render) TE_LOG_ERROR("движок не видит render: file->entry.hpp, func->main ");
    if (!game_inst.on_resize) TE_LOG_ERROR("движок не видит on_resize: file->entry.hpp, func->main ");
    if (!game_inst.shutdown) TE_LOG_ERROR("движок не видит shutdown: file->entry.hpp, func->main ");

    if (!game_inst.initialize || !game_inst.update ||
        !game_inst.render || !game_inst.on_resize || !game_inst.shutdown) {
        TE_LOG_FATAL("движок не видит функции игры: file->entry.hpp, func->main ");
        return static_cast<int>(ReturnCodes::ERROR_MISSING_GAME_FUNC);
    }

    Application app_engine(&game_inst);
    if (!app_engine.run()) {
        TE_LOG_FATAL(" Главный цикл движка завершился ошибкой: file->entry.hpp, func->main");
        return static_cast<int>(ReturnCodes::ERROR_ENGINE_APP_RUN);
    }

    event_shutdown();
    memory_system_shutdown();
    return static_cast<int>(ReturnCodes::SUCCESS);
}