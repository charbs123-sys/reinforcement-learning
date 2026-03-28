cmake_minimum_required(VERSION 3.15)
project(MyGame)

add_executable(MyGame 
    main.cpp
    libs/glad/src/glad.c
)

target_include_directories(MyGame PRIVATE 
    ${CMAKE_SOURCE_DIR}
    libs/glfw/include
    libs/glfw/include/glm
    libs/glad/include
)

target_link_libraries(MyGame PRIVATE 
    ${CMAKE_SOURCE_DIR}/libs/glfw/lib/glfw3.lib
    opengl32
)

add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/3.3.shader.vs
        $<TARGET_FILE_DIR:MyGame>/3.3.shader.vs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/3.3.shader.fs
        $<TARGET_FILE_DIR:MyGame>/3.3.shader.fs
)