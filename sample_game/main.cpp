#define NOMINMAX
#define _HAS_STD_BYTE 0

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/shader.h"
#include "engine/camera.h"
#include "engine/scene_object.h"
#include "engine/functions.h"
#include "engine/game_api_state.h"

#define STB_IMAGE_IMPLEMENTATION
#include "engine/model.h"

#include <iostream>
#include <cstdio>
#include <limits>
#include <random>
#include <cmath>

// This is to implement shared memory for RL
#include <windows.h>
#include <cstring>

const unsigned int ENEMY_NUMBER = 5;

// Create a struct for the information we would like to write to the file
struct GameState {
    float player_pos[3];
    float enemy_pos[ENEMY_NUMBER][3];
    int enemy_alive[ENEMY_NUMBER];
    int enemy_count;
    int enemy_hit;
    int player_hit;
    int action;
    float timer;
    int ready;
};

// randomness
std::random_device rd;
static std::mt19937 rng{rd()}; 

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float ENEMY_MOVE_SPEED = 4.0f;
const float ENEMY_STOP_DISTANCE = 3.0f;
const float BULLET_MOVE_SPEED = 25.0f;
const float BULLET_HIT_DISTANCE = ENEMY_STOP_DISTANCE + 1.0f;

//Player positions
const glm::vec3 INITIAL_PLAYER_POS = glm::vec3(-15.0f, -25.0f, -9.0f);
glm::vec3 playerPos = INITIAL_PLAYER_POS;
float cameraDistanceZ = 60.0f;
float cameraDistanceX = 53.0f;
float cameraDistanceY = 15.0f;
glm::vec3 cameraPos = glm::vec3(playerPos.x + cameraDistanceX, playerPos.y + cameraDistanceY, playerPos.z + cameraDistanceZ);

// camera
Camera camera(cameraPos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool fixCamera = true;
bool fixCursor = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Scale vector
glm::vec3 scaled_tiles(1.0f, 1.0f, 1.0f);
glm::vec3 scaled_cube(0.1f, 0.1f, 0.1f);


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, glm::vec3 playerLocalCenter, const std::vector<BoundingBox>& colliders, int action);
bool checkCollision(glm::vec3 playerPos, glm::vec3 cubeMin, glm::vec3 cubeMax);
void LogVec(glm::vec3 vecToLog, std::string name);
void cursor_enter_callback(GLFWwindow* window, int entered);
glm::mat4 model_matrices_enemy(EnemyProperties& enemy_prop, const Model& Enemy, const glm::vec3& playerPos, float deltatime);
glm::mat4 model_matrices_bullet(BulletProperties& bullet_prop, const Model& Bullet, float deltatime);
struct BoundingBox;
BoundingBox model_boundaries(Model Cube, ObjectProperties obj);
BoundingBox model_boundaries_enemy(Model enemyModel, EnemyProperties enemy);
glm::vec3 rayPlaneIntersection(glm::vec3 ray, glm::vec3 playerPos, glm::vec3 cameraPos);
glm::vec3 RaycastCursor(double mouse_x, double mouse_y, int width, int height, glm::mat4 projection_matrix, glm::mat4 view_matrix);
glm::vec3 calculate_player_local_center(const Model& model);
void calculate_model_local_center(const Model& model, glm::vec3& outMin, glm::vec3& outMax);
glm::mat4 model_matrices_impose(ObjectProperties obj);
void reset_environment(std::vector<EnemyProperties>& sceneEntityEnemy,
                       std::vector<glm::mat4>& enemies,
                       std::vector<BoundingBox>& colliders,
                       std::vector<BulletProperties>& bullets,
                       size_t staticColliderCount,
                       const Model& Enemy,
                       const glm::vec3& enemyLocalCenter,
                       int number_of_enemies,
                       float min_x_radius,
                       float max_x_radius,
                       float min_z_radius,
                       float max_z_radius);

