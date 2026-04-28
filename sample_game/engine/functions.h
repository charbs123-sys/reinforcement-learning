#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <glm/glm.hpp>
#include <vector>

struct BoundingBox {
	glm::vec3 modelMin;
	glm::vec3 modelMax;
	float radius;
	glm::mat4 transformation;
};

struct ObjectProperties {
	float radius;
	float y_translation;
	float z_offset;
	glm::vec3 world_translation;
	bool should_rotate;
	glm::vec3 scale;
};

struct EnemyProperties {
	glm::vec3 enemyLocalCenter;
	glm::vec3 enemyPos;
};

struct BulletProperties {
	glm::vec3 bulletPos;
	glm::vec3 bulletDir;
	glm::mat4 bulletModel;
};

bool checkCollision(glm::vec3 playerPos, glm::vec3 cubeMin, glm::vec3 cubeMax);

#endif