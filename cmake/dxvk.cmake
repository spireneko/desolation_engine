## Загрузка и распаковка готового бинарника dxvk для linux x64

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

set(DXVK_TARGET dxvk_native)
