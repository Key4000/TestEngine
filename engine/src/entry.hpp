/*
*  делаем сокрытие main ,от игры 
*  здесь , начиная с main , все стартует 
*  один движок запускает -> много игр 
*/
#pragma once 

#include <core/asserts.hpp>
#include <core/logger.hpp>
#include <core/application.hpp>
#include <game_interface.hpp>
#include <defines.hpp>
#include <return_codes.hpp>
//#include <cstdlib>

/*
* через эту функцию игра
* обращается к движку 
* и запускается
*/
extern b8 create_game(Game* game_inst);

int main() {
  if (!log_init()) {
    printf("Системе логирования не удалось инициализироваться: file->entry.hpp, func->main");
    return static_cast<int>(ReturnCodes::ERROR_LOG_INIT);
  }
  /* 
  * создаем экземпляр игры
  * и сразу делаем create_game
  */
  Game game_inst;
  if(!create_game(&game_inst))
  {
    TE_LOG_FATAL("Не удалось создать игру: file->entry.hpp , func -> main");
    return static_cast<int>(ReturnCodes::ERROR_GAME_CREATE);
  }
  /*
  * проверяем реализованы ли функции
  * через которые игра будет общаться с движком
  */
  if (!game_inst.initialize) TE_LOG_ERROR("движок не видит initialize: file->entry.hpp, func->main ");
  if (!game_inst.update) TE_LOG_ERROR("движок не видит update: file->entry.hpp, func->main ");
  if (!game_inst.render) TE_LOG_ERROR("движок не видит render: file->entry.hpp, func->main ");
   if (!game_inst.on_resize) TE_LOG_ERROR("движок не видит on_resize: file->entry.hpp, func->main ");
   if (!game_inst.shutdown) TE_LOG_ERROR("движок не видит shutdown: file->entry.hpp, func->main ");
   if (!game_inst.initialize || !game_inst.update || 
       !game_inst.render || !game_inst.on_resize || !game_inst.shutdown) {
       TE_LOG_FATAL("движок не видит функции игры: file->entry.hpp, func->main ");
       return static_cast<int>(ReturnCodes::ERROR_MISSING_GAME_FUNC);
   }
  /*
  * если игра создалась удачно
  * создаем(вызываем конструктор) движок
  * и вызываем запуск главного цикла
  * движка 
  */
  Application app_engine(&game_inst);
  if(!app_engine.run())
  {
    TE_LOG_FATAL(" Главный цикл движка завершился ошибкой: file->entry.hpp, func->main");
    return static_cast<int>(ReturnCodes::ERROR_ENGINE_APP_RUN);
  }
//system("pause");
return static_cast<int>(ReturnCodes::SUCCESS);
}

