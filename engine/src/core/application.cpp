/**
 * @file application.cpp
 * @brief Реализация класса Application.
 *
 * Выполняет инициализацию всех подсистем в правильном порядке:
 * логгер, память, события, ввод, платформа, игра.
 */

#include <game_interface.hpp>
#include <core/application.hpp>
#include <core/logger.hpp>
#include <core/te_memory.hpp>
#include <core/event.hpp>
#include <core/input.hpp>
#include <platform/platform.hpp>
#include <string.h>
#include <cstdio>

//==============================================================================
// Внутренняя структура состояния
//==============================================================================

struct Application::ApplicationState {
    Game* game_inst;          ///< Указатель на игровой интерфейс
    b8 is_running;            ///< Флаг работы главного цикла
    b8 is_suspended;          ///< Флаг приостановки (окно свёрнуто)
    platform_state platform;  ///< Состояние платформы
    i16 width;                ///< Текущая ширина окна
    i16 height;               ///< Текущая высота окна
    f64 last_time;            ///< Время предыдущего кадра (для расчёта дельты)
};

//==============================================================================
// Конструктор (инициализация всех подсистем)
//==============================================================================

Application::Application(Game* game) {
    /*
     *------------------------------
     *  инициализация подсистем
     *------------------------------
     */
    // Логгер – инициализируется самым первым, чтобы можно было логировать ошибки
    if (!log_init()) {
        // Если логгер не работает, используем printf для критического сообщения
        printf("Критическая ошибка: не удалось инициализировать систему логирования: file->application.cpp, func->Application::Application\n");
        app_state = nullptr;
        return;
    }
    // Система памяти
    memory_system_init();
    // Система событий
    if (!event_init()) {
        TE_LOG_FATAL("Не удалось инициализировать систему событий: file->application.cpp, func->Application::Application");
        app_state = nullptr;
        return;
    }
    // Система ввода (переименована в input_init)
    input_init();  // тоже пока без возврата кода
    /*
     *------------------------------
     * Выделение памяти под состояние приложения(а так же окно)
     *------------------------------
     */
    app_state = static_cast<ApplicationState*>(
        te_memory_allocate(sizeof(ApplicationState), MEMORY_TAG_APPLICATION));
    if (!app_state) {
        TE_LOG_FATAL("Не удалось выделить память для состояния приложения: file->application.cpp, func->Application::Application");
        app_state = nullptr;
        return;
    }
    app_state->game_inst = game;
    /*
     *------------------------------
     * Подготовка данных для создания окна и создание окна
     *------------------------------
     */
    window_data w_data;
    w_data.x = game->app_config.start_pos_x;
    w_data.y = game->app_config.start_pos_y;
    w_data.width = game->app_config.start_width;
    w_data.height = game->app_config.start_height;
    w_data.app_name = game->app_config.name;

    // Инициализация платформенного слоя (создание окна)
    if (!platform_init(&app_state->platform, &w_data)) {
        TE_LOG_FATAL("Не удалось инициализировать платформенный слой: file->application.cpp, func->Application::Application");
        te_memory_free(app_state, sizeof(ApplicationState), MEMORY_TAG_APPLICATION);
        app_state = nullptr;
        return;
    }
    /*
     *---------------------------------
     * регистрация обработчиков событий
     *---------------------------------
     */
    // Событие выхода(QUIT)
    if (!event_register(EVENT_CODE_APPLICATION_QUIT, this, application_on_quit)) {
        TE_LOG_WARN("Не удалось зарегистрировать обработчик QUIT: file->application.cpp, func->Application::Application");
    }
    // Клавиатура
    if (!event_register(EVENT_CODE_KEY_PRESSED, this, application_on_key)) {
        TE_LOG_WARN("Не удалось зарегистрировать обработчик KEY_PRESSED: file->application.cpp, func->Application::Application");
    }
    if (!event_register(EVENT_CODE_KEY_RELEASED, this, application_on_key)) {
        TE_LOG_WARN("Не удалось зарегистрировать обработчик KEY_RELEASED: file->application.cpp, func->Application::Application");
    }
    /*
     *---------------------------------
     * Инициализация игры
     *---------------------------------
     */
    if (!app_state->game_inst->initialize(app_state->game_inst)) {
        TE_LOG_FATAL("Не удалось инициализировать игру: file->application.cpp, func->Application::Application");
        platform_shutdown(&app_state->platform);
        te_memory_free(app_state, sizeof(ApplicationState), MEMORY_TAG_APPLICATION);
        app_state = nullptr;
        return;
    }
    /*
     *---------------------------------
     * Сохранение параметров окна и запуск таймера
     *---------------------------------
     */
    app_state->width = w_data.width;
    app_state->height = w_data.height;
    app_state->last_time = platform_get_absolute_time();
    app_state->game_inst->on_resize(app_state->game_inst, app_state->width, app_state->height);
    app_state->is_running = true;
    app_state->is_suspended = false;

    TE_LOG_INFO("Приложение успешно инициализировано: file->application.cpp, func->Application::Application");
}

