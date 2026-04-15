#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

struct aiNodeAnim;
class Bone
{
public:
    Bone() = default;
    Bone(const std::string& name, const aiNodeAnim* channel);
    // 根据时间戳更新变换矩阵
    void Update(float animationTime);
    glm::mat4 GetLocalTransform() { return m_localTransform; }
    std::string GetBoneName() const { return m_name; }

private:
    // 获取某个时刻的平移/旋转/缩放的变换矩阵
    glm::mat4 InterpolatePosition(float animationTime);
    glm::mat4 InterpolateRotation(float animationTime);
    glm::mat4 InterpolateScale(float animationTime);

    // 获取两个关键帧之间插值的系数
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    // 获取某个时刻的平移/旋转/缩放信息索引
    int GetPositionIndex(float animationTime);
    int GetRotationIndex(float animationTime);
    int GetScaleIndex(float animationTime);

private:
    std::vector<KeyPosition> m_positions;
    std::vector<KeyRotation> m_rotations;
    std::vector<KeyScale> m_scales;
    int m_positionNum;
    int m_rotationNum;
    int m_scaleNum;

    glm::mat4 m_localTransform;
    std::string m_name;
};