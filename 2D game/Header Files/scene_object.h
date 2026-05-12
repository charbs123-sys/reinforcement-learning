#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <random>
#include "functions.h"


inline std::vector<ObjectProperties> getSceneObjectsCube(const glm::vec3& scaled_tiles) {
    return {
        // Cube
        ObjectProperties{
            .radius            = 1.0f,
            .y_translation     = -1.0f,
            .z_offset          = -10.0f,
            .world_translation = glm::vec3(27.2f, -21.66f, 25.76f),
            .should_rotate     = true,
            .scale             = scaled_tiles
        },
        // Add more objects here...
        ObjectProperties{
            .radius            = 1.0f,
            .y_translation     = -1.0f,
            .z_offset          = -10.0f,
            .world_translation = glm::vec3(40.2f, -21.66f, 25.76f),
            .should_rotate     = true,
            .scale             = scaled_tiles
        },
    };
}

inline std::vector<ObjectProperties> getSceneObjectsMarbelFloor(const glm::vec3& scaled_tiles)
{
    return {
        // Cube
        ObjectProperties{
            .radius            = 0.0f,
            .y_translation     = -4.0f,
            .z_offset          = 0.0f,
            .world_translation = glm::vec3(37.2f, -21.66f, 25.76f),
            .should_rotate     = true,
            .scale             = scaled_tiles
        },
        // Add more objects here...
        ObjectProperties{
            .radius            = 0.0f,
            .y_translation     = -4.0f,
            .z_offset          = 0.0f,
            .world_translation = glm::vec3(27.2f, -21.66f, 25.76f),
            .should_rotate     = true,
            .scale             = scaled_tiles
        },
    };
} 