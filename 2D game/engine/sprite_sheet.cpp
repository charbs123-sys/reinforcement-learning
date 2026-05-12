#include "../Header Files/sprite_sheet.h"
#include "../stb_image.h"
#include "../json.hpp"

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <limits>
#include <algorithm>


using json = nlohmann::json;

SpriteSheet::SpriteSheet(const std::string& relative_path_to_sprite_sheet, const std::string& relative_path_to_json)
{
    this->relative_path_to_sheet = relative_path_to_sprite_sheet;
    this->relative_path_to_json = relative_path_to_json;
    load_png_file_bind_texture();
    load_json_data_populate_sprites();
    setup_sprite_quad();

    if (!sprite_sheets_data.empty())
        update_uvs_for_frame(sprite_sheets_data[0]);
}

void SpriteSheet::load_png_file_bind_texture()
{
    glGenTextures(1, &texture_id);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(relative_path_to_sheet.c_str(), &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glGenerateMipmap(GL_TEXTURE_2D); // generates mip maps for the texture -> smaller resolutions for when the texture is further away

        // s, t, r -> treated as x, y and z axis -> we implement GL_REPEAT or any other rendition to these specific axes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // basically says how we want to treat scaled images
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        sprite_sheet_width = static_cast<float>(width);
        sprite_sheet_height = static_cast<float>(height);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << relative_path_to_sheet << std::endl;
    }
    stbi_image_free(data);
}

void SpriteSheet::load_json_data_populate_sprites()
{
    std::ifstream file(relative_path_to_json);
    std::cout << "Loading JSON from: " << relative_path_to_json << std::endl;
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

    if (!file.is_open())
    {
        std::cout << "Failed to open JSON file at path: " << relative_path_to_json << std::endl;
        return;
    }

    json data = json::parse(file, nullptr, false);
    if (data.is_discarded())
    {
        std::cout << "Failed to parse JSON file: " << relative_path_to_json << std::endl;
        return;
    }

    if (!data.contains("meta") || !data["meta"].contains("size") || !data.contains("frames"))
    {
        std::cout << "JSON missing required keys (meta.size / frames): " << relative_path_to_json << std::endl;
        return;
    }

    sprite_sheet_height = data["meta"]["size"]["h"];
    sprite_sheet_width = data["meta"]["size"]["w"];

    number_of_animations = 0;
    sprite_sheets_data.clear();

    const json& frames = data["frames"];
    if (frames.is_object())
    {
        for (const auto& [frame_name, frame_data] : frames.items())
        {
            SpriteSheetFrames single_frame_sprite_sheet;

            single_frame_sprite_sheet.frame.x_coord = frame_data["frame"]["x"];
            single_frame_sprite_sheet.frame.y_coord = frame_data["frame"]["y"];
            single_frame_sprite_sheet.frame.width_of_frame = frame_data["frame"]["w"];
            single_frame_sprite_sheet.frame.height_of_frame = frame_data["frame"]["h"];
            single_frame_sprite_sheet.duration = frame_data.value("duration", 0.0f);

            number_of_animations += 1;
            sprite_sheets_data.push_back(single_frame_sprite_sheet);
        }
    }
    else if (frames.is_array())
    {
        for (const auto& frame_data : frames)
        {
            SpriteSheetFrames single_frame_sprite_sheet;

            single_frame_sprite_sheet.frame.x_coord = frame_data["frame"]["x"];
            single_frame_sprite_sheet.frame.y_coord = frame_data["frame"]["y"];
            single_frame_sprite_sheet.frame.width_of_frame = frame_data["frame"]["w"];
            single_frame_sprite_sheet.frame.height_of_frame = frame_data["frame"]["h"];
            single_frame_sprite_sheet.duration = frame_data.value("duration", 0.0f);

            number_of_animations += 1;
            sprite_sheets_data.push_back(single_frame_sprite_sheet);
        }
    }
    else
    {
        std::cout << "Unsupported frames format in JSON: " << relative_path_to_json << std::endl;
        return;
    }

    std::cout << "Loaded " << number_of_animations << " sprite frames from JSON." << std::endl;
}

