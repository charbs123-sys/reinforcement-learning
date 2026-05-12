#include "../Header Files/shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{

    ShaderStrings shaders_as_strings = convert_shader_to_string(vertexPath, fragmentPath);

    const char* vShaderCode = shaders_as_strings.vertexShaderString.c_str(); // returns a pointer to the strings array
    const char* fShaderCode = shaders_as_strings.fragmentShaderString.c_str();

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER); // Initialize a shader of vertex type
    glShaderSource(vertex, 1, &vShaderCode, NULL); // attaches source code to vertex shader, 1 means we are using one string for the whole shader
    glCompileShader(vertex); // compiles the shader since program doesn't understand the shadre otherwise

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    //Create the program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    // Clean up
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

ShaderStrings Shader::convert_shader_to_string(const char* vertexPath, const char* fragmentPath)
{
    ShaderStrings shader_strings;

    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile; // read the file content
    std::stringstream vShaderStream, fShaderStream; // store the buffer content

    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        vShaderStream << vShaderFile.rdbuf(); // returns a pointer to stream buffer object
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        shader_strings.vertexShaderString = vShaderStream.str();
        shader_strings.fragmentShaderString = fShaderStream.str();

    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    return shader_strings;
}