/*
 * Слой приложения , связывающий testbed с внутренними механизмами движка
 */

#include <core/application.hpp>
#include <platform/platform.hpp>
#include <core/logger.hpp>
#include <game_interface.hpp>

//------------------------------------
struct Application::ApplicationState {
    Game* game_inst;  // Указатель на объект игры
    b8 is_running;    // Приложение запущено
    b8 is_suspended;  // Приложение приостановлено/свёрнуто
    platform_state platform;
    i16 width;
    i16 height;
    f64 last_time;  // Временная метка для расчёта дельты времени
};
//------------------------------------
Application::Application(Game* game) {
    /*
     * Создаем состояние
     */
    app_state = new ApplicationState();
    app_state->game_inst = game;
    /*
     * Подготовка данных окна
     */
    window_data w_data;
    w_data.x = game->app_config.start_pos_x;
    w_data.y = game->app_config.start_pos_y;
    w_data.width = game->app_config.start_width;
    w_data.height = game->app_config.start_height;
    w_data.app_name = game->app_config.name;
    /*
     *  Инициализируем платформу
     */
    if (!platform_init(&app_state->platform, &w_data)) {
        TE_LOG_FATAL("Platform не удалось инициализировать: file->application.cpp, func->конструктор класса Application");
        delete app_state;
        app_state = nullptr;
        return;
    }
    /*
     * Инициализация игры
     */
    if (!app_state->game_inst->initialize(app_state->game_inst)) {
        TE_LOG_FATAL("Game не удалось иниц ализировать: file->application.cpp, func->конструктор класса Application");
        delete app_state;
        app_state = nullptr;
        return;
    }
    /*
     * Сохраняем состояние
     * (размеры окна и таймер )
     * (уведомление о размере окна)
     * (меняем флаги)
     */
    app_state->width = w_data.width;
    app_state->height = w_data.height;
    app_state->last_time = platform_get_absolute_time();
    app_state->game_inst->on_resize(app_state->game_inst, app_state->width, app_state->height);
    app_state->is_running = true;
    app_state->is_suspended = false;
}
//------------------------------------
Application::~Application() {
    if (app_state) {
        platform_shutdown(&app_state->platform);
        delete app_state;
    }
}
//------------------------------------
b8 Application::run() {
    if (!app_state) {
        TE_LOG_ERROR("Application не инициализирован: file->application.cpp,func->run");
        return false;
    }
    /*
     * главный цикл
     */
    while (app_state->is_running) {
        /*
         * обработка сообщений ОС
         * и вычислем delta
         */
        if (!platform_pump_messages(&app_state->platform)) {
            app_state->is_running = false;
            break;
        }
        f64 current_time = platform_get_absolute_time();
        f32 delta_time = (f32)(current_time - app_state->last_time);
        app_state->last_time = current_time;
        /*
         * обновление и рендер
         * в конце вызываем завершение
         */
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
    if (app_state->game_inst && app_state->game_inst->shutdown) {
        app_state->game_inst->shutdown(app_state->game_inst);
    }
    return true;
}