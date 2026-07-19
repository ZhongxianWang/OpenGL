#include "Mesh.h"
#include <glad/glad.h> 
#include <iostream>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Material material)
{
    m_vertices = vertices;
    m_indices = indices;
    m_textures = textures;
    m_material = material;
    setupMesh();
}

Mesh::~Mesh()
{
}

void Mesh::draw(Shader& shader)
{
    // 绑定纹理
    unsigned int diffuseNr  = 0;
    unsigned int specularNr = 0;
    unsigned int normalNr   = 0;
    unsigned int heightNr   = 0;
    for(unsigned int i = 0; i < m_textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string name = m_textures[i].type;
        std::string uniformName;
        
        if(name == "texture_diffuse") {
            uniformName = "texture_diffuse[" + std::to_string(diffuseNr++) + "]";
        } else if(name == "texture_specular") {
            uniformName = "texture_specular[" + std::to_string(specularNr++) + "]";
        } else if(name == "texture_normal") {
            uniformName = "texture_normal[" + std::to_string(normalNr++) + "]";
        } else if(name == "texture_height") {
            uniformName = "texture_height[" + std::to_string(heightNr++) + "]";
        }

        shader.setInt(uniformName, i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
    }
    
    shader.setVec3("ambient", m_material.ambient);
    shader.setVec3("diffuse", m_material.diffuse);
    shader.setVec3("specular", m_material.specular);
    shader.setFloat("shininess", m_material.shininess);

    // 绘制
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh()
{
    // 创建 VBO, EBO, VAO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

    // 设置顶点属性指针
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}