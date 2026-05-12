#ifndef GRAPHICS_CLASS_H
#define GRAPHICS_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class GlfwSetup
{
    public:
        GlfwSetup(float window_width, float window_height, std::string game_title);

        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
        void configure_callbacks();
        void configure_cursor();
        void configure_glad();

        GLFWwindow* get_window() const;
        float get_window_width();
        float get_window_height();
        
        void compute_fps();

        float get_delta_time();

        void update_window_width_height();
        void activate();
        void delete_class();
    
    private:
        float window_width;
        float window_height;
        std::string game_title;
        GLFWwindow* window;
        GLFWcursor* cursor;
        void window_setup();

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        unsigned int counter = 0;
};

#endif