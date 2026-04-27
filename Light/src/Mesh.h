#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
};

struct Material {
    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float shininess = 64.0f;
};

class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Material material);
    ~Mesh();
    void draw(Shader& shader);

private:
    void setupMesh();

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Texture> m_textures;
    Material m_material;

    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;

};