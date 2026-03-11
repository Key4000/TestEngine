/**
 * @file application.cpp
 * @brief Реализация класса Application и его внутреннего состояния.
 *
 * Содержит логику главного цикла, инициализацию платформы и игрового кода,
 * а также обработку события выхода.
 */

#include <game_interface.hpp>
#include <core/application.hpp>
#include <core/logger.hpp>
#include <core/te_memory.hpp>
#include <core/event.hpp>
#include <platform/platform.hpp>
#include <string.h>

//==============================================================================
// Внутренняя структура состояния приложения
//==============================================================================

/**
 * @struct Application::ApplicationState
 * @brief Скрытая реализация состояния приложения (PImpl).
 *
 * Хранит все переменные, необходимые для работы главного цикла и управления
 * платформой.
 */
struct Application::ApplicationState {
    Game* game_inst;      ///< Указатель на структуру Game (игровой интерфейс).
    b8 is_running;        ///< Флаг работы главного цикла.
    b8 is_suspended;      ///< Флаг приостановки приложения (например, при свёртывании окна).
    platform_state platform; ///< Внутреннее состояние платформы.
    i16 width;            ///< Текущая ширина окна.
    i16 height;           ///< Текущая высота окна.
    f64 last_time;        ///< Временная метка предыдущего кадра (для расчёта delta_time).
};

//==============================================================================
// Конструктор / Деструктор
//==============================================================================

Application::Application(Game* game) {
    // 1. Выделяем память под внутреннее состояние
    app_state = static_cast<ApplicationState*>(te_memory_allocate(sizeof(ApplicationState), MEMORY_TAG_APPLICATION));
    app_state->game_inst = game;

    // 2. Готовим данные для создания окна
    window_data w_data;
    w_data.x = game->app_config.start_pos_x;
    w_data.y = game->app_config.start_pos_y;
    w_data.width = game->app_config.start_width;
    w_data.height = game->app_config.start_height;
    w_data.app_name = game->app_config.name;

    // 3. Инициализация платформы (создание окна и т.д.)
    if (!platform_init(&app_state->platform, &w_data)) {
        TE_LOG_FATAL("Platform не удалось инициализировать: file->application.cpp, func->конструктор класса Application");
        delete app_state;
        app_state = nullptr;
        return;
    }

    // 4. Регистрируем обработчик события выхода
    if (!event_register(EVENT_CODE_APPLICATION_QUIT, this, application_on_quit)) {
        TE_LOG_WARN("Не удалось зарегистрировать обработчик QUIT: file -> application.cpp, func -> конструктор Application");
    }

    // 5. Инициализация игры
    if (!app_state->game_inst->initialize(app_state->game_inst)) {
        TE_LOG_FATAL("Game не удалось инициализировать: file->application.cpp, func->конструктор класса Application");
        delete app_state;
        app_state = nullptr;
        return;
    }

    // 6. Сохраняем параметры окна, запускаем таймер и уведомляем игру о начальном размере
    app_state->width = w_data.width;
    app_state->height = w_data.height;
    app_state->last_time = platform_get_absolute_time();
    app_state->game_inst->on_resize(app_state->game_inst, app_state->width, app_state->height);
    app_state->is_running = true;
    app_state->is_suspended = false;
}

Application::~Application() {
    if (app_state) {
        event_unregister(EVENT_CODE_APPLICATION_QUIT, this, application_on_quit);
        platform_shutdown(&app_state->platform);
        te_memory_free(app_state, sizeof(ApplicationState), MEMORY_TAG_APPLICATION);
    }
}

//==============================================================================
// Главный цикл
//==============================================================================

b8 Application::run() {
    if (!app_state) {
        TE_LOG_ERROR("Application не инициализирован: file->application.cpp,func->run");
        return false;
    }

#ifdef TE_MEMORY_STATS_ENABLED
    char* memory_stats = get_memory_usage_str();
    TE_LOG_INFO(memory_stats);
    te_memory_free(memory_stats, strlen(memory_stats) + 1, MEMORY_TAG_STRING);
#endif

    // Главный цикл приложения
    while (app_state->is_running) {
        // Обработка сообщений ОС (оконные события, ввод)
        if (!platform_pump_messages(&app_state->platform)) {
            app_state->is_running = false;
            break;
        }

        // Расчёт времени кадра
        f64 current_time = platform_get_absolute_time();
        f32 delta_time = (f32)(current_time - app_state->last_time);
        app_state->last_time = current_time;

        // Обновление и рендер (если приложение не приостановлено)
        if (!app_state->is_suspended) {
            if (!app_state->game_inst->update(app_state->game_inst, delta_time)) {
                TE_LOG_FATAL("Обновление игры прошло неудачно, останавливаем главный цикл: file->application.cpp, func->run");
                app_state->is_running = false;
                break;
            }
            if (!app_state->game_inst->render(app_state->game_inst, delta_time)) {
                TE_LOG_FATAL("Рендер игры произошел неудачно , останавливаем главный цикл: file->application.cpp, func->run");
                app_state->is_running = false;
                break;
            }
        }
    }

    // Завершение игры (вызов shutdown)
    if (app_state->game_inst && app_state->game_inst->shutdown) {
        app_state->game_inst->shutdown(app_state->game_inst);
    }

    return true;
}

//==============================================================================
// Обработчик события выхода
//==============================================================================

b8 Application::application_on_quit(u16 code, void* sender, void* listener, EventContext context) {
    Application* app = (Application*)listener; // listener — это указатель на экземпляр Application
    app->app_state->is_running = false;
    return true; // событие обработано
}