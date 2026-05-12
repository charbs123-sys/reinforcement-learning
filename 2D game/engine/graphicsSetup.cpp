#include "../Header Files/graphicsSetup.h"
#include <iostream>

GlfwSetup::GlfwSetup(float window_width, float window_height, std::string game_title)
{
    this->window_width = window_width;
    this->window_height = window_height;
    this->game_title = game_title;

    this->window_setup();
}

void GlfwSetup::window_setup()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->window = glfwCreateWindow(this->window_width, this->window_height, "game", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create window" << std::endl;
        std::cout.flush();
        this->delete_class();
        return;
    }

    glfwMakeContextCurrent(window);
}

void GlfwSetup::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void GlfwSetup::configure_callbacks()
{
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}


void GlfwSetup::configure_cursor()
{
    unsigned char pixels[16 * 16 * 4];
    memset(pixels, 0xff, sizeof(pixels));
    
    GLFWimage image;
    image.width = 16;
    image.height = 16;
    image.pixels = pixels;
    
    this->cursor = glfwCreateCursor(&image, 0, 0);

    if(!cursor)
    {
        std::cout << "Failed to add texture to cursor" << std::endl;
        return;
    }
    glfwSetCursor(window, this->cursor);

}

void GlfwSetup::configure_glad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::cout.flush();
        this->delete_class();
    }
}

void GlfwSetup::compute_fps()
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = (currentFrame - lastFrame);
    lastFrame = currentFrame;
    counter++;
    if (deltaTime >= 1.0 / 30.0)
    {
        std::string FPS = std::to_string((1.0 / deltaTime) * counter);
        std::string newTitle = "FPS- " + FPS;
        glfwSetWindowTitle(get_window(), newTitle.c_str());
        lastFrame = currentFrame;
        counter = 0;
    }
}

GLFWwindow* GlfwSetup::get_window() const
{
    return window;
}

float GlfwSetup::get_window_width()
{
    return window_width;
}

float GlfwSetup::get_window_height()
{
    return window_height;
}

float GlfwSetup::get_delta_time()
{
    return deltaTime;
}

void GlfwSetup::update_window_width_height()
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    this->window_height = height;
    this->window_width = width;
}

void GlfwSetup::delete_class()
{
    glfwTerminate();
}