int main() {

    // Create shared memory
    HANDLE hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,   // use paging file
        NULL,                   // default security
        PAGE_READWRITE,         // read/write access
        0,                      // max object size (high)
        sizeof(GameState),      // max object size (low)
        "GameState"             // named shared memory
    );

    GameState* state = (GameState*)MapViewOfFile(
        hMapFile,               // handle to map object
        FILE_MAP_ALL_ACCESS,    // read/write permission
        0, 0,
        sizeof(GameState)
    );

    

    // Write to a file to check if we even reach main
    FILE* logFile = fopen("debug_log.txt", "w");
    if (logFile) {
        fprintf(logFile, "MAIN STARTED\n");
        fflush(logFile);
        fclose(logFile);
    }

    std::cout << "Starting application..." << std::endl;
    std::cout.flush();

    glfwInit();
    logFile = fopen("debug_log.txt", "a");
    if (logFile) { fprintf(logFile, "GLFW INIT\n"); fflush(logFile); fclose(logFile); }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "game", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create window" << std::endl;
        std::cout.flush();
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // Set your callbacks here - add mouse and scroll callback later
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    

    // Capture the mouse
    if(fixCursor)
    {   
        glfwSetCursorPosCallback(window, mouse_callback);
        // Constrain the cursor to the window content area.
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        glfwSetCursorEnterCallback(window, cursor_enter_callback);
    }
    
    unsigned char pixels[16 * 16 * 4];
    memset(pixels, 0xff, sizeof(pixels));
    
    GLFWimage image;
    image.width = 16;
    image.height = 16;
    image.pixels = pixels;
    
    GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);

    if(!cursor)
    {
        std::cout << "Failed to add texture to cursor" << std::endl;
        return -1;
    }
    glfwSetCursor(window, cursor);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::cout.flush();
        glfwTerminate();
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST); // enable the z-buffer -> opengl knows which triangle to render at any given point

    glEnable(GL_CULL_FACE); // performs face culling -> decides when to render triangles based on orientation (if they are front or back side)
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    Shader ourShader("shaders/sample.vs", "shaders/sample.fs");

    Model MarbelFloor("assets/floor-marble-tiled-floor/source/FloorTiledMarble.fbx");
    Model Player("assets/PlayerTemplate/source/objects/character.obj");
    Model Enemy("assets/PlayerTemplate/source/objects/character.obj");
    Model Cube("assets/cube/source/cube.obj");
    Model Bullet("assets/Bullet/Bullet.fbx");

    glm::vec3 playerLocalCenter = calculate_player_local_center(Player);
    glm::vec3 enemyLocalCenter = calculate_player_local_center(Enemy);
    glm::vec3 bulletLocalCenter = calculate_player_local_center(Bullet);
    glm::vec3 playerWorldTarget(0.0f, 0.0f, 0.0f);

    // Creating Scene Objects
    std::vector<ObjectProperties> sceneObjectsCube = getSceneObjectsCube(scaled_tiles);
    std::vector<BoundingBox> colliders;
    for (const auto& obj : sceneObjectsCube)
    {
        colliders.push_back(model_boundaries(Cube, obj));
    }
    const size_t staticColliderCount = colliders.size();


    std::vector<ObjectProperties> sceneObjectsMarbelFloor = getSceneObjectsMarbelFloor(scaled_tiles);
    std::vector<glm::mat4> floors;
    for (const auto& obj : sceneObjectsMarbelFloor)
    {
        floors.push_back(model_matrices_impose(obj));
    }

    int number_of_enemies = ENEMY_NUMBER;
    float min_x_radius = 10.0f;
    float min_z_radius = 10.0f;
    float max_x_radius = 30.0f;
    float max_z_radius = 30.0f;
    std::vector<EnemyProperties> sceneEntityEnemy = spawnEnemy(number_of_enemies, enemyLocalCenter, playerPos, min_x_radius, max_x_radius, min_z_radius, max_z_radius, rng);
    std::vector<glm::mat4> enemies(sceneEntityEnemy.size(), glm::mat4(1.0f));
    for (const auto& obj : sceneEntityEnemy)
    {
        colliders.push_back(model_boundaries_enemy(Enemy, obj));
    }
    api_update_snapshot(playerPos, sceneEntityEnemy);
    
    int i = 0;
    

    double xpos {394}, ypos {186}; // cursor position
    int width, height; // frame buffer size

    // Bullets
    std::vector<BulletProperties> bullets;
    bool resetKeyWasDown = false;


    unsigned int counter = 0;
    float episodeStartTime = static_cast<float>(glfwGetTime());

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {

        int cur_action = state->action;
        state->action = 0;
        state->enemy_hit = 0;
        state->player_hit = 0;

        //glfwGetCursorPos(window, &xpos, &ypos);
        // glfwSetCursorPos(window, xpos, ypos);
        glfwGetWindowSize(window, &width, &height);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        counter++;
        if (deltaTime >= 1.0 / 30.0)
        {
            std::string FPS = std::to_string((1.0 / deltaTime) * counter);
            std::string newTitle = "FPS- " + FPS;
            glfwSetWindowTitle(window, newTitle.c_str());
            lastFrame = currentFrame;
            counter = 0;
        }

        for (size_t j = 0; j < sceneEntityEnemy.size();)
        {
            glm::vec3 toPlayer = playerPos - sceneEntityEnemy[j].enemyPos;
            toPlayer.y = 0.0f;
            float distanceToPlayer = glm::length(toPlayer);

            if (distanceToPlayer <= ENEMY_STOP_DISTANCE + 1)
            {
                sceneEntityEnemy.erase(sceneEntityEnemy.begin() + static_cast<std::ptrdiff_t>(j));
                enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(j));
                colliders.erase(colliders.begin() + static_cast<std::ptrdiff_t>(staticColliderCount + j));
                state->player_hit = 1;
                continue;
            }

            enemies[j] = model_matrices_enemy(sceneEntityEnemy[j], Enemy, playerPos, deltaTime);
            colliders[staticColliderCount + j] = model_boundaries_enemy(Enemy, sceneEntityEnemy[j]);
            ++j;
        }

        for (BulletProperties& bullet : bullets)
        {
            bullet.bulletModel = model_matrices_bullet(bullet, Bullet, deltaTime);
        }


        const float bulletHitDistanceSq = BULLET_HIT_DISTANCE * BULLET_HIT_DISTANCE;
        for (size_t bulletIndex = 0; bulletIndex < bullets.size();)
        {
            bool bulletRemoved = false;

            for (size_t enemyIndex = 0; enemyIndex < sceneEntityEnemy.size();)
            {
                glm::vec3 enemyCenter = sceneEntityEnemy[enemyIndex].enemyPos + sceneEntityEnemy[enemyIndex].enemyLocalCenter;
                glm::vec3 bulletCenter = bullets[bulletIndex].bulletPos + bulletLocalCenter;
                glm::vec3 toEnemy = enemyCenter - bulletCenter;
                float distanceToEnemySq = glm::dot(toEnemy, toEnemy);

                if (distanceToEnemySq <= bulletHitDistanceSq)
                {
                    std::cout << "Collision detected" << std::endl;
                    api_record_enemy_hit();
                    sceneEntityEnemy.erase(sceneEntityEnemy.begin() + static_cast<std::ptrdiff_t>(enemyIndex));
                    enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(enemyIndex));
                    colliders.erase(colliders.begin() + static_cast<std::ptrdiff_t>(staticColliderCount + enemyIndex));
                    bullets.erase(bullets.begin() + static_cast<std::ptrdiff_t>(bulletIndex));
                    bulletRemoved = true;
                    state->enemy_hit = 1;
                    break;
                }

                ++enemyIndex;
            }

            if (!bulletRemoved)
            {
                ++bulletIndex;
            }
        }

        processInput(window, playerLocalCenter, colliders, cur_action);
        glm::vec3 playerPivotWorld = playerPos + playerLocalCenter;
        bool resetPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        bool resetRequestedByPython = api_consume_reset_request();
        state->timer = static_cast<float>(glfwGetTime()) - episodeStartTime;
        if ((resetPressed && !resetKeyWasDown) || resetRequestedByPython or cur_action == -1)
        {
            reset_environment(sceneEntityEnemy,
                              enemies,
                              colliders,
                              bullets,
                              staticColliderCount,
                              Enemy,
                              enemyLocalCenter,
                              number_of_enemies,
                              min_x_radius,
                              max_x_radius,
                              min_z_radius,
                              max_z_radius);
            episodeStartTime = static_cast<float>(glfwGetTime());
            state->enemy_count = ENEMY_NUMBER;
            state->timer = 0.0f;
            state->ready = 1;
        }
        resetKeyWasDown = resetPressed;
                    api_update_snapshot(playerPos, sceneEntityEnemy);

        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
    

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // gives the closest and furthest point we can see
        playerWorldTarget = playerPivotWorld;


        cameraPos = glm::vec3(playerPos.x + cameraDistanceX, playerPos.y + cameraDistanceY, playerPos.z + cameraDistanceZ);
        glm::mat4 view = camera.GetViewMatrix(cameraPos, playerWorldTarget, fixCamera);
        ourShader.setMat4("view", view); // indicates which direction and how much to move the world
        ourShader.setMat4("projection", projection); // 
        


        glm::vec3 ray_world = RaycastCursor(xpos, ypos, width, height, projection, view);
        glm::vec3 plane_point = rayPlaneIntersection(ray_world, playerWorldTarget, cameraPos);
        // Direction from player to cursor point, flattened on Y
        glm::vec3 lookDir = plane_point - playerPivotWorld;
        lookDir.y = 0.0f;

        glm::mat4 playerModel = glm::mat4(1.0f);
        playerModel = glm::translate(playerModel, playerPos);
        playerModel = glm::translate(playerModel, playerLocalCenter);

        if (glm::length(lookDir) > 0.001f) // guard against zero length
        {
            lookDir = glm::normalize(lookDir);
            float angle = atan2(-lookDir.x, -lookDir.z);
            playerModel = glm::rotate(playerModel, angle, glm::vec3(0, 1, 0));
        }

        playerModel = glm::translate(playerModel, -playerLocalCenter);
        
        // Reuse the same transform used for camera targeting.
        ourShader.setMat4("model", playerModel);
        Player.Draw(ourShader);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS or cur_action == 5)
        {
            glm::vec3 bullet_offset(0.0f, 1.0f, 0.0f);
            glm::vec3 spawnPos = playerPivotWorld - bulletLocalCenter + bullet_offset;
            glm::mat4 bulletMat = glm::mat4(1.0f);
            bulletMat = glm::translate(bulletMat, spawnPos);
            bulletMat = glm::translate(bulletMat, bulletLocalCenter);

            glm::vec3 bulletDir(0.0f, 0.0f, -1.0f);
            if (glm::length(lookDir) > 0.001f) // guard against zero length
            {
                bulletDir = glm::normalize(lookDir);
                float angle = atan2(-bulletDir.x, -bulletDir.z);
                bulletMat = glm::rotate(bulletMat, glm::radians(90.0f), glm::vec3(0,1,0));
                bulletMat = glm::rotate(bulletMat, angle, glm::vec3(0, 1, 0));
            }

            bulletMat = glm::translate(bulletMat, -bulletLocalCenter);
            BulletProperties cur_bullet = BulletProperties{spawnPos, bulletDir, bulletMat};
            bullets.push_back(cur_bullet);
        }



        // for (const glm::mat4& matrices : floors)
        // {
        //     ourShader.setMat4("model", matrices);
        //     MarbelFloor.Draw(ourShader);
        // }

        for (const BoundingBox& box : colliders)
        {
            ourShader.setMat4("model", box.transformation);
            Cube.Draw(ourShader);
        }

        for (const glm::mat4& matrix : enemies)
        {
            ourShader.setMat4("model", matrix);
            Enemy.Draw(ourShader);
        }
        
        for (BulletProperties& bullet : bullets)
        {
            ourShader.setMat4("model", bullet.bulletModel);
            Bullet.Draw(ourShader);
        }


        state -> player_pos[0] = playerPos.x;
        state -> player_pos[1] = playerPos.y;
        state -> player_pos[2] = playerPos.z;
        
        
        state->enemy_count = static_cast<int>(sceneEntityEnemy.size());

        for (int j = 0; j < state->enemy_count; j++)
        {
            state->enemy_pos[j][0] = sceneEntityEnemy[j].enemyPos.x;
            state->enemy_pos[j][1] = sceneEntityEnemy[j].enemyPos.y;
            state->enemy_pos[j][2] = sceneEntityEnemy[j].enemyPos.z;
            state->enemy_alive[j] = 1;
        }

        // Zero out remaining slots so Python doesn't read stale data
        for (int j = state->enemy_count; j < ENEMY_NUMBER; j++)
        {
            state->enemy_pos[j][0] = 0.0f;
            state->enemy_pos[j][1] = 0.0f;
            state->enemy_pos[j][2] = 0.0f;
            state->enemy_alive[j] = 0;
        }

        // if (i % 50 == 0)
        // {
        // LogVec(playerWorldTarget, "Player World ");
        // LogVec(playerLocalCenter, "Local Player Center");
        // LogVec(playerPos, "Player Position");
        // LogVec(plane_point, "Point of intersection");
        // LogVec(ray_world, "ray_world");
        // // LogVec(cubeBounds.modelMin, "cubeBounds.modelMin");
        // // LogVec(cubeBounds.modelMax, "cubeBounds.modelMax");
        // // std::cout << "cubeBounds.radius " << cubeBounds.radius << std::endl;
        // }
        // i += 1;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, glm::vec3 playerLocalCenter, const std::vector<BoundingBox>& colliders, int action) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || action == 1)
        camera.ProcessKeyboard(FORWARD, deltaTime, playerPos, playerLocalCenter, colliders, action);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || action == 3)
        camera.ProcessKeyboard(BACKWARD, deltaTime, playerPos, playerLocalCenter, colliders, action);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || action == 4)
        camera.ProcessKeyboard(LEFT, deltaTime, playerPos, playerLocalCenter, colliders, action);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || action == 2)
        camera.ProcessKeyboard(RIGHT, deltaTime, playerPos, playerLocalCenter, colliders, action);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Cursor position relative to top-left corner of the window content area:t
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // camera.ProcessMouseMovement(xoffset, yoffset);
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
    if (entered)
    {
        std::cout << "In the content area" << std::endl;
    }
    else
    {
        std::cout << "not in the content area" << std::endl;
    }
}

