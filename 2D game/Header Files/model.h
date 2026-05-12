#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <array>
using namespace std;


unsigned int create_texture(const char *path, const string &directory, bool gamma = false);


class Model
{
public:
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    
    float radius;
    glm::vec3 local_min_coords, local_max_coords;
    array<glm::vec3, 8> local_corners;
    glm::vec3 world_min_coords, world_max_coords;
    glm::vec3 model_min_bounds, model_max_bounds;


    
    Model(string const &path, bool gamma = false);
    void Draw(Shader &shader);

    void set_world_center(glm::vec3 object_world_position);
    void compute_and_set_local_center_min_max_coords();
    void compute_and_set_local_corners();
    void compute_and_set_world_min_and_max(glm::mat4 model_matrix);
    void compute_and_set_model_bounds();
    
    glm::vec3 object_local_center;
    glm::vec3 object_world_center;

private:
    void loadModel(string const &path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void process_nodes_into_meshes(aiNode *node, const aiScene *scene);
    Mesh populate_vertex_index_textures(aiMesh *mesh, const aiScene *scene);

    glm::vec3 assigning_vector_positions(aiVector3D *vertices, unsigned int i);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> populate_texture_properties(aiMaterial *mat, aiTextureType type, string typeName);

    
};

#endif