//==============================================================================
// Деструктор (освобождение ресурсов в обратном порядке)
//==============================================================================

Application::~Application() {
    if (app_state) {
        // Отмена регистрации события выхода
        event_unregister(EVENT_CODE_APPLICATION_QUIT, this, application_on_quit);
        event_unregister(EVENT_CODE_KEY_PRESSED, this, application_on_key);
        event_unregister(EVENT_CODE_KEY_RELEASED, this, application_on_key);
        // Завершение работы платформы
        platform_shutdown(&app_state->platform);

        // Освобождение памяти состояния
        te_memory_free(app_state, sizeof(ApplicationState), MEMORY_TAG_APPLICATION);

        // Завершение работы подсистем (порядок обратный инициализации)
        input_shutdown();          // ввод
        event_shutdown();          // события
        memory_system_shutdown();  // память
        // log_shutdown() – не обязателен, но можно добавить
    }
}

//==============================================================================
// Главный цикл приложения
//==============================================================================

b8 Application::run() {
    if (!app_state) {
        TE_LOG_ERROR("Попытка запустить главный цикл при неинициализированном приложении: file->application.cpp, func->Application::run");
        return false;
    }

#ifdef TE_MEMORY_STATS_ENABLED
    char* memory_stats = get_memory_usage_str();
    TE_LOG_INFO(memory_stats);
    te_memory_free(memory_stats, strlen(memory_stats) + 1, MEMORY_TAG_STRING);
#endif

    while (app_state->is_running) {
        // Обработка сообщений ОС (оконные события, ввод)
        if (!platform_pump_messages(&app_state->platform)) {
            app_state->is_running = false;
            break;
        }

        // Обновление состояния ввода (выполняется после получения сообщений)
        input_update(0.0);  // delta_time пока не используется

        // Расчёт времени кадра
        f64 current_time = platform_get_absolute_time();
        f32 delta_time = (f32)(current_time - app_state->last_time);
        app_state->last_time = current_time;

        // Обновление и рендер, если приложение не приостановлено
        if (!app_state->is_suspended) {
            if (!app_state->game_inst->update(app_state->game_inst, delta_time)) {
                TE_LOG_FATAL("Обновление игры завершилось ошибкой, главный цикл остановлен: file->application.cpp, func->Application::run");
                app_state->is_running = false;
                break;
            }
            if (!app_state->game_inst->render(app_state->game_inst, delta_time)) {
                TE_LOG_FATAL("Рендер игры завершился ошибкой, главный цикл остановлен: file->application.cpp, func->Application::run");
                app_state->is_running = false;
                break;
            }
        }
    }

    // Вызов функции завершения игры (если она предоставлена)
    if (app_state->game_inst && app_state->game_inst->shutdown) {
        app_state->game_inst->shutdown(app_state->game_inst);
    }

    return true;
}

//==============================================================================
// Обработчик события выхода
//==============================================================================

b8 Application::application_on_quit(u16 code, void* sender, void* listener, EventContext context) {
    Application* app = static_cast<Application*>(listener);
    if (app->app_state) {
        app->app_state->is_running = false;
        TE_LOG_INFO("Получен сигнал завершения приложения: file->application.cpp, func->application_on_quit");
    }
    return true;
}
//==============================================================================
// Обработчик событий клавиатуры
//==============================================================================

b8 Application::application_on_key(u16 code, void* sender, void* listener, EventContext context) {
    // listener — это указатель на экземпляр Application (мы передавали this при регистрации)
    Application* app = static_cast<Application*>(listener);
    if (!app || !app->app_state) return false;

    // Извлекаем код клавиши из контекста
    u16 key_code = context.data.key.key_code;

    switch (code) {
        case EVENT_CODE_KEY_PRESSED:
            TE_LOG_DEBUG("Клавиша нажата: %d (file->application.cpp, func->application_on_key)", key_code);

            // Если нажата Escape — инициируем выход
            if (key_code == KEY_ESCAPE) {
                TE_LOG_INFO("Обнаружено нажатие Escape, отправляем событие QUIT: file->application.cpp, func->application_on_key");
                EventContext quit_context = {};
                event_fire(EVENT_CODE_APPLICATION_QUIT, nullptr, quit_context);
                return true;  // событие обработано
            }
            break;

        case EVENT_CODE_KEY_RELEASED:
            TE_LOG_DEBUG("Клавиша отпущена: %d (file->application.cpp, func->application_on_key)", key_code);
            break;

        default:
            return false;  // не обработано
    }

    // Возвращаем false, чтобы событие могло быть обработано другими подписчиками
    return false;
}