cmake_minimum_required(VERSION 3.15)
project(MyGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASSIMP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/assimp/include)
set(ASSIMP_LIB_DEBUG ${CMAKE_SOURCE_DIR}/libs/assimp/lib/assimp-vc145-mtd.lib)
set(ASSIMP_DLL_DEBUG ${CMAKE_SOURCE_DIR}/libs/assimp/bin/assimp-vc145-mtd.dll)

add_executable(MyGame 
    main.cpp
    libs/glad/src/glad.c
)

target_include_directories(MyGame PRIVATE 
    ${CMAKE_SOURCE_DIR}
    libs/glfw/include
    libs/glfw/include/glm
    libs/glad/include
    ${ASSIMP_INCLUDE_DIR}
)

target_link_libraries(MyGame PRIVATE 
    ${CMAKE_SOURCE_DIR}/libs/glfw/lib/glfw3.lib
    ${ASSIMP_LIB_DEBUG}
    opengl32
)

add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/3.3.shader.vs
        $<TARGET_FILE_DIR:MyGame>/3.3.shader.vs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/3.3.shader.fs
        $<TARGET_FILE_DIR:MyGame>/3.3.shader.fs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ASSIMP_DLL_DEBUG}
        $<TARGET_FILE_DIR:MyGame>/assimp-vc145-mtd.dll
)