cmake_minimum_required(VERSION 3.28.3)
project(desolation_engine)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

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

# Включаем поддержку разных конфигураций
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Флаги для Debug сборки
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -g -O0")
# Флаги для Release сборки
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -O2 -DNDEBUG")

# Избавляемся от стрёмных макросов винды
add_compile_definitions(NOMINMAX)

# Избавляемся от ворнингов, что мы не используем secure функции винды
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Основной экзешник
add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)

## Подключаем SDL3
include(FetchContent)

set(SDL_VERSION 3.4.2)

# Опции для статической сборки SDL3
set(SDL_STATIC OFF CACHE BOOL "Не собирать статическую библиотеку" FORCE)
set(SDL_SHARED ON  CACHE BOOL "Собирать динамическую библиотеку SDL" FORCE)
set(SDL_TEST  OFF CACHE BOOL "Отключить тесты SDL" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG "release-${SDL_VERSION}"
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    UPDATE_DISCONNECTED TRUE
)

FetchContent_MakeAvailable(SDL3)
set(SDL3_TARGET SDL3-shared)

## Закачка и распаковка DXVK
if(LINUX)
    # URL для скачивания готового архива с GitHub Releases
    set(DXVK_VERSION "2.7.1")
    set(DXVK_ARCHIVE "dxvk-native-${DXVK_VERSION}-steamrt-sniper.tar.gz")
    set(DXVK_URL "https://github.com/doitsujin/dxvk/releases/download/v${DXVK_VERSION}/${DXVK_ARCHIVE}")

    # Временная папка для распаковки
    set(DXVK_TEMP_DIR "${CMAKE_BINARY_DIR}/dxvk_tmp")
    set(DXVK_INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/dxvk")

    # Скачиваем архив, если его ещё нет
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/${DXVK_ARCHIVE}")
        message(STATUS "Скачивание dxvk-native ${DXVK_VERSION}...")
        file(DOWNLOAD "${DXVK_URL}" "${CMAKE_CURRENT_BINARY_DIR}/${DXVK_ARCHIVE}" SHOW_PROGRESS)
    endif()

    # Распаковываем и копируем, если заголовочный файл ещё не установлен
    if(NOT EXISTS "${DXVK_INCLUDE_DIR}")
        message(STATUS "Распаковка dxvk-native...")
        file(ARCHIVE_EXTRACT INPUT "${CMAKE_BINARY_DIR}/${DXVK_ARCHIVE}"
             DESTINATION "${DXVK_TEMP_DIR}")
        
        # Копируем заголовки
        file(COPY "${DXVK_TEMP_DIR}/usr/include/" DESTINATION "${PROJECT_INCLUDE_DIR}")
        
        # Копируем библиотеки
        file(COPY "${DXVK_TEMP_DIR}/usr/lib/" DESTINATION "${PROJECT_LIB_DIR}")
        
        # Удаляем временную папку
        file(REMOVE_RECURSE "${DXVK_TEMP_DIR}")
    endif()

    # Создаём интерфейсную библиотеку для удобного подключения
    add_library(dxvk_native INTERFACE)
    target_include_directories(dxvk_native INTERFACE "${DXVK_INCLUDE_DIR}")
    target_link_directories(dxvk_native INTERFACE "${PROJECT_LIB_DIR}")
    target_link_libraries(dxvk_native INTERFACE
        dxvk_d3d11
        dxvk_dxgi
        # при необходимости добавьте другие:
        # dxvk_d3d9
        # dxvk_d3d10core
    )
endif()

## Подключаем всё это добро
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL3_TARGET})

if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dxvk_native)
endif()
