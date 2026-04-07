#include "Camera.h"
#include <GLFW/glfw3.h>

Camera::Camera(glm::vec3 position, glm::vec3 center, glm::vec3 up)
    : m_position(position)
    , m_center(center)
    , m_up(up)
{
    m_view = glm::lookAt(m_position, m_center, up);
}

Camera::~Camera()
{
}

void Camera::move(float dx, float dy, float dz)
{
    // 计算相机的前方向向量（从相机位置指向视觉中心）
    glm::vec3 forward = glm::normalize(m_center - m_position);
    // 计算右向量（前向量 × 上向量）
    glm::vec3 right = glm::normalize(glm::cross(forward, m_up));

    // dx 沿右向量移动，dz 沿前向量移动，dy 沿世界 Y 轴移动
    glm::vec3 movement = right * dx + forward * dz + glm::vec3(0.0f, dy, 0.0f);

    // 同时更新相机位置和视觉中心
    m_position += movement;
    m_center += movement;

    m_view = glm::lookAt(m_position, m_center, m_up);
}

void Camera::rotate(float yaw, float pitch)
{
    // 计算相机的前方向向量（从相机位置指向视觉中心）
    glm::vec3 forward = glm::normalize(m_center - m_position);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_up));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));
    
    // 绕右向量旋转（垂直旋转pitch）- 抬头/低头
    glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), pitch, right);
    forward = glm::vec3(pitchRotation * glm::vec4(forward, 0.0f));
    up = glm::vec3(pitchRotation * glm::vec4(up, 0.0f));
    
    // 绕世界上向量旋转（水平旋转yaw）- 左转/右转
    glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    forward = glm::vec3(yawRotation * glm::vec4(forward, 0.0f));
    right = glm::vec3(yawRotation * glm::vec4(right, 0.0f));
    up = glm::vec3(yawRotation * glm::vec4(up, 0.0f));
    
    // 更新视觉中心（保持相机位置不变）
    float distance = glm::length(m_center - m_position);
    m_center = m_position + glm::normalize(forward) * distance;
    
    // 更新上向量
    m_up = glm::normalize(up);
    
    // 重新计算视图矩阵
    m_view = glm::lookAt(m_position, m_center, m_up);
}

float Camera::getDistance() const
{
    return glm::length(m_center - m_position);
}