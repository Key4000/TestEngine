/*
* реализация конкретной игры по интерфейсу из движка 
*/
#pragma once

#include <defines.hpp>
#include <game_interface.hpp>

//внутренне состояние игры 
struct GameState{
  f32 delta_time;
};


b8 game_initialize(Game* game_inst);
b8 game_update(Game* game_inst, f32 delta_time); 
b8 game_render(Game* game_inst, f32 delta_time);
void game_on_resize(Game* game_inst, u32 width, u32 height);
b8 game_shutdown(Game* game_inst);

  
  
  