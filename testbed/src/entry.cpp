/*
* реализация функции создания игры
*/
#include <entry.hpp>
#include "game.hpp"

b8 create_game(Game* game_inst){
  game_inst->app_config.start_pos_x = 100;
  game_inst->app_config.start_pos_y = 100;
  game_inst->app_config.start_width = 1280;
  game_inst->app_config.start_height = 720;
  game_inst->app_config.name = "TestEngine Testbed";
  game_inst->initialize = game_initialize;
  game_inst->update = game_update ;
  game_inst->render = game_render ;
  game_inst->on_resize = game_on_resize ;
  game_inst->shutdown = game_shutdown ;
  game_inst->state = nullptr;
  
  
  return true;
}