glm::vec3 calculate_player_local_center(const Model& model)
{
    glm::vec3 minPoint(std::numeric_limits<float>::max());
    glm::vec3 maxPoint(std::numeric_limits<float>::lowest());
    bool hasVertex = false;

    for (const auto& mesh : model.meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            hasVertex = true;
            minPoint.x = std::min(minPoint.x, vertex.Position.x);
            minPoint.y = std::min(minPoint.y, vertex.Position.y);
            minPoint.z = std::min(minPoint.z, vertex.Position.z);
            maxPoint.x = std::max(maxPoint.x, vertex.Position.x);
            maxPoint.y = std::max(maxPoint.y, vertex.Position.y);
            maxPoint.z = std::max(maxPoint.z, vertex.Position.z);
        }
    }

    if (!hasVertex)
    {
        return glm::vec3(0.0f);
    }

    return (minPoint + maxPoint) * 0.5f;
}

void calculate_model_local_center(const Model& model, glm::vec3& outMin, glm::vec3& outMax)
{
    outMin = glm::vec3(std::numeric_limits<float>::max());
    outMax = glm::vec3(std::numeric_limits<float>::lowest());
    bool hasVertex = false;

    for (const auto& mesh : model.meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            hasVertex = true;
            outMin.x = std::min(outMin.x, vertex.Position.x);
            outMin.y = std::min(outMin.y, vertex.Position.y);
            outMin.z = std::min(outMin.z, vertex.Position.z);
            outMax.x = std::max(outMax.x, vertex.Position.x);
            outMax.y = std::max(outMax.y, vertex.Position.y);
            outMax.z = std::max(outMax.z, vertex.Position.z);
        }
    }

    if (!hasVertex)
    {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(0.0f);
    }
}

