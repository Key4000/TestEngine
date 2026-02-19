/*
 *   Все необходимые включения
 *  (переменные ,макросы для функций и 
 *   тд)
*/

#pragma once

// Беззнаковые целые
typedef unsigned char u8;       // 1 байт 
typedef unsigned short u16;     // 2 байта 
typedef unsigned int u32;       // 4 байта 
typedef unsigned long long u64; // 8 байт 

// Знаковые целые
typedef signed char i8;       // 1 байт
typedef signed short i16;     // 2 байта
typedef signed int i32;       // 4 байта
typedef signed long long i64; // 8 байт

// Числа с плавающей точкой
typedef float f32;  // 4 байта
typedef double f64; // 8 байт

// Логические типы
//b32 используется в некоторых структурах для совместимости с winAPI или для выравнивания
typedef int b32; // 4 байта
typedef bool b8; // 1 байт


/*
*  Проверка размеров типов
*  Это отловит странные платформы, где
*  int не 32 бита 
*  Если компилятор или платформа ведёт
*  себя неожиданно — компиляция упадёт
*  сразу, а не создаст скрытые баги.
*/
 
static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

static_assert(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

/*
* Экспорт/импорт символов для 
* динамической библиотеки
*/
#if defined(_WIN32) && defined(ENGINE_SHARED)
    #ifdef ENGINE_BUILD_DLL
        #define TE_API __declspec(dllexport)
    #else
        #define TE_API __declspec(dllimport)
    #endif
#else
    #define TE_API
#endif

/*
*  Определение платформы для кросс
*  -платформенного кода.
*  Устанавливает макросы вида 
*  TEST_PLATFORM_* в зависимости от 
*  целевой ОС у нас это windows
*/
// Запрещаем 32-битную сборку → движок строго 64-bit
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define TE_PLATFORM_WINDOWS 1
    // Требуем 64-битную сборку
    #ifndef _WIN64
        #error "64-bit is required on Windows!"
    #endif
#endif
//запрещаем сборку на другой платформе 
#ifndef _WIN32
    #error "Windows - единственная поддерживаемая платформа на данный момент."
#endif
  