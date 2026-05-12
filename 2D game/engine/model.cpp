#include "../Header Files/model.h"

namespace
{
    bool is_absolute_path(const std::string& path)
    {
        if (path.empty())
            return false;

        // Windows drive path: C:/... or C:\...
        if (path.size() > 1 && path[1] == ':')
            return true;

        // UNC path: \\server\share
        if (path.size() > 1 && path[0] == '\\' && path[1] == '\\')
            return true;

        // Unix absolute path: /...
        if (path[0] == '/')
            return true;

        return false;
    }

    std::string join_path(const std::string& directory, const std::string& relative_path)
    {
        if (directory.empty() || directory == ".")
            return relative_path;

        if (directory.back() == '/' || directory.back() == '\\')
            return directory + relative_path;

        return directory + '/' + relative_path;
    }
}


void Model::Draw(Shader &shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}


Model::Model(string const &path, bool gamma) : gammaCorrection(gamma)
{
    loadModel(path);
}

void Model::loadModel(string const &path)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    // Read the file and triangulate to ensure the model consists only of triangles
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }
    // retrieve the directory containing the file
    const size_t last_separator = path.find_last_of("/\\");
    if (last_separator == string::npos)
        directory = ".";
    else
        directory = path.substr(0, last_separator);

    // process ASSIMP's root node recursively
    process_nodes_into_meshes(scene->mRootNode, scene);
}


void Model::process_nodes_into_meshes(aiNode *node, const aiScene *scene)
{
    // process each mesh located at the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(populate_vertex_index_textures(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_nodes_into_meshes(node->mChildren[i], scene);
    }

}

Mesh Model::populate_vertex_index_textures(aiMesh *mesh, const aiScene *scene)
{
    vector<Vertex> vertices;
    
    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 base_vector_pos;
        // positions
        base_vector_pos = assigning_vector_positions(mesh->mVertices, i);
        vertex.Position = base_vector_pos;

        // normals
        if (mesh->HasNormals())
        {
            base_vector_pos = assigning_vector_positions(mesh->mNormals, i);
            vertex.Normal = base_vector_pos;
        }

        // texture coordinates
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            base_vector_pos = assigning_vector_positions(mesh->mTangents, i);
            vertex.Tangent = base_vector_pos;
            // bitangent
            base_vector_pos = assigning_vector_positions(mesh->mBitangents, i);
            vertex.Bitangent = base_vector_pos;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    vector<unsigned int> index_mapping_vertex_data;
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            index_mapping_vertex_data.push_back(face.mIndices[j]);        
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    

    vector<Texture> textures;
    // 1. diffuse maps
    vector<Texture> diffuseMaps = populate_texture_properties(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture> specularMaps = populate_texture_properties(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = populate_texture_properties(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = populate_texture_properties(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    
    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, index_mapping_vertex_data, textures);
}


vector<Texture> Model::populate_texture_properties(aiMaterial *mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = create_texture(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
    }
    return textures;
}

glm::vec3 Model::assigning_vector_positions(aiVector3D *vertices, unsigned int i)
{
    glm::vec3 vector_pos;
    vector_pos.x = vertices[i].x;
    vector_pos.y = vertices[i].y;
    vector_pos.z = vertices[i].z;
    
    return vector_pos;
}

unsigned int create_texture(const char *path, const string &directory, bool gamma)
{
    const string requested_path = path ? string(path) : string();
    string filename = requested_path;

    // Keep absolute texture paths as-is; resolve relative paths against the model directory.
    if (!is_absolute_path(filename))
        filename = join_path(directory, filename);

    unsigned int textureID;
    glGenTextures(1, &textureID); // creating the texture

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glGenerateMipmap(GL_TEXTURE_2D); // generates mip maps for the texture -> smaller resolutions for when the texture is further away

        // s, t, r -> treated as x, y and z axis -> we implement GL_REPEAT or any other rendition to these specific axes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // basically says how we want to treat scaled images
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << requested_path << std::endl;
        std::cout << "Texture failed to load at filename: " << filename << std::endl;
        std::cout << "Texture failed to load at directory: " << directory << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


void Model::compute_and_set_local_center_min_max_coords()
{
    bool hasVertex = false;

    for (const auto& mesh : meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            hasVertex = true;
            local_min_coords.x = std::min(local_min_coords.x, vertex.Position.x);
            local_min_coords.y = std::min(local_min_coords.y, vertex.Position.y);
            local_min_coords.z = std::min(local_min_coords.z, vertex.Position.z);
            local_max_coords.x = std::max(local_max_coords.x, vertex.Position.x);
            local_max_coords.y = std::max(local_max_coords.y, vertex.Position.y);
            local_max_coords.z = std::max(local_max_coords.z, vertex.Position.z);
        }
    }

    if (!hasVertex)
    {
        this->object_local_center = glm::vec3(0.0f);
        return;
    }

    this->object_local_center = (local_min_coords + local_max_coords) * 0.5f;
}

void Model::set_world_center(glm::vec3 object_world_position)
{
    this->object_world_center = object_world_position;
}

void Model::compute_and_set_model_bounds()
{
    model_min_bounds = glm::vec3(world_min_coords.x - radius, world_min_coords.y, world_min_coords.z - radius);
    model_max_bounds = glm::vec3(world_max_coords.x + radius, world_min_coords.y, world_max_coords.z + radius);
}

void Model::compute_and_set_local_corners()
{
    this->local_corners = {
        glm::vec3(local_min_coords.x, local_min_coords.y, local_min_coords.z),
        glm::vec3(local_max_coords.x, local_min_coords.y, local_min_coords.z),
        glm::vec3(local_min_coords.x, local_max_coords.y, local_min_coords.z),
        glm::vec3(local_max_coords.x, local_max_coords.y, local_min_coords.z),
        glm::vec3(local_min_coords.x, local_min_coords.y, local_max_coords.z),
        glm::vec3(local_max_coords.x, local_min_coords.y, local_max_coords.z),
        glm::vec3(local_min_coords.x, local_max_coords.y, local_max_coords.z),
        glm::vec3(local_max_coords.x, local_max_coords.y, local_max_coords.z)
    };
}

void Model::compute_and_set_world_min_and_max(glm::mat4 model_matrix)
{
    for (const auto& local_corner : local_corners)
    {
        glm::vec3 world_corner = glm::vec3(model_matrix * glm::vec4(local_corner, 1.0f));
        world_min_coords.x = std::min(world_min_coords.x, world_corner.x);
        world_min_coords.y = std::min(world_min_coords.y, world_corner.y);
        world_min_coords.z = std::min(world_min_coords.z, world_corner.z);
        world_max_coords.x = std::max(world_max_coords.x, world_corner.x);
        world_max_coords.y = std::max(world_max_coords.y, world_corner.y);
        world_max_coords.z = std::max(world_max_coords.z, world_corner.z);
    }
}