void SpriteSheet::setup_sprite_quad()
{
    // Layout matches vertex shader: position(3) + normal(3) + texcoords(2)
    // Normal is (0,0,1) for a front-facing quad.
    const float vertices[] = {
        // positions          // normal          // uv
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f
    };

    const unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // location 0: position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // location 1: normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // location 2: texcoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void SpriteSheet::update_uvs_for_frame(const SpriteSheetFrames& frame)
{
    if (sprite_sheet_width <= 0.0f || sprite_sheet_height <= 0.0f)
        return;

    const float u0 = frame.frame.x_coord / sprite_sheet_width;
    const float u1 = (frame.frame.x_coord + frame.frame.width_of_frame) / sprite_sheet_width;
    const float v0 = frame.frame.y_coord / sprite_sheet_height;
    const float v1 = (frame.frame.y_coord + frame.frame.height_of_frame) / sprite_sheet_height;

    // Layout matches vertex shader: position(3) + normal(3) + texcoords(2)
    const float vertices[] = {
        // positions          // normal          // uv
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  u0, v0,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  u1, v0,
         0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  u1, v1,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  u0, v1
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

void SpriteSheet::Draw()
{
    if (texture_id == 0 || vao == 0 || sprite_sheets_data.empty())
        return;

    const int safe_frame = current_frame % static_cast<int>(sprite_sheets_data.size());
    update_uvs_for_frame(sprite_sheets_data[safe_frame]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SpriteSheet::set_world_center(glm::vec3 object_world_position)
{
    this->object_world_center = object_world_position;
}

void SpriteSheet::compute_and_set_local_center_min_max_coords()
{
    // The sprite quad is generated in local space from (-0.5, -0.5, 0) to (0.5, 0.5, 0).
    local_min_coords = glm::vec3(-0.5f, -0.5f, 0.0f);
    local_max_coords = glm::vec3(0.5f, 0.5f, 0.0f);
    object_local_center = (local_min_coords + local_max_coords) * 0.5f;
}

void SpriteSheet::compute_and_set_local_corners()
{
    this->local_corners = {
        glm::vec3(local_min_coords.x, local_min_coords.y, local_min_coords.z),
        glm::vec3(local_max_coords.x, local_min_coords.y, local_min_coords.z),
        glm::vec3(local_min_coords.x, local_max_coords.y, local_min_coords.z),
        glm::vec3(local_max_coords.x, local_max_coords.y, local_min_coords.z),
        glm::vec3(local_min_coords.x, local_min_coords.y, local_max_coords.z),
        glm::vec3(local_max_coords.x, local_min_coords.y, local_max_coords.z),
        glm::vec3(local_min_coords.x, local_max_coords.y, local_max_coords.z),
        glm::vec3(local_max_coords.x, local_max_coords.y, local_max_coords.z)
    };
}

void SpriteSheet::compute_and_set_world_min_and_max(glm::mat4 model_matrix)
{
    world_min_coords = glm::vec3(std::numeric_limits<float>::max());
    world_max_coords = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& local_corner : local_corners)
    {
        glm::vec3 world_corner = glm::vec3(model_matrix * glm::vec4(local_corner, 1.0f));
        world_min_coords.x = std::min(world_min_coords.x, world_corner.x);
        world_min_coords.y = std::min(world_min_coords.y, world_corner.y);
        world_min_coords.z = std::min(world_min_coords.z, world_corner.z);
        world_max_coords.x = std::max(world_max_coords.x, world_corner.x);
        world_max_coords.y = std::max(world_max_coords.y, world_corner.y);
        world_max_coords.z = std::max(world_max_coords.z, world_corner.z);
    }
}

void SpriteSheet::compute_and_set_model_bounds()
{
    model_min_bounds = glm::vec3(world_min_coords.x - radius, world_min_coords.y, world_min_coords.z - radius);
    model_max_bounds = glm::vec3(world_max_coords.x + radius, world_min_coords.y, world_max_coords.z + radius);
}

void SpriteSheet::increment_current_frame()
{
    if (sprite_sheets_data.empty())
        return;
    current_frame = (current_frame + 1) % static_cast<int>(sprite_sheets_data.size());
}