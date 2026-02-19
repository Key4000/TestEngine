/*
*  Реализация системы логирования
*  1. run-time фильтрация 
*  2. compile-time фильтрация 
*  3. вспомогательная функция для приведения уровня в строку 
*  4. основная функция логирования
*/

#include <core/logger.hpp>
#include <cstdio>   // fprintf, vfprintf, stderr  
#include <cstdarg>  // va_list, va_start, va_end

/*
*  run-time фильтрация 
*/

//
LogLevel glob_min_log_level = LogLevel::TRACE;

void te_set_log_level(LogLevel level){
  glob_min_log_level = level;
}

/*
* вспомогательная функция для приведения уровня в строку 
*/

static const char* level_to_string(LogLevel level)
{
  switch(level)
  {
    case LogLevel::TRACE:  return "[TRACE]";
    case LogLevel::DEBUG:  return "[DEBUG]";
    case LogLevel::INFO:   return "[INFO]";
    case LogLevel::WARN:   return "[WARN]";
    case LogLevel::ERROR:  return "[ERROR]";
    case LogLevel::FATAL:  return "[FATAL]";
    default:              return "[UNKNOWN]";
  }
}

/*
*  основная функция логирования
*/
void te_log_write(LogLevel level, const char* format, ...)
{
  if(level < glob_min_log_level)
  {
    return; //run-time фильтрация
  }
  
  fprintf(stderr, "%s : ", level_to_string(level));
  
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
  // fflush(stderr); // опционально для немедленного вывода
}

/*
* функция инициализации и завершения логировария 
*/ 

b8 initialize_logging()
{
  //TODO: дописать позже
  return true;
}

void shutdown_logging()
{
  //TODO: дописать позже
}

