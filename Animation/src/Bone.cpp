#define GLM_ENABLE_EXPERIMENTAL
#include "Bone.h"
#include "assimp/scene.h"
#include "AssimpGLMHelpers.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

Bone::Bone(const std::string& name, const aiNodeAnim* channel)
    : m_name(name)
    , m_localTransform(1.0f)
{
    m_positionNum = channel->mNumPositionKeys;
    for (int positionIndex = 0; positionIndex < m_positionNum; ++positionIndex) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
        data.timeStamp = timeStamp;
        m_positions.emplace_back(data);
    }

    m_rotationNum = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < m_rotationNum; ++rotationIndex) {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
        data.timeStamp = timeStamp;
        m_rotations.emplace_back(data);
    }

    m_scaleNum = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < m_scaleNum; ++keyIndex) {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        KeyScale data;
        data.scale = AssimpGLMHelpers::GetGLMVec(scale);
        data.timeStamp = timeStamp;
        m_scales.emplace_back(data);
    }
}

void Bone::Update(float animationTime) {
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScale(animationTime);
    m_localTransform = translation * rotation * scale;
}

glm::mat4 Bone::InterpolatePosition(float animationTime)
{
    if (m_positionNum == 1) {
        return glm::translate(glm::mat4(1.0f), m_positions[0].position);
    }

    // 根据两个关键帧的平移量，计算插值
    int p0Index = GetPositionIndex(animationTime);
    if (p0Index == -1) {
        return glm::mat4(1.0f);
    }
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_positions[p0Index].timeStamp,m_positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(m_positions[p0Index].position,m_positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime)
{
    if (m_rotationNum == 1) {
        glm::quat rotation = glm::normalize(m_rotations[0].orientation);
        return glm::toMat4(rotation);
    }
    int p0Index = GetRotationIndex(animationTime);
    if (p0Index == -1) {
        return glm::mat4(1.0f);
    }
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_rotations[p0Index].timeStamp,m_rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_rotations[p0Index].orientation,m_rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::InterpolateScale(float animationTime)
{
    if (m_scaleNum == 1) {
        return glm::scale(glm::mat4(1.0f), m_scales[0].scale);
    }

    int p0Index = GetScaleIndex(animationTime);
    if (p0Index == -1) {
        return glm::mat4(1.0f);
    }
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_scales[p0Index].timeStamp,m_scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_scales[p0Index].scale, m_scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}


float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

int Bone::GetPositionIndex(float animationTime)
{
    for (int index = 0; index < m_positionNum - 1; ++index) {
        if (animationTime >=  m_positions[index].timeStamp && animationTime < m_positions[index + 1].timeStamp) {
            return index;
        }
    }
    return -1;
}

int Bone::GetRotationIndex(float animationTime)
{
    for (int index = 0; index < m_rotationNum - 1; ++index) {
        if (animationTime >= m_rotations[index].timeStamp && animationTime < m_rotations[index + 1].timeStamp) {
            return index;
        }
    }
    return -1;
}

int Bone::GetScaleIndex(float animationTime)
{
    for (int index = 0; index < m_scaleNum - 1; ++index) {
        if (animationTime >= m_scales[index].timeStamp && animationTime < m_scales[index + 1].timeStamp) {
            return index;
        }
    }
    return -1;
}