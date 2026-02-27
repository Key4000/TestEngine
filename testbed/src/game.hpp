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

Game* create_game();
b8 initialize(Game* game_inst);
b8 update(Game* game_inst, f32 delta_time); 
b8 render(Game* game_inst, f32 delta_time);
void on_resize(Game* game_inst, u32 width, u32 height);
b8 shutdown(Game* game_inst);


  
  
  