#include "Camera.h"
#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include <GLFW/glfw3.h>

Camera::Camera(glm::vec3 position, glm::vec3 center, glm::vec3 up)
    : m_position(position)
    , m_center(center)
    , m_up(up)
{
    updateViewMatrix();
}

Camera::~Camera()
{
}

void Camera::setProjectionMatrix(glm::mat4x4 projection) 
{
    m_projection = projection;
    m_invProjection = glm::inverse(projection);
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
    updateViewMatrix();
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
    
    updateViewMatrix();
}

void Camera::handleMousePress(GLFWwindow* window, int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_leftMousePressed = (action == GLFW_PRESS);
        if (m_leftMousePressed) {
            glfwGetCursorPos(window, &m_lastMouseX, &m_lastMouseY);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_rightMousePressed = (action == GLFW_PRESS);
        if (m_rightMousePressed) {
            glfwGetCursorPos(window, &m_lastMouseX, &m_lastMouseY);
        }
    }
}

void Camera::handleMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if (m_leftMousePressed) {
        glm::vec3 currentWorldPos = rayCastFromScreen(xpos, ypos, width, height);
        glm::vec3 lastWorldPos = rayCastFromScreen(m_lastMouseX, m_lastMouseY, width, height);
        glm::vec3 worldDelta = currentWorldPos - lastWorldPos;

        m_position -= worldDelta;
        m_center -= worldDelta;

    } else if (m_rightMousePressed) {
        // 将鼠标坐标映射到球体表面
        glm::vec3 currentSpherePos = screenToSphere(xpos, ypos , width, height);
        glm::vec3 lastSpherePos = screenToSphere(m_lastMouseX, m_lastMouseY, width, height);

        // 计算旋转轴（垂直于两个向量的叉积）
        glm::vec3 rotationAxis = glm::cross(lastSpherePos, currentSpherePos);

        // 计算旋转角度（基于两个向量的点积）
        float dotProduct = glm::dot(glm::normalize(lastSpherePos), glm::normalize(currentSpherePos));
        float rotationAngle = std::acos(glm::clamp(dotProduct, -1.0f, 1.0f));

        // 如果旋转轴长度很小，忽略旋转
        if (glm::length(rotationAxis) < 0.001f) {
            return;
        }

        // 将旋转轴从屏幕空间转换到世界空间
        // 只取视图矩阵的旋转部分（去除平移）
        glm::mat3 viewRotation = glm::mat3(m_invView);
        rotationAxis = viewRotation * rotationAxis;
        rotationAxis = glm::normalize(rotationAxis);
        
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, rotationAxis);
        m_up = glm::mat3(rotationMatrix) * m_up;
        glm::vec3 viewDirection = m_center - m_position;
        viewDirection = glm::mat3(rotationMatrix) * viewDirection;
        m_position = m_center - viewDirection;
        updateViewMatrix();
    }

    // 更新上一个鼠标位置
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
}

void Camera::handleScroll(double yoffset)
{
    float scaleFactor = 1.0f - static_cast<float>(yoffset) * 0.1f;
    glm::vec3 viewDirection = m_center - m_position;
    viewDirection = viewDirection * scaleFactor;
    m_position = m_center - viewDirection;
    updateViewMatrix();
}

glm::vec3 Camera::rayCastFromScreen(double screenX, double screenY, int windowWidth, int windowHeight) {
    glm::vec3 nearPoint = screenToWorld(screenX, screenY, 0.0f, windowWidth, windowHeight);
    glm::vec3 farPoint = screenToWorld(screenX, screenY, 1.0f, windowWidth, windowHeight);
    glm::vec3 rayDirection = glm::normalize(farPoint - nearPoint);
    float t = (m_center.z - nearPoint.z) / rayDirection.z;
    return nearPoint + rayDirection * t;
}

glm::vec3 Camera::screenToWorld(double screenX, double screenY, float depth, int windowWidth, int windowHeight) {
    float ndcX = (2.0f * screenX) / windowWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY) / windowHeight;
    float ndcZ = 2.0f * depth - 1.0f;
    glm::vec4 clipCoords(ndcX, ndcY, ndcZ, 1.0f);
    glm::vec4 eyeCoords = m_invProjection * clipCoords;
    eyeCoords = eyeCoords / eyeCoords.w;
    // 使用视图矩阵的逆矩阵来计算世界坐标
    glm::vec4 worldCoords = m_invView * eyeCoords;
    return glm::vec3(worldCoords);
}

glm::vec3 Camera::screenToSphere(double screenX, double screenY, int windowWidth, int windowHeight) {
    // 归一化鼠标坐标到[-1, 1]（原点在屏幕中心）
    // 与屏幕坐标系一致：X 向右，Y 向下
    glm::vec3 spherePos;
    spherePos.x = (2.0f * screenX / windowWidth - 1.0f);
    spherePos.y = (1.0f - 2.0f * screenY / windowHeight);
    spherePos.z = 0.0f;

    // 计算z值（保证点在球体表面）
    float lengthSq = spherePos.x * spherePos.x + spherePos.y * spherePos.y;
    if (lengthSq <= 1.0f) {
        spherePos.z = std::sqrt(1.0f - lengthSq);
    } else {
        spherePos = glm::normalize(spherePos);
        spherePos.z = 0.0f;
    }

    return spherePos;
}

void Camera::updateViewMatrix() {
    m_view = glm::lookAt(m_position, m_center, m_up);
    m_invView = glm::inverse(m_view);
}