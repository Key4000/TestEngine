/*
*  Вводим свою систему утверждений 
*  Проверка условий на этапе выполнения
*  , для лучшей отладки(точки останова)
*  , а так же вывод логов 
*/
#pragma once 

#include <defines.hpp>
/*
*  Вспомогательные функции
*/

//функция обработки условий(asserts)
//принимает:1.строку с выражением expr
//2.доп.сообщение msg
//3.имя файла name
//4.номер строки line
TE_API void te_assert_fail(const char* expr, const char* msg, const char* name,i32 line);

/*
*  Вспомогательные
*  макросы 
*/

// Если этот макрос закомментировать, то все проверки утверждений будут
// отключены, и код в блоках #ifdef TE_ASSERT_ENABLED не будет выполняться.
#define TE_ASSERT_ENABLED

//запускаем условие
#ifdef TE_ASSERT_ENABLED

//определяем точку останова debugBreak
#define debugBreak() __debugbreak()

/*
* макросы для условий asserts
* (можно использовать и в realise)
*/

// Простое условие с остановкой
#define TE_ASSERT(expr) \
    do { \
        if(!(expr)) { \
            te_assert_fail(#expr, "", __FILE__, __LINE__); \
            debugBreak(); \
        } \
    } while(0)

// Условие с остановкой и дополнительным сообщением
#define TE_ASSERT_MSG(expr, msg) \
    do { \
        if(!(expr)) { \
            te_assert_fail(#expr, msg, __FILE__, __LINE__); \
            debugBreak(); \
        } \
    } while(0)
  
/*
*  макросы для условий asserts
*  (только для debug)
*/
 
 // Только для Debug-сборок
#ifdef _DEBUG
    #define TE_ASSERT_DEBUG(expr) \
        do { \
            if(!(expr)) { \
                te_assert_fail(#expr, "", __FILE__, __LINE__); \
                debugBreak(); \
            } \
        } while(0)
#else
    #define TE_ASSERT_DEBUG(expr) ((void)0)
#endif
 
/*
*   если TE_ASSERT_ENABLED будет    закомментирован 
*  , то открываем пустые assert
*/
#else
   #define TE_ASSERT(expr) ((void)0)
   #define TE_ASSERT_MSG(expr, msg) ((void)0)
   #define TE_ASSERT_DEBUG(expr) ((void)0)
#endif



 
  