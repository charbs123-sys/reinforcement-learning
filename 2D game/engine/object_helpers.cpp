#include "../Header Files/object_helpers.h"

// void calculate_model_local_center(const Model& model, glm::vec3& outMin, glm::vec3& outMax)
// {
//     outMin = glm::vec3(std::numeric_limits<float>::max());
//     outMax = glm::vec3(std::numeric_limits<float>::lowest());
//     bool hasVertex = false;

//     for (const auto& mesh : model.meshes)
//     {
//         for (const auto& vertex : mesh.vertices)
//         {
//             hasVertex = true;
//             outMin.x = std::min(outMin.x, vertex.Position.x);
//             outMin.y = std::min(outMin.y, vertex.Position.y);
//             outMin.z = std::min(outMin.z, vertex.Position.z);
//             outMax.x = std::max(outMax.x, vertex.Position.x);
//             outMax.y = std::max(outMax.y, vertex.Position.y);
//             outMax.z = std::max(outMax.z, vertex.Position.z);
//         }
//     }

//     if (!hasVertex)
//     {
//         outMin = glm::vec3(0.0f);
//         outMax = glm::vec3(0.0f);
//     }
// }


// BoundingBox model_boundaries(Model Cube, ObjectProperties obj)
// {
//     glm::vec3 localMin, localMax;
//     calculate_model_local_center(Cube, localMin, localMax);
//     glm::mat4 cubeModel = glm::mat4(1.0f);
//     cubeModel = glm::translate(cubeModel, glm::vec3(0.0, obj.y_translation, 0.0f));
//     cubeModel = glm::translate(cubeModel, obj.world_translation);
//     glm::mat4 cubeModelOffset = glm::translate(cubeModel, glm::vec3(0.0f, 0.0f, obj.z_offset));
//     if (obj.should_rotate)
//     {
//         cubeModel = glm::rotate(cubeModel, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
//         cubeModelOffset = glm::rotate(cubeModelOffset, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
//     }
//     cubeModel = glm::scale(cubeModel, obj.scale);
//     cubeModelOffset = glm::scale(cubeModelOffset, obj.scale);
   

//     glm::vec3 localCorners[8] = {
//         glm::vec3(localMin.x, localMin.y, localMin.z),
//         glm::vec3(localMax.x, localMin.y, localMin.z),
//         glm::vec3(localMin.x, localMax.y, localMin.z),
//         glm::vec3(localMax.x, localMax.y, localMin.z),
//         glm::vec3(localMin.x, localMin.y, localMax.z),
//         glm::vec3(localMax.x, localMin.y, localMax.z),
//         glm::vec3(localMin.x, localMax.y, localMax.z),
//         glm::vec3(localMax.x, localMax.y, localMax.z)
//     };

//     glm::vec3 worldMin(std::numeric_limits<float>::max());
//     glm::vec3 worldMax(std::numeric_limits<float>::lowest());

//     for (const glm::vec3& localCorner : localCorners)
//     {
//         glm::vec3 worldCorner = glm::vec3(cubeModel * glm::vec4(localCorner, 1.0f));
//         worldMin.x = std::min(worldMin.x, worldCorner.x);
//         worldMin.y = std::min(worldMin.y, worldCorner.y);
//         worldMin.z = std::min(worldMin.z, worldCorner.z);
//         worldMax.x = std::max(worldMax.x, worldCorner.x);
//         worldMax.y = std::max(worldMax.y, worldCorner.y);
//         worldMax.z = std::max(worldMax.z, worldCorner.z);
//     }

//     glm::vec3 cubeMin(worldMin.x - obj.radius, worldMin.y, worldMin.z - obj.radius);
//     glm::vec3 cubeMax(worldMax.x + obj.radius, worldMax.y, worldMax.z + obj.radius);
//     return BoundingBox{cubeMin, cubeMax, obj.radius, cubeModelOffset};
// }

// glm::mat4 model_matrices_impose(ObjectProperties obj)
// {
//     glm::mat4 model = glm::mat4(1.0f);
//     model = glm::translate(model, glm::vec3(0.0, obj.y_translation, 0.0));
//     model = glm::translate(model, obj.world_translation);
//     if (obj.should_rotate)
//     {
//         model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
//     }
//     model = glm::scale(model, obj.scale);
//     return model;
// }
