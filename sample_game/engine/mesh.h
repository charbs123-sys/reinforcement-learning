#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <vector>
using namespace std;

// maximum number of bones (joints) affecting a single vertex
// basically limits how complex the mesh should be
#define MAX_BONE_INFLUENCE 4


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

// Helper function to create or get a white default texture
static unsigned int getDefaultWhiteTexture()
{
    static unsigned int whiteTexture = 0;
    if (whiteTexture == 0)
    {
        glGenTextures(1, &whiteTexture);
        unsigned char white[] = {255, 255, 255, 255}; // RGBA white
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    return whiteTexture;
}

class Mesh {
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        /*
        Basically, take self.vertices and set vertices
        */
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }

    void Draw(Shader &shader)
    {
        /*
        This code draws the shader
        */
        
        unsigned int diffuseNr = 1, specularNr = 1, normalNr = 1, heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            // activates any one of GL_TEXTUREi depending on i
            glActiveTexture(GL_TEXTURE0 + i); 
            string number;
            string name = textures[i].type;
            // Detech the type of texture in index i and count the number of that particular texture
            // Detects whether it is a diffuse, specular or other texture type
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);
            else if(name == "texture_normal")
                number = std::to_string(normalNr++);
             else if(name == "texture_height")
                number = std::to_string(heightNr++);

            // get the location of for example texture_normali and then set its value based on location
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // bind the texture to the location
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // If mesh has no textures, bind a white default texture to prevent texture state leakage
        if (textures.size() == 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, getDefaultWhiteTexture());
            glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse1"), 0);
        }
    
        //Drawing the mesh
        glBindVertexArray(VAO); // make active object for subsequent vertex operations
        // Draws onto the currently bound VAO
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int> (indices.size()), GL_UNSIGNED_INT, 0);
        
        // Go back to default
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }
private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO); // we use the VAO so opengl can find the data in the VBO - stores pointers to one or more VBO objects
        glGenBuffers(1, &VBO); // 1 means 1 3D object
        glGenBuffers(1, &EBO); // (index buffer) - Tells us the order to pass over the vertices -> helps us only store a reference to a vertex one time 

        glBindVertexArray(VAO); // Bind so that subsequent vector operations performed on VAO
        
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO); // binding makes the VBO the current object
        // Store the vertices into the VBO
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);
        /*
        Stream - vertices modified once and used a few once
        Statics - vertices modified once and used many time
        Dynamic - vertices modified multiple time and used many times

        Draw - vertices will be modified and used to draw an image on the screen
        read - 
        copy - 
        */

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

        // set the vertex attribute pointers - allows us to interact with vertex shader outside the script
        // vertex Positions
        glEnableVertexAttribArray(0); // enables vertix attribute at index 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
		// ids
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

		// weights
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};

#endif