void calculate_stride(Model MarbelFloor) {
    float current_min_x = INFINITY, current_min_y = INFINITY;
    float current_max_x = -INFINITY, current_max_y = -INFINITY;
    float min_x, min_y, max_x, max_y;
    for(unsigned int i = 0; i < MarbelFloor.meshes[0].vertices.size(); i++)
    {
        min_x = std::min(MarbelFloor.meshes[0].vertices[i].Position.x, current_min_x);
        min_y = std::min(MarbelFloor.meshes[0].vertices[i].Position.y, current_min_y);
        max_x = std::max(MarbelFloor.meshes[0].vertices[i].Position.x, current_max_x);
        max_y = std::max(MarbelFloor.meshes[0].vertices[i].Position.y, current_max_y);
        current_min_x = min_x;
        current_min_y = min_y;
        current_max_x = max_x;
        current_max_y = max_y;
    };
    float width = max_x - min_x;
    float depth = max_y - min_y;
    width *= scaled_tiles.x;
    depth *= scaled_tiles.y;
}


bool checkCollision(glm::vec3 playerPos, glm::vec3 cubeMin, glm::vec3 cubeMax) {
    // std::cout << "Collision compare -> playerPos: ("
    //           << playerPos.x << ", " << playerPos.y << ", " << playerPos.z
    //           << ") | boxMin: ("
    //           << cubeMin.x << ", " << cubeMin.y << ", " << cubeMin.z
    //           << ") | boxMax: ("
    //           << cubeMax.x << ", " << cubeMax.y << ", " << cubeMax.z
    //           << ")" << std::endl;

    bool collideX = playerPos.x >= cubeMin.x && playerPos.x <= cubeMax.x;
    bool collideZ = playerPos.z >= cubeMin.z && playerPos.z <= cubeMax.z;
    return collideX && collideZ;
}


