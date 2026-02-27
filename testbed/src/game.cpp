/*
* реализация конкретной игры по интерфейсу из движка 
*/

#include <game.hpp>
#include <core/logger.hpp>
#include <platform/platform.hpp>
//-------------------------------------
static b8 initialize(Game* game_inst)
{
  GameState* state = (GameState*)platform_allocate_memory(sizeof(GameState), false);
  if(!state)
  {
    TE_LOG_FATAL("Проблемы с выделением памяти под GameState(file->game.cpp,func->initialize)");
    return false;
  }
  platform_zero_memory(state, sizeof(GameState));
  game_inst->state = state;
  TE_LOG_INFO("Игра инициализирована!");
  return true;
}
//-------------------------------------
static b8 update(Game* game_inst, f32 delta_time)
{
  return true;
}
//-------------------------------------
static b8 render(Game* game_inst, f32 delta_time)
{
  return true;
}
//-------------------------------------
static void on_resize(Game* game_inst, u32 width, u32 height){
  TE_LOG_INFO("Изменили размер окна на %ux%u", width, height);
}
//-------------------------------------
static b8 shutdown(Game* game_inst)
{
  platform_free_memory(game_inst->state);
  game_inst->state = nullptr;
  return true;
}
//-------------------------------------
Game* create_game() {
    static Game game;
    game.initialize = initialize;
    game.update = update;
    game.render = render;
    game.on_resize = on_resize;
    game.shutdown = shutdown;
    game.state = nullptr;
    
    // Заполните конфигурацию приложения
    game.app_config.name = "TestGame";
    game.app_config.start_pos_x = 100;
    game.app_config.start_pos_y = 100;
    game.app_config.start_width = 1280;
    game.app_config.start_height = 720;
    return &game;
}
