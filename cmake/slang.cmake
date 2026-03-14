# find_package(slang REQUIRED CONFIG HINTS "$ENV{VULKAN_SDK}/lib/cmake")

# Не используем find_package, создаем цель вручную
set(VULKAN_SDK $ENV{VULKAN_SDK})

if(LINUX)
    # Находим библиотеку (учитываем, что libslang.so - симлинк)
    find_library(SLANG_LIBRARY
        NAMES slang
        HINTS "${VULKAN_SDK}/lib"
        REQUIRED
    )
    # Проходим вдоль симлинков
    file(REAL_PATH "${SLANG_LIBRARY}" SLANG_LIBRARY)

    # Явно указываем include директорию
    set(SLANG_INCLUDE_DIR "${VULKAN_SDK}/include/slang")

    if(NOT SLANG_LIBRARY)
        message(FATAL_ERROR "Slang library not found in ${VULKAN_SDK}/lib")
    endif()

    message(STATUS "Found Slang library: ${SLANG_LIBRARY}")
    message(STATUS "Slang include dir: ${SLANG_INCLUDE_DIR}")

    # Создаем импортированную цель
    add_library(slang::slang SHARED IMPORTED GLOBAL)
    set_target_properties(slang::slang PROPERTIES
        IMPORTED_LOCATION "${SLANG_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
    )
else()
    find_package(slang CONFIG HINTS "${VULKAN_SDK}/lib/cmake")
endif()

set(SLANG_TARGET slang::slang)