void LogVec(glm::vec3 vecToLog, std::string name)
{
    std::cout << "This is vector " << name << " " << vecToLog.x << " " << vecToLog.y << " " << vecToLog.z << std::endl;
}

glm::vec3 RaycastCursor(double mouse_x, double mouse_y, int width, int height, glm::mat4 projection_matrix, glm::mat4 view_matrix)
{   
    // Converting to NDC [-1,1] range
    float x = (2.0f * mouse_x) / width - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / height;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);
    
    // point ray to the negative z direction
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    // Go backwards from clip space to eye space by inverse of projection matrix
    glm::vec4 ray_eye = glm::inverse(projection_matrix) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0); // ignore projection of z and w part

    // Move backward to world space
    glm::vec3 ray_wor = glm::vec3(glm::inverse(view_matrix) * ray_eye);
    // don't forget to normalize the vector at some point
    ray_wor = glm::normalize(ray_wor);

    return ray_wor; // We have the camera as the origin and direction as the cursor position
}

glm::vec3 rayPlaneIntersection(glm::vec3 ray, glm::vec3 playerPos, glm::vec3 cameraPos)
{
    glm::vec3 normal_vec(0.0f, 1.0f, 0.0f);
    float d = playerPos.y; // plane distance from origin
    float t = (d - glm::dot(cameraPos, normal_vec)) / glm::dot(ray, normal_vec);
    glm::vec3 point = cameraPos + t * ray;
    return point;
}

BoundingBox model_boundaries(Model Cube, ObjectProperties obj)
{
    glm::vec3 localMin, localMax;
    calculate_model_local_center(Cube, localMin, localMax);
    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(0.0, obj.y_translation, 0.0f));
    cubeModel = glm::translate(cubeModel, obj.world_translation);
    glm::mat4 cubeModelOffset = glm::translate(cubeModel, glm::vec3(0.0f, 0.0f, obj.z_offset));
    if (obj.should_rotate)
    {
        cubeModel = glm::rotate(cubeModel, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        cubeModelOffset = glm::rotate(cubeModelOffset, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    }
    cubeModel = glm::scale(cubeModel, obj.scale);
    cubeModelOffset = glm::scale(cubeModelOffset, obj.scale);
   

    glm::vec3 localCorners[8] = {
        glm::vec3(localMin.x, localMin.y, localMin.z),
        glm::vec3(localMax.x, localMin.y, localMin.z),
        glm::vec3(localMin.x, localMax.y, localMin.z),
        glm::vec3(localMax.x, localMax.y, localMin.z),
        glm::vec3(localMin.x, localMin.y, localMax.z),
        glm::vec3(localMax.x, localMin.y, localMax.z),
        glm::vec3(localMin.x, localMax.y, localMax.z),
        glm::vec3(localMax.x, localMax.y, localMax.z)
    };

    glm::vec3 worldMin(std::numeric_limits<float>::max());
    glm::vec3 worldMax(std::numeric_limits<float>::lowest());

    for (const glm::vec3& localCorner : localCorners)
    {
        glm::vec3 worldCorner = glm::vec3(cubeModel * glm::vec4(localCorner, 1.0f));
        worldMin.x = std::min(worldMin.x, worldCorner.x);
        worldMin.y = std::min(worldMin.y, worldCorner.y);
        worldMin.z = std::min(worldMin.z, worldCorner.z);
        worldMax.x = std::max(worldMax.x, worldCorner.x);
        worldMax.y = std::max(worldMax.y, worldCorner.y);
        worldMax.z = std::max(worldMax.z, worldCorner.z);
    }

    glm::vec3 cubeMin(worldMin.x - obj.radius, worldMin.y, worldMin.z - obj.radius);
    glm::vec3 cubeMax(worldMax.x + obj.radius, worldMax.y, worldMax.z + obj.radius);
    return BoundingBox{cubeMin, cubeMax, obj.radius, cubeModelOffset};
}

BoundingBox model_boundaries_enemy(Model enemyModel, EnemyProperties enemy)
{
    glm::vec3 localMin, localMax;
    calculate_model_local_center(enemyModel, localMin, localMax);
    
    // Create transformation matrix (just translation, no rotation for simplicity)
    glm::mat4 enemyTransform = glm::mat4(1.0f);
    enemyTransform = glm::translate(enemyTransform, enemy.enemyPos);
    
    // Transform all 8 corners (same as original model_boundaries for consistency)
    glm::vec3 localCorners[8] = {
        glm::vec3(localMin.x, localMin.y, localMin.z),
        glm::vec3(localMax.x, localMin.y, localMin.z),
        glm::vec3(localMin.x, localMax.y, localMin.z),
        glm::vec3(localMax.x, localMax.y, localMin.z),
        glm::vec3(localMin.x, localMin.y, localMax.z),
        glm::vec3(localMax.x, localMin.y, localMax.z),
        glm::vec3(localMin.x, localMax.y, localMax.z),
        glm::vec3(localMax.x, localMax.y, localMax.z)
    };
    
    glm::vec3 worldMin(std::numeric_limits<float>::max());
    glm::vec3 worldMax(std::numeric_limits<float>::lowest());
    
    for (const glm::vec3& corner : localCorners)
    {
        glm::vec3 worldCorner = glm::vec3(enemyTransform * glm::vec4(corner, 1.0f));
        worldMin = glm::min(worldMin, worldCorner);
        worldMax = glm::max(worldMax, worldCorner);
    }
    
    float enemyRadius = 1.0f;
    return BoundingBox{
        worldMin - glm::vec3(enemyRadius),
        worldMax + glm::vec3(enemyRadius),
        enemyRadius,
        enemyTransform
    };
}

glm::mat4 model_matrices_impose(ObjectProperties obj)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, obj.y_translation, 0.0));
    model = glm::translate(model, obj.world_translation);
    if (obj.should_rotate)
    {
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    }
    model = glm::scale(model, obj.scale);
    return model;
}

