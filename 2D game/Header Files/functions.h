#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <glm/glm.hpp>
#include <vector>



struct ObjectProperties {
	float radius;
	float y_translation;
	float z_offset;
	glm::vec3 world_translation;
	bool should_rotate;
	glm::vec3 scale;
};

bool checkCollision(glm::vec3 playerPos, glm::vec3 cubeMin, glm::vec3 cubeMax);

#endif