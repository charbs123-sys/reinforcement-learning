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

    SpriteSheet player_sprite("assets/asperite_sprite_sheet/player_sprite_run.png", "assets/asperite_sprite_sheet/player_sprite_run.json");

    LoadMap level_one_map("assets/level_one/level_one.json", player_sprite.object_world_center, 5, 5);

    Camera camera;
    camera.set_relative_camera_position(player_sprite.object_world_center);
    glm::vec3 camera_lookat_target(0.0f, 0.0f, 0.0f);

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(game_window.get_window())) {

        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use_shader_program();
        game_window.compute_fps();
        game_window.update_window_width_height();
        processInput(game_window.get_window(), game_window.get_delta_time(), camera, player_sprite);
        std::cout << "this is th ecurrent time - " << game_window.get_delta_time() << std::endl;
        
        camera.set_relative_camera_position(player_sprite.object_world_center);
        // camera.configure_projection_view_matrices();
        
        glm::vec3 camera_lookat_target = player_sprite.object_world_center + player_sprite.object_local_center;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // gives the closest and furthest point we can see
        glm::mat4 view = camera.get_view_matrix(camera.Position, camera_lookat_target, fixCamera);
        ourShader.setMat4("view", view); // indicates which direction and how much to move the world
        ourShader.setMat4("projection", projection); // 

        player_sprite.Draw(ourShader);
        level_one_map.draw_map(ourShader);

        player_sprite.increment_current_frame_using_duration(game_window.get_delta_time());
        level_one_map.increment_sprites_for_map_with_duration(game_window.get_delta_time());

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