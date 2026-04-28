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


inline std::vector<EnemyProperties> spawnEnemy(int number_of_enemies, glm::vec3 playerLocalCenter, glm::vec3 playerPos, float min_x_radius, float max_x_radius, float min_z_radius, float max_z_radius, std::mt19937 rng)
{

    std::vector<EnemyProperties> enemies;
    for (int i = 0; i < number_of_enemies; i++)
    {
        static std::uniform_real_distribution<float> dist_x(min_x_radius, max_x_radius);
        float x_radius = dist_x(rng);
        static std::uniform_real_distribution<float> dist_z(min_z_radius, max_z_radius);
        float z_radius = dist_z(rng);
        
        float sign_x = (rand() % 2 == 0) ? 1.0f : -1.0f;
        float sign_z = (rand() % 2 == 0) ? 1.0f : -1.0f;

        glm::vec3 enemyPos = playerPos;
        enemyPos.x += sign_x * x_radius;
        enemyPos.z += sign_z * z_radius;

        enemies.push_back(EnemyProperties{
            .enemyLocalCenter = playerLocalCenter,
            .enemyPos = enemyPos 
        });
    }

    return enemies;
} 