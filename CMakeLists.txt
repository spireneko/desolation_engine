cmake_minimum_required(VERSION 3.28.3)
project(desolation_engine)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

# Папка с .cmake файлами
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Вывод git clone progress bar'ов
set(FETCHCONTENT_QUIET FALSE)

# Папки для подключения заголовков и динамических библиотек
set(PROJECT_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include")
set(PROJECT_LIB_DIR "${CMAKE_BINARY_DIR}/lib")

# Устанавливаем папки, куда всё будет компилироваться
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_LIB_DIR})

# Для Linux — чтобы бинарник искал .so рядом с собой
set(CMAKE_INSTALL_RPATH "$ORIGIN/lib;$ORIGIN;$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Флаги для Debug сборки
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -g -O0")
# Флаги для Release сборки
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -O2 -DNDEBUG")

# Избавляемся от стрёмных макросов винды
add_compile_definitions(NOMINMAX)

# Избавляемся от ворнингов, что мы не используем secure функции винды
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Сборка под x64 процессоры
set(CMAKE_GENERATOR_PLATFORM x64)

# Основной экзешник
add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)

# Зависимости собираем всегда в Release
set(ORIGINAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Release CACHE STRING "Forced Release for dependencies" FORCE)

## Скачиваем SDL3
# include(sdl)

## Скачиваем slang
# include(slang)

## Скачиваем DXVK
if(LINUX)
    include(dxvk)
endif()

# Возвращаемся к оригинальному типу сборки
set(CMAKE_BUILD_TYPE "${ORIGINAL_BUILD_TYPE}" CACHE STRING "" FORCE)

## Подключаем всё это добро
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL3_TARGET} ${SLANG_TARGET})

if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${DXVK_TARGET})
endif()
