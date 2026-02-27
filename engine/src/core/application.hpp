/*
 * Слой приложения , связывающий testbed с внутренними механизмами движка
 */
#pragma once

#include <defines.hpp>

struct Game;

/*
 * Эти параметры передаются на слой платформы при создании окна
 */
struct ApplicationConfig {
    // начальная позиция окна движка
    i16 start_pos_x;
    i16 start_pos_y;

    // стартовый размер окна движка
    i16 start_width;
    i16 start_height;
    // имя
    const char* name;
};

// TE_API ,потому что запускаться движок будет из testbed
class TE_API Application {
   public:
    // конструктор вместо create
    Application(Game* game);
    ~Application();

    b8 run();  // главный цикл

   private:
    struct ApplicationState;
    ApplicationState* app_state;  // скрытая реализация (pointer to implementation)
};