cmake_minimum_required(VERSION 3.15)
project(MyGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libs)

set(ASSIMP_INCLUDE_DIR ${LIBS_DIR}/assimp/include)
set(ASSIMP_LIB_DEBUG ${LIBS_DIR}/assimp/lib/assimp-vc145-mtd.lib)
set(ASSIMP_DLL_DEBUG ${LIBS_DIR}/assimp/bin/assimp-vc145-mtd.dll)

add_executable(MyGame 
    main.cpp
    engine/game_api_state.cpp
    ${LIBS_DIR}/glad/src/glad.c
)

target_include_directories(MyGame PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBS_DIR}/glfw/include
    ${LIBS_DIR}/glfw/include/glm
    ${LIBS_DIR}/glad/include
    ${ASSIMP_INCLUDE_DIR}
)

target_link_libraries(MyGame PRIVATE 
    ${LIBS_DIR}/glfw/lib/glfw3.lib
    ${ASSIMP_LIB_DEBUG}
    opengl32
)

# Copy assimp DLL to build directory
add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_DLL_DEBUG}"
    "$<TARGET_FILE_DIR:MyGame>/assimp-vc145-mtd.dll"
    COMMENT "Copying assimp DLL..."
)

option(BUILD_PYTHON_BINDINGS "Build pybind11 Python module" ON)
if(BUILD_PYTHON_BINDINGS)
    # Auto-detect pybind11 cmake dir from the project venv Python
    if(NOT DEFINED pybind11_DIR)
        set(VENV_PYTHON "${CMAKE_CURRENT_SOURCE_DIR}/../.venv/Scripts/python.exe")
        execute_process(
            COMMAND "${VENV_PYTHON}" -c "import pybind11; print(pybind11.get_cmake_dir())"
            OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE PYBIND11_FIND_RESULT
        )
        if(NOT PYBIND11_FIND_RESULT EQUAL 0)
            message(FATAL_ERROR "Could not find pybind11 in venv at ${VENV_PYTHON}. Run: ${VENV_PYTHON} -m pip install pybind11")
        endif()
        set(pybind11_DIR "${PYBIND11_CMAKE_DIR}")
    endif()
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(game_api
        python_bindings.cpp
        engine/game_api_state.cpp
    )

    target_include_directories(game_api PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LIBS_DIR}/glfw/include/glm
    )
endif()