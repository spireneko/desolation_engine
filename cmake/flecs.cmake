find_package(flecs CONFIG REQUIRED)
set(FLECS_TARGET $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>)
