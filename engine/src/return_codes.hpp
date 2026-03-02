/*
* Файл с кодами возвратов 
*/
#pragma once 

enum class ReturnCodes{
 SUCCESS = 0,
 ERROR_LOG_INIT = 1,
 ERROR_GAME_CREATE = 2,
 ERROR_MISSING_GAME_FUNC = 3,
 ERROR_ENGINE_APP_RUN = 4
};