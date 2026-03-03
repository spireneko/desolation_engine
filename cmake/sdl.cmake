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
    USES_TERMINAL_DOWNLOAD TRUE
)

FetchContent_MakeAvailable(SDL3)
set(SDL3_TARGET SDL3-shared)
