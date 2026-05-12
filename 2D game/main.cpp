#define NOMINMAX
#define _HAS_STD_BYTE 0

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Header Files/shader.h"
#include "Header Files/camera.h"
#include "Header Files/scene_object.h"
#include "Header Files/functions.h"
#include "Header Files/graphicsSetup.h"
#include "Header Files/object_helpers.h"
#include "Header Files/model.h"
#include "Header Files/player.h"
#include "Header Files/sprite_sheet.h"

#include <iostream>
#include <cstdio>
#include <limits>
#include <random>
#include <cmath>

#include <cstring>

// randomness
std::random_device rd;
static std::mt19937 rng{rd()}; 

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool fixCamera = false;

// Scale vector
glm::vec3 scaled_tiles(1.0f, 1.0f, 1.0f);


void processInput(GLFWwindow* window, float deltaTime, Camera& camera, SpriteSheet& player);

int main() {

    GlfwSetup game_window(SCR_WIDTH, SCR_HEIGHT, "game");
    game_window.configure_callbacks();
    game_window.configure_cursor();
    game_window.configure_glad();

    // stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST); // enable the z-buffer -> opengl knows which triangle to render at any given point

    Shader ourShader("shaders/sample.vs", "shaders/sample.fs");

    Model MarbelFloor("assets/floor-marble-tiled-floor/source/FloorTiledMarble.fbx");


    Model cube_model("assets/blender_minecraft/minecraft.obj");
    cube_model.set_world_center(glm::vec3(-15.0f, 0.0f, -9.0f));
    cube_model.compute_and_set_local_center_min_max_coords();
    cube_model.compute_and_set_local_corners();

    SpriteSheet sprite_sheet_model("assets/asperite_sprite_sheet/player_sprite_run.png", "assets/asperite_sprite_sheet/player_sprite_run.json");
    sprite_sheet_model.set_world_center(glm::vec3(-15.0f, 1.0f, -9.0f));
    sprite_sheet_model.compute_and_set_local_center_min_max_coords();
    sprite_sheet_model.compute_and_set_local_corners();

    Camera camera;
    camera.set_relative_camera_position(sprite_sheet_model.object_world_center);
    glm::vec3 camera_lookat_target(0.0f, 0.0f, 0.0f);

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(game_window.get_window())) {

        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use_shader_program();
        game_window.compute_fps();
        game_window.update_window_width_height();
        processInput(game_window.get_window(), game_window.get_delta_time(), camera, sprite_sheet_model);

        
        camera.set_relative_camera_position(sprite_sheet_model.object_world_center);
        
        glm::vec3 camera_lookat_target = sprite_sheet_model.object_world_center + sprite_sheet_model.object_local_center;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // gives the closest and furthest point we can see
        glm::mat4 view = camera.get_view_matrix(camera.Position, camera_lookat_target, fixCamera);
        ourShader.setMat4("view", view); // indicates which direction and how much to move the world
        ourShader.setMat4("projection", projection); // 

        glm::mat4 player_model_matrix = glm::mat4(1.0f);
        player_model_matrix = glm::translate(player_model_matrix, sprite_sheet_model.object_world_center);
        player_model_matrix = glm::rotate(player_model_matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        player_model_matrix = glm::scale(player_model_matrix, glm::vec3(5.0, 5.0, 5.0));
        sprite_sheet_model.compute_and_set_world_min_and_max(player_model_matrix);
        sprite_sheet_model.compute_and_set_model_bounds();
        ourShader.setMat4("model", player_model_matrix);
        sprite_sheet_model.increment_current_frame();
        sprite_sheet_model.Draw();

        glm::mat4 cube_model_matrix = glm::mat4(1.0f);
        cube_model_matrix = glm::translate(cube_model_matrix, cube_model.object_world_center);
        cube_model.compute_and_set_world_min_and_max(cube_model_matrix);
        cube_model.compute_and_set_model_bounds();
        ourShader.setMat4("model", cube_model_matrix);
        cube_model.Draw(ourShader);

        glfwSwapBuffers(game_window.get_window());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
};

void processInput(GLFWwindow* window, float deltaTime, Camera& camera, SpriteSheet& player) {
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.process_camera_movement(FORWARD, deltaTime, player);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.process_camera_movement(BACKWARD, deltaTime, player);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.process_camera_movement(LEFT, deltaTime, player);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.process_camera_movement(RIGHT, deltaTime, player);

}