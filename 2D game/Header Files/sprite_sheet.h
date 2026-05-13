#ifndef SPRITE_SHEET_H
#define SPRITE_SHEET_H

#include <array>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "shader.h"

struct Frame {
    float x_coord;
    float y_coord;
    float width_of_frame;
    float height_of_frame;
};

struct SpriteSheetFrames {
    Frame frame;
    float duration;
};

class SpriteSheet {

    public:
        SpriteSheet(const std::string& relative_path_to_sprite_sheet, const std::string& relative_path_to_json);
        SpriteSheet();

        std::vector<SpriteSheetFrames> sprite_sheets_data;
        void Draw(Shader& shader);

        float radius = 0.0f;
        glm::vec3 local_min_coords{0.0f};
        glm::vec3 local_max_coords{0.0f};
        std::array<glm::vec3, 8> local_corners{};
        glm::vec3 world_min_coords{0.0f};
        glm::vec3 world_max_coords{0.0f};
        glm::vec3 model_min_bounds{0.0f};
        glm::vec3 model_max_bounds{0.0f};
        glm::vec3 object_local_center{0.0f};
        glm::vec3 object_world_center{0.0f};
        glm::mat4 sprite_model_matrix = glm::mat4(1.0f);
        int current_frame = 0;
        

        
        void set_world_center(glm::vec3 object_world_position);
        void compute_and_set_local_center_min_max_coords();
        void compute_and_set_local_corners();
        void compute_and_set_world_min_and_max();
        void compute_and_set_model_bounds();
        void increment_current_frame_using_duration(float delta_time);

    private:
        float sprite_sheet_width;
        float sprite_sheet_height;
        
        int number_of_animations = 0;
        std::string relative_path_to_sheet;
        std::string relative_path_to_json;
        unsigned int texture_id = 0;
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ebo = 0;
        float timer = 0.0f;

        void load_png_file_bind_texture();
        void load_json_data_populate_sprites();
        void setup_sprite_quad();
        void update_uvs_for_frame(const SpriteSheetFrames& frame);
        void compute_and_set_model_matrix();
        
};

#endif