if(CMAKE_C_COMPILER_ID MATCHES "Clang" AND WIN32)
    # Find libomp.lib manually
    find_library(OMP_LIBRARY
        NAMES libomp omp
        PATHS
            "C:/Program Files/LLVM/lib"
            "C:/Program Files (x86)/LLVM/lib"
            ENV LIBRARY_PATH
        DOC "LLVM OpenMP library path"
    )

    if(NOT OMP_LIBRARY)
        get_filename_component(CLANG_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
        find_library(OMP_LIBRARY
            NAMES libomp omp
            PATHS "${CLANG_DIR}/../lib"
            NO_DEFAULT_PATH
        )
    endif()

    if(NOT OMP_LIBRARY)
        message(FATAL_ERROR "OpenMP library not found for Clang. Install LLVM with OpenMP support.")
    endif()

    message(STATUS "Found LLVM OpenMP: ${OMP_LIBRARY}")

    # Set CACHE variables to satisfy FindOpenMP for dependencies (DirectXTex, etc.)
    # These variables trick CMake's FindOpenMP into thinking detection already succeeded
    set(OpenMP_C_FLAGS "-fopenmp" CACHE STRING "OpenMP C flags" FORCE)
    set(OpenMP_CXX_FLAGS "-fopenmp" CACHE STRING "OpenMP CXX flags" FORCE)
    set(OpenMP_C_LIB_NAMES "libomp" CACHE STRING "OpenMP C lib names" FORCE)
    set(OpenMP_CXX_LIB_NAMES "libomp" CACHE STRING "OpenMP CXX lib names" FORCE)
    set(OpenMP_libomp_LIBRARY "${OMP_LIBRARY}" CACHE FILEPATH "OpenMP libomp library" FORCE)
    set(OpenMP_C_FOUND TRUE CACHE BOOL "OpenMP C found" FORCE)
    set(OpenMP_CXX_FOUND TRUE CACHE BOOL "OpenMP CXX found" FORCE)
    set(OpenMP_FOUND TRUE CACHE BOOL "OpenMP found" FORCE)

    # Create imported targets if they don't exist
    if(NOT TARGET OpenMP::OpenMP_C)
        add_library(OpenMP::OpenMP_C IMPORTED INTERFACE)
        set_property(TARGET OpenMP::OpenMP_C PROPERTY INTERFACE_COMPILE_OPTIONS "-fopenmp")
        set_property(TARGET OpenMP::OpenMP_C PROPERTY INTERFACE_LINK_LIBRARIES "${OMP_LIBRARY}")
    endif()

    if(NOT TARGET OpenMP::OpenMP_CXX)
        add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
        set_property(TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_COMPILE_OPTIONS "-fopenmp")
        set_property(TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_LINK_LIBRARIES "${OMP_LIBRARY}")
    endif()

    set(OPENMP_TARGET OpenMP::OpenMP_CXX)
else()
    find_package(OpenMP REQUIRED)
    set(OPENMP_TARGET OpenMP::OpenMP_CXX)
endif()
