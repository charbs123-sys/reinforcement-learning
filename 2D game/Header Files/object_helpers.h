#ifndef OBJECT_HELPERS
#define OBJECT_HELPERS

#include <string>
#include <glm/glm.hpp>
#include "model.h"
#include "functions.h"

void calculate_model_local_center(const Model& model, glm::vec3& outMin, glm::vec3& outMax);
// BoundingBox model_boundaries(Model Cube, ObjectProperties obj);
glm::mat4 model_matrices_impose(ObjectProperties obj);

#endif