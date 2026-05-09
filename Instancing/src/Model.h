#pragma once

#include "assimp/scene.h"
#include <vector>
#include <string>
#include <map>
#include "Mesh.h"

class aiNode;
class aiScene;
class aiMesh;
class Model
{
public:
	Model(const std::string& path);
	~Model();
    void draw(Shader& shader);
    void drawInstanced(Shader& shader, unsigned int instanceCount);
    void setInstacedModelMatrices(const std::vector<glm::mat4>& modelMatrices);
    std::vector<Mesh> meshes() { return m_meshes; }

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);
    Material loadMaterial(aiMaterial* material);
    unsigned int loadTextureFromFile(const std::string& path);

private:
    std::vector<Mesh> m_meshes;
    std::string m_modelPath;
    std::map<std::string, unsigned int> m_textureCache;
};
