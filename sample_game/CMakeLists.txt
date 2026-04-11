cmake_minimum_required(VERSION 3.15)
project(MyGame)

set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libs)

set(ASSIMP_INCLUDE_DIR ${LIBS_DIR}/assimp/include)
set(ASSIMP_LIB_DEBUG ${LIBS_DIR}/assimp/lib/assimp-vc145-mtd.lib)
set(ASSIMP_DLL_DEBUG ${LIBS_DIR}/assimp/bin/assimp-vc145-mtd.dll)

add_executable(MyGame 
    main.cpp
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