#include "../Header Files/object_helpers.h"
#include "../json.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

LoadMap::LoadMap(const std::string& relative_path_to_map_json, glm::vec3 starting_pos, float tile_world_width, float tile_world_height)
{
    this->tile_world_width = tile_world_width;
    this->tile_world_height = tile_world_height;
    map_tile_coordinates.clear();
    map_tiles.clear();

    const std::filesystem::path map_path(relative_path_to_map_json);
    const std::filesystem::path map_parent = map_path.parent_path();

    std::ifstream file(relative_path_to_map_json);
    json data = json::parse(file, nullptr, false);
    load_sprite_tilesets_from_map(data, map_parent);

    const glm::vec3 origin = starting_pos;

    for (const auto& layer : data["layers"])
    {

        int width = 0;
        width = layer["width"].get<int>();

        const auto& tile_data = layer["data"];
        for (std::size_t i = 0; i < tile_data.size(); ++i)
        {
            const int tile_id = tile_data[i].get<int>();
            if (tile_id == 0)
                continue;

            const int row = static_cast<int>(i / static_cast<std::size_t>(width));
            const int col = static_cast<int>(i % static_cast<std::size_t>(width));

            glm::vec3 tile_position = origin;
            tile_position.x += static_cast<float>(col) * this->tile_world_width;
            tile_position.y -= static_cast<float>(row) * this->tile_world_height;

            map_tile_coordinates.push_back(tile_position);

            MapTileInstance tile;
            tile.world_position = tile_position;
            tile.gid = tile_id;
            map_tiles.push_back(tile);
        }
    }
}

void LoadMap::load_sprite_tilesets_from_map(const nlohmann::json& parsed_map, const std::filesystem::path& map_parent)
{
    sprites_for_map.clear();
    sprites_for_map.reserve(parsed_map["tilesets"].size());

    for (const auto& tileset : parsed_map["tilesets"])
    {
        if (!tileset.contains("image") || !tileset["image"].is_string())
            continue;

        std::filesystem::path sprite_png_path = (map_parent / tileset["image"].get<std::string>()).lexically_normal();
        std::filesystem::path sprite_json_path = sprite_png_path;
        sprite_json_path.replace_extension(".json");

        SpriteSheet new_sprite(sprite_png_path.string(), sprite_json_path.string());

        SpriteGid tileset_sprite;

        tileset_sprite.sprites_for_map = new_sprite;
        tileset_sprite.gid = tileset.value("firstgid", 0);
        tileset_sprite.first_gid = tileset.value("firstgid", 0);
        tileset_sprite.last_gid = tileset.value("firstgid", 0) + std::max(1, tileset.value("tilecount", 1)) - 1;
        tileset_sprite.tile_width = tileset.value("tilewidth", 0);
        tileset_sprite.columns = tileset.value("columns", 0);
        tileset_sprite.tile_height = tileset.value("tileheight", 0);
        
        sprites_for_map.push_back(tileset_sprite);
    }
}

void LoadMap::draw_map(Shader& shader)
{
    for (const auto& tile : map_tiles)
    {
        SpriteSheet* sprite_to_draw = find_sprite_using_gid(tile.gid);
        if (sprite_to_draw == nullptr)
            continue;

        sprite_to_draw->set_world_center(tile.world_position);
        sprite_to_draw->Draw(shader);
    }
}

SpriteSheet* LoadMap::find_sprite_using_gid(int gid)
{
    for (auto& sprite : sprites_for_map)
    {
        if (gid >= sprite.first_gid && gid <= sprite.last_gid)
        {
            return &sprite.sprites_for_map;
        }
    }

    return nullptr;
}

void LoadMap::increment_sprites_for_map_with_duration(float delta_time)
{
    for (auto& sprite : sprites_for_map)
    {
        sprite.sprites_for_map.increment_current_frame_using_duration(delta_time);
    }
}