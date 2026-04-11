#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <glm/glm.hpp>
#include <vector>

struct BoundingBox {
	glm::vec3 modelMin;
	glm::vec3 modelMax;
	float radius;
};

bool checkCollision(glm::vec3 playerPos, glm::vec3 cubeMin, glm::vec3 cubeMax);

#endif