void reset_environment(std::vector<EnemyProperties>& sceneEntityEnemy,
                       std::vector<glm::mat4>& enemies,
                       std::vector<BoundingBox>& colliders,
                       std::vector<BulletProperties>& bullets,
                       size_t staticColliderCount,
                       const Model& Enemy,
                       const glm::vec3& enemyLocalCenter,
                       int number_of_enemies,
                       float min_x_radius,
                       float max_x_radius,
                       float min_z_radius,
                       float max_z_radius)
{
    playerPos = INITIAL_PLAYER_POS;

    bullets.clear();
    colliders.resize(staticColliderCount);

    sceneEntityEnemy = spawnEnemy(number_of_enemies,
                                  enemyLocalCenter,
                                  playerPos,
                                  min_x_radius,
                                  max_x_radius,
                                  min_z_radius,
                                  max_z_radius,
                                  rng);

    enemies.assign(sceneEntityEnemy.size(), glm::mat4(1.0f));
    for (const auto& enemy : sceneEntityEnemy)
    {
        colliders.push_back(model_boundaries_enemy(Enemy, enemy));
    }
}

glm::mat4 model_matrices_enemy(EnemyProperties& enemy_prop, const Model& Enemy, const glm::vec3& playerPos, float deltatime)
{
    glm::vec3 toPlayer = playerPos - enemy_prop.enemyPos;
    toPlayer.y = 0.0f;
    float distanceToPlayer = glm::length(toPlayer);

    if (distanceToPlayer > ENEMY_STOP_DISTANCE)
    {
        glm::vec3 moveDir = toPlayer / distanceToPlayer;
        float maxStep = ENEMY_MOVE_SPEED * deltatime;
        float step = std::min(maxStep, distanceToPlayer - ENEMY_STOP_DISTANCE);
        enemy_prop.enemyPos += moveDir * step;
    }

    glm::mat4 enemyModel(1.0f);
    enemyModel = glm::translate(enemyModel, enemy_prop.enemyPos);

    glm::vec3 lookDir = playerPos - enemy_prop.enemyPos;
    lookDir.y = 0.0f;
    enemyModel = glm::translate(enemyModel, enemy_prop.enemyLocalCenter);
    if (glm::length(lookDir) > 0.001f) // guard against zero length
    {
        lookDir = glm::normalize(lookDir);
        float angle = static_cast<float>(std::atan2(-lookDir.x, -lookDir.z));
        enemyModel = glm::rotate(enemyModel, angle, glm::vec3(0, 1, 0));
    }
    enemyModel = glm::translate(enemyModel, -enemy_prop.enemyLocalCenter);

    return enemyModel;
}

glm::mat4 model_matrices_bullet(BulletProperties& bullet_prop, const Model& Bullet, float deltatime)
{
    bullet_prop.bulletPos += bullet_prop.bulletDir * BULLET_MOVE_SPEED * deltatime;

    glm::mat4 bulletModel(1.0f);
    bulletModel = glm::translate(bulletModel, bullet_prop.bulletPos);

    glm::vec3 bulletLocalCenter = calculate_player_local_center(Bullet);
    bulletModel = glm::translate(bulletModel, bulletLocalCenter);
    if (glm::length(bullet_prop.bulletDir) > 0.001f)
    {
        glm::vec3 moveDir = glm::normalize(bullet_prop.bulletDir);
        float angle = static_cast<float>(std::atan2(-moveDir.x, -moveDir.z));
        bulletModel = glm::rotate(bulletModel, glm::radians(90.0f), glm::vec3(0, 1, 0));
        bulletModel = glm::rotate(bulletModel, angle, glm::vec3(0, 1, 0));
    }
    bulletModel = glm::translate(bulletModel, -bulletLocalCenter);

    return bulletModel;
}