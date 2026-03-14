/**
* @file application.hpp
* @brief Слой приложения, связывающий игру с внутренними механизмами движка.
*
* Этот файл содержит класс Application, который управляет главным циклом,
* инициализацией платформы и игрового кода, а также обработкой событий.
*/

#pragma once

#include <defines.hpp>

// Предварительное объявление структуры Game (определена в game_interface.hpp)
struct Game;

/**
* @brief Параметры конфигурации окна приложения.
*
* Эти параметры передаются на платформенный слой при создании окна.
* Заполняются игрой в структуре Game.
*/
struct ApplicationConfig {
  i16 start_pos_x; ///< Начальная позиция окна по оси X (в пикселях).
  i16 start_pos_y; ///< Начальная позиция окна по оси Y (в пикселях).
  i16 start_width; ///< Стартовая ширина окна (в пикселях).
  i16 start_height; ///< Стартовая высота окна (в пикселях).
  const char* name; ///< Заголовок окна (строка в кодировке UTF-8).
};

/**
* @brief Главный класс приложения.
*
* Инкапсулирует инициализацию, главный цикл и завершение работы движка.
* Использует идиому PImpl (Pointer to Implementation) для сокрытия внутреннего состояния.
*/
class TE_API Application {
  public:
  /**
  * @brief Конструктор приложения.
  * @param game Указатель на структуру Game, предоставленную игрой.
  *             Должен быть корректно заполнен (включая указатели на функции).
  */
  Application(Game* game);

  /**
  * @brief Деструктор приложения.
  * Освобождает ресурсы, завершает работу платформы.
  */
  ~Application();

  /**
  * @brief Запускает главный цикл приложения.
  * @return true, если цикл завершился успешно (по запросу выхода), false при ошибке.
  */
  b8 run();
  
  /**
 * @brief Проверяет, успешно ли проинициализировано приложение.
 * @return true, если конструктор отработал без ошибок, иначе false.
 */
b8 is_initialized() const { return app_state != nullptr; }

  private:
  /**
 * @brief Обработчик событий клавиатуры (нажатие/отпускание).
 * @param code Код события (EVENT_CODE_KEY_PRESSED или EVENT_CODE_KEY_RELEASED).
 * @param sender Отправитель события (не используется).
 * @param listener Указатель на экземпляр Application (передаётся при регистрации).
 * @param context Контекст события, содержащий код клавиши.
 * @return true, если событие обработано (например, при Escape), иначе false.
 */
b8 application_on_key(u16 code, void* sender, void* listener, EventContext context);
  /**
  * @brief Обработчик события выхода из приложения.
  * @param code Код события (должен быть EVENT_CODE_APPLICATION_QUIT).
  * @param sender Отправитель события (не используется).
  * @param listener Указатель на экземпляр Application (передаётся при регистрации).
  * @param context Контекст события (не используется).
  * @return true (событие обработано).
  */
  b8 application_on_quit(u16 code, void* sender, void* listener, EventContext context);

  /**
  * @brief Внутреннее состояние приложения (скрытая реализация).
  * @see ApplicationState
  */
  struct ApplicationState;

  ApplicationState* app_state; ///< Указатель на внутреннее состояние (PImpl).
};