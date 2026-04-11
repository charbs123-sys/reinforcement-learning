#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/shader.h"
#include "engine/camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "engine/model.h"

#include <iostream>
#include <cstdio>
#include <limits>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//Player positions
glm::vec3 playerPos = glm::vec3(-15.0f, -25.0f, -9.0f);
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
void processInput(GLFWwindow* window, glm::vec3 playerLocalCenter, const std::vector<BoundingBox>& colliders);
bool checkCollision(glm::vec3 playerPos, glm::vec3 cubeMin, glm::vec3 cubeMax);
void LogVec(glm::vec3 vecToLog, std::string name);
void cursor_enter_callback(GLFWwindow* window, int entered);
struct BoundingBox;
BoundingBox model_boundaries(Model Cube, float radius, float y_translation, glm::vec3 world_translation, bool is_rotation, glm::vec3 scale, float z_offset);
glm::vec3 rayPlaneIntersection(glm::vec3 ray, glm::vec3 playerPos, glm::vec3 cameraPos);
glm::vec3 RaycastCursor(double mouse_x, double mouse_y, int width, int height, glm::mat4 projection_matrix, glm::mat4 view_matrix);
glm::vec3 calculate_player_local_center(const Model& model);
void calculate_model_local_center(const Model& model, glm::vec3& outMin, glm::vec3& outMax);

int main() {
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
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
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

    glEnable(GL_DEPTH_TEST); // enable the z-buffer

    Shader ourShader("shaders/sample.vs", "shaders/sample.fs");

    Model MarbelFloor("assets/floor-marble-tiled-floor/source/FloorTiledMarble.fbx");
    if (!MarbelFloor.meshes.empty()) {
        std::cout << "MarbelFloor mesh[0] vertex count: "
                  << MarbelFloor.meshes[0].vertices.size() << std::endl;
        std::cout << MarbelFloor.meshes.size() << std::endl;
    } else {
        std::cout << "MarbelFloor has no meshes loaded." << std::endl;
    }

    Model Player("assets/PlayerTemplate/source/objects/character.obj");
    if (!Player.meshes.empty()) {
        std::cout << "Player mesh[0] vertex count: "
                  << Player.meshes[0].vertices.size() << std::endl;
        std::cout << Player.meshes.size() << std::endl;
    } else {
        std::cout << "Player has no meshes loaded." << std::endl;
    }

    Model Cube("assets/cube/source/cube.obj");
    if (!Cube.meshes.empty()) {
        std::cout << "Cube mesh[0] vertex count: "
                  << Cube.meshes[0].vertices.size() << std::endl;
        std::cout << Cube.meshes.size() << std::endl;
    } else {
        std::cout << "Cube has no meshes loaded." << std::endl;
    }

    glm::vec3 playerLocalCenter = calculate_player_local_center(Player);
    glm::vec3 playerWorldTarget(0.0f, 0.0f, 0.0f);

    // Cube Properties
    float radius = 1.0f;
    float y_translation = -1;
    float offset_z = -10; // only for rendered object not the collision
    glm::vec3 world_translation = glm::vec3(27.2f, -21.66f, 25.76f);
    bool should_rotate = true;
    glm::vec3 scale = scaled_tiles;
    

    BoundingBox cubeBounds = model_boundaries(Cube, radius, y_translation, world_translation, should_rotate, scale, offset_z);
    std::vector<BoundingBox> colliders = { cubeBounds };

    
    int i = 0;
    

    double xpos, ypos; // cursor position
    int width, height; // frame buffer size
    while (!glfwWindowShouldClose(window)) {
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &width, &height);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, playerLocalCenter, colliders);

        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::vec3 playerPivotWorld = playerPos + playerLocalCenter;
        playerWorldTarget = playerPivotWorld;


        cameraPos = glm::vec3(playerPos.x + cameraDistanceX, playerPos.y + cameraDistanceY, playerPos.z + cameraDistanceZ);
        glm::mat4 view = camera.GetViewMatrix(cameraPos, playerWorldTarget, fixCamera);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


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

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, -4, 0.0));
        model = glm::translate(model, glm::vec3(37.2f, -21.66f, 25.76f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::scale(model, scaled_tiles);
        ourShader.setMat4("model", model);
        MarbelFloor.Draw(ourShader);



        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, -4, 0.0));
        model = glm::translate(model, glm::vec3(27.2f, -21.66f, 25.76f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::scale(model, scaled_tiles);
        ourShader.setMat4("model", model);
        MarbelFloor.Draw(ourShader);

        ourShader.setMat4("model", cubeBounds.transformation);
        Cube.Draw(ourShader);

        if (i % 50 == 0)
        {
        LogVec(playerWorldTarget, "Player World ");
        LogVec(playerLocalCenter, "Local Player Center");
        LogVec(playerPos, "Player Position");
        LogVec(plane_point, "Point of intersection");
        LogVec(ray_world, "ray_world");
        LogVec(plane_point, "plane_point");
        LogVec(cubeBounds.modelMin, "cubeBounds.modelMin");
        LogVec(cubeBounds.modelMax, "cubeBounds.modelMax");
        std::cout << "cubeBounds.radius " << cubeBounds.radius << std::endl;
        }
        i += 1;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, glm::vec3 playerLocalCenter, const std::vector<BoundingBox>& colliders) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime, playerPos, playerLocalCenter, colliders);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime, playerPos, playerLocalCenter, colliders);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime, playerPos, playerLocalCenter, colliders);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime, playerPos, playerLocalCenter, colliders);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Cursor position relative to top-left corner of the window content area:t
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{

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

    camera.ProcessMouseMovement(xoffset, yoffset);
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
    std::cout << "Collision compare -> playerPos: ("
              << playerPos.x << ", " << playerPos.y << ", " << playerPos.z
              << ") | boxMin: ("
              << cubeMin.x << ", " << cubeMin.y << ", " << cubeMin.z
              << ") | boxMax: ("
              << cubeMax.x << ", " << cubeMax.y << ", " << cubeMax.z
              << ")" << std::endl;

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

BoundingBox model_boundaries(Model Cube, float radius, float y_translation, glm::vec3 world_translation, bool is_rotation, glm::vec3 scale, float z_offset)
{
    glm::vec3 localMin, localMax;
    calculate_model_local_center(Cube, localMin, localMax);
    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, glm::vec3(0.0, y_translation, 0.0f));
    cubeModel = glm::translate(cubeModel, world_translation);
    glm::mat4 cubeModelOffset = glm::translate(cubeModel, glm::vec3(0.0f, 0.0f, z_offset));
    if (is_rotation)
    {
        cubeModel = glm::rotate(cubeModel, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        cubeModelOffset = glm::rotate(cubeModelOffset, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    }
    cubeModel = glm::scale(cubeModel, scale);
    cubeModelOffset = glm::scale(cubeModelOffset, scale);
   

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

    glm::vec3 cubeMin(worldMin.x - radius, worldMin.y, worldMin.z - radius);
    glm::vec3 cubeMax(worldMax.x + radius, worldMax.y, worldMax.z + radius);
    return BoundingBox{cubeMin, cubeMax, radius, cubeModelOffset};
}