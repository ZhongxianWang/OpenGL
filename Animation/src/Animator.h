#pragma once
#include <string>

#include "Model.h"
#include "Bone.h"
class Animator
{   
public:
    Animator(const std::string& animationPath, Model* model);
    void UpdateAnimation(float dt);
    std::vector<glm::mat4> GetFinalBoneMatrices() { 
        return m_finalBoneMatrices;  
    }

private:
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
    
private:
    std::vector<glm::mat4> m_finalBoneMatrices;
    float m_currentTick;
    float m_totalTicks;
    int m_ticksPerSecond;
    std::map<std::string, Bone> m_bones;
    Model* m_model;
};