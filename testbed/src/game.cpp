/*
 * реализация конкретной игры по интерфейсу из движка
 */

#include "game.hpp"
#include <core/logger.hpp>
#include <platform/platform.hpp>

//------------------------------------
b8 game_initialize(Game* game_inst) {
    GameState* state = (GameState*)platform_allocate_memory(sizeof(GameState), false);
    if (!state) {
        TE_LOG_FATAL("Проблемы с выделением памяти под GameState(file->game.cpp,func->initialize)");
        return false;
    }
    platform_zero_memory(state, sizeof(GameState));
    game_inst->state = state;
    TE_LOG_INFO("Игра инициализирована!");
    return true;
}
//------------------------------------
b8 game_update(Game* game_inst, f32 delta_time) {
    return true;
}
//------------------------------------
b8 game_render(Game* game_inst, f32 delta_time) {
    return true;
}
//------------------------------------
void game_on_resize(Game* game_inst, u32 width, u32 height) {
    TE_LOG_INFO("Изменили размер окна на %ux%u", width, height);
}
//------------------------------------
b8 game_shutdown(Game* game_inst) {
    platform_free_memory(game_inst->state, false);
    game_inst->state = nullptr;
    return true;
}