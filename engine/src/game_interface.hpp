/*
 * Файл  взаимодействия движка с testbed
 */
#pragma once

#include <core/application.hpp>

struct Game {
    // для хранения состояния игры , это состояние определяется в testbed game.hpp
    void* state;
    ApplicationConfig app_config;

    b8 (*initialize)(struct Game* game_inst);
    b8 (*update)(struct Game* game_inst, f32 delta_time);
    b8 (*render)(struct Game* game_inst, f32 delta_time);
    void (*on_resize)(struct Game* game_inst, u32 width, u32 height);
    b8 (*shutdown)(Game* game_inst);
};

/*
 * Указатели на функции (initialize, update, render, on_resize) — движок будет их вызывать.
 * Поле void* state — место, куда игра спрячет свои внутренние данные (движок про них ничего не знает).
 * Поле app_config — настройки окна (имя, размеры), которые игра сообщает движку.
 */
/*
 * Этот файл виден и движку, и игре.
 * Движок использует его, чтобы знать, какие функции вызывать.
 * Игра использует его, чтобы знать, какую структуру нужно заполнить.
 */