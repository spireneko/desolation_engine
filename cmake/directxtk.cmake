message("${CMAKE_TOOLCHAIN_FILE}")

find_package(directxtk CONFIG REQUIRED)
set(DIRECTXTK_TARGET Microsoft::DirectXTK)
