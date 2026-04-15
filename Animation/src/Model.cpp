#include "Model.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include "glm/fwd.hpp"
#include "stb_image/stb_image.h"
#include "AssimpGLMHelpers.h"

namespace fs = std::filesystem;
Model::Model(const std::string& path)
    : m_modelPath(path)
{
    loadModel(path);
}

Model::~Model()
{
}

void Model::draw(Shader& shader)
{
    for (auto& mesh : m_meshes) {
        mesh.draw(shader);
    }
}

void Model::loadModel(const std::string& path)
{
    std::cout << "Loading model from: " << path << std::endl;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
    aiProcess_GenSmoothNormals |
    aiProcess_LimitBoneWeights |  // 关键！处理骨骼
    aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    m_root = processNode(scene->mRootNode, scene);
}

AssimpNodeData* Model::processNode(aiNode* node, const aiScene* scene)
{
    // 处理当前节点的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processMesh(mesh, scene));
    }

    AssimpNodeData* nodeData = new AssimpNodeData;
    nodeData->name = node->mName.data;
    nodeData->transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);
    nodeData->childrenCount = node->mNumChildren;

    if (node == scene->mRootNode) {
        nodeData->transformation = glm::mat4(1.0f);
    }

    // 递归处理所有子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        nodeData->children.emplace_back(processNode(node->mChildren[i], scene));
    }
    return nodeData;
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if (mesh->mNormals) {
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        unsigned int numUVChannels = mesh->GetNumUVChannels();
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }

        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            vertex.boneIDs[i] = -1;
            vertex.weights[i] = 0.0f;
        }
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        
        if (m_boneOffsetMap.find(boneName) == m_boneOffsetMap.end()) {
            BoneOffset boneOffset;
            boneOffset.id = m_boneIndex;
            boneOffset.offsetMatrix = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                mesh->mBones[boneIndex]->mOffsetMatrix);
            m_boneOffsetMap[boneName] = boneOffset;
            boneID = m_boneIndex;
            m_boneIndex++;
        } else {
            boneID = m_boneOffsetMap[boneName].id;
        }

        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= vertices.size());

            // 找到第一个空位，设置权重和骨骼ID
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                if (vertices[vertexId].boneIDs[i] < 0) {
                    vertices[vertexId].weights[i] = weight;
                    vertices[vertexId].boneIDs[i] = boneID;
                    break;
                }
            }
        }
    }
    
    // 处理材质和纹理
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    Material mat;
    if (material) {
        // 加载漫反射纹理
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 加载镜面反射纹理
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 加载法线纹理
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 加载材质属性
        mat = loadMaterial(material);
    }
    return Mesh(vertices, indices, textures, mat);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        fs::path directory =  fs::path(m_modelPath).parent_path();
        directory.append(str.C_Str());
        std::string filename = directory.string();

        if (!std::filesystem::exists(filename)) {
            std::string lowerPath = filename;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
                [](unsigned char c){ return std::tolower(c); });
            if (std::filesystem::exists(lowerPath)) {
                filename = lowerPath;
            }
        }

        unsigned int textureID = loadTextureFromFile(filename);
        if (textureID != 0) {
            Texture texture;
            texture.id = textureID;
            texture.type = typeName;
            textures.push_back(texture);
        } else {
            std::cout << "  Failed to load texture!" << std::endl;
        }
    }
    return textures;
}

Material Model::loadMaterial(aiMaterial* material) {
    Material mat;
    if (!material) {
        return mat;
    }
    
    aiColor3D color;
    if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
        mat.ambient = glm::vec3(color.r, color.g, color.b);
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        mat.diffuse = glm::vec3(color.r, color.g, color.b);
    if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
        mat.specular = glm::vec3(color.r, color.g, color.b);
    
    float shininess;
    if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
        mat.shininess = shininess;
    
    return mat;
}

unsigned int Model::loadTextureFromFile(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);
    // 检查缓存中是否已有此纹理
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        //std::cout << "Texture loaded: " << path << " (" << width << "x" << height << ")" << std::endl;

        // 添加到缓存
        m_textureCache[path] = textureID;
        return textureID;
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        std::cout << "  Current working directory: " << std::filesystem::current_path() << std::endl;
        std::cout << "  File exists: " << (std::filesystem::exists(path) ? "yes" : "no") << std::endl;
        stbi_image_free(data);
        return 0;
    }
}
