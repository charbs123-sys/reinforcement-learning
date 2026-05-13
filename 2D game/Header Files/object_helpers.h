#ifndef OBJECT_HELPERS
#define OBJECT_HELPERS

#include <string>
#include <vector>
#include <filesystem>
#include <glm/glm.hpp>
#include "model.h"
#include "functions.h"
#include "../json.hpp"
#include "sprite_sheet.h"

using json = nlohmann::json;

struct MapTileInstance
{
    glm::vec3 world_position{0.0f};
    int gid = 0;
};

struct SpriteGid
{
    SpriteSheet sprites_for_map;
    int gid = 0;

    int first_gid = 0;
    int last_gid = 0;
    int tile_width = 0;
    int tile_height = 0;
    int columns = 0;
};

class LoadMap
{
    public:
        LoadMap(const std::string& relative_path_to_map_json, glm::vec3 starting_pos, float tile_world_width, float tile_world_height);
        std::vector<glm::vec3> map_tile_coordinates;
        std::vector<MapTileInstance> map_tiles;
        std::vector<SpriteGid> sprites_for_map;
        void draw_map(Shader& shader);
        void increment_sprites_for_map_with_duration(float delta_time);
    private:
        float tile_world_width;
        float tile_world_height;
        void load_sprite_tilesets_from_map(const nlohmann::json& parsed_map, const std::filesystem::path& map_parent);
        SpriteSheet* find_sprite_using_gid(int gid);
};

#endif