#include "Animator.h"
#include <filesystem>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Animator::Animator(const std::string& animationPath, Model* model)
    : m_model(model)
{
    m_currentTick = 0.0;
    m_finalBoneMatrices.reserve(100);
    int boneCount = model->GetBoneCount();
    for (int i = 0; i < boneCount; i++) {
        m_finalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    auto animation = scene->mAnimations[0];
    m_totalTicks = animation->mDuration;
    m_ticksPerSecond = animation->mTicksPerSecond;

    int size = animation->mNumChannels;
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;
        m_bones[boneName] = Bone(boneName, channel);
    }
}

void Animator::UpdateAnimation(float dt)
{
    m_currentTick += m_ticksPerSecond * dt;
    m_currentTick = fmod(m_currentTick, m_totalTicks);
    CalculateBoneTransform(m_model->GetRootNode(), glm::mat4(1.0f));
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;
    if (m_bones.find(nodeName) != m_bones.end()) {
        Bone bone = m_bones[nodeName];
        bone.Update(m_currentTick);
        nodeTransform = bone.GetLocalTransform();
    }
    
    auto boneOffset = m_model->GetBoneOffsetMap();
    if (boneOffset.find(nodeName) != boneOffset.end()) {
        int index = boneOffset[nodeName].id;
        glm::mat4 offset = boneOffset[nodeName].offsetMatrix;
        m_finalBoneMatrices[index] = parentTransform * nodeTransform * offset;
    }

    for (int i = 0; i < node->childrenCount; i++) {
        CalculateBoneTransform(node->children[i], parentTransform * nodeTransform);
    }
}