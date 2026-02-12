cmake_minimum_required(VERSION 3.20)
project(TestEngine VERSION 0.1.0 LANGUAGES C)

# Опции сборки
option(BUILD_TESTBED "Build testbed application" ON)
option(BUILD_SHARED_LIBS "Build engine as shared library" OFF)
option(USE_SANITIZER "Enable address sanitizer" OFF)

# Настройки компилятора для C
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Путь к external зависимостям
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Тип сборки
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Флаги для Clang
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror=implicit-function-declaration
        -Werror=incompatible-pointer-types
        -Werror=int-conversion
        -g
    )
    
    if(USE_SANITIZER)
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# Генерация compile_commands.json для VSCode
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Подключение подпроектов
add_subdirectory(engine)

if(BUILD_TESTBED)
    add_subdirectory(testbed)
endif()