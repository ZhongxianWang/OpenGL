#include "Transform.h"
#include <GLFW/glfw3.h>

Transform::Transform()
    : m_viewCenter(0.0f, 0.0f, 0.0f)
    , m_view(1.0f)
    , m_projection(1.0f)
    , m_transformMat(1.0f)
{
}

Transform::Transform(const glm::vec3& viewCenter, const glm::mat4& view, const glm::mat4& projection)
    : m_viewCenter(viewCenter)
    , m_view(view)
    , m_projection(projection)
    , m_transformMat(1.0f)
{
    m_invView = glm::inverse(m_view);
    m_invProjection = glm::inverse(m_projection);
}

Transform::~Transform()
{
}

void Transform::setViewCenter(const glm::vec3& viewCenter)
{
    m_viewCenter = viewCenter;
}

void Transform::setViewMatrix(const glm::mat4& view)
{
    if (m_view == view) {
        return;
    }
    m_view = view;
    m_invView = glm::inverse(m_view);
}

void Transform::setProjectionMatrix(const glm::mat4& projection)
{
    if (m_projection == projection) {
        return;
    }
    m_projection = projection;
    m_invProjection = glm::inverse(m_projection);
}

void Transform::handleMousePress(GLFWwindow* window, int button, int action)
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

void Transform::handleMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if (m_leftMousePressed) {
        glm::vec3 currentWorldPos = getMouseWorldPosition(xpos, ypos, width, height);
        glm::vec3 lastWorldPos = getMouseWorldPosition(m_lastMouseX, m_lastMouseY, width, height);
        glm::vec3 worldDelta = currentWorldPos - lastWorldPos;

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), worldDelta);
        m_transformMat = translationMatrix * m_transformMat;

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

        m_transformMat = rotationMatrix * m_transformMat;
    }

    // 更新上一个鼠标位置
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
}

void Transform::handleScroll(double yoffset)
{
    float scaleFactor = 1.0f + static_cast<float>(yoffset) * 0.1f;
    m_scale *= scaleFactor;
    m_scale = glm::clamp(m_scale, 0.01f, 100.0f);

    // 围绕视图中心缩放
    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -m_viewCenter);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), m_viewCenter);

    m_transformMat = translateBack * scaleMatrix * translateToOrigin * m_transformMat;
}

glm::vec3 Transform::getMouseWorldPosition(double screenX, double screenY, int windowWidth, int windowHeight) {
    glm::vec3 nearPoint = screenToWorld(screenX, screenY, 0.0f, windowWidth, windowHeight);
    glm::vec3 farPoint = screenToWorld(screenX, screenY, 1.0f, windowWidth, windowHeight);
    glm::vec3 rayDirection = glm::normalize(farPoint - nearPoint);
    float t = (m_viewCenter.z - nearPoint.z) / rayDirection.z;
    return nearPoint + rayDirection * t;
}

glm::vec3 Transform::screenToWorld(double screenX, double screenY, float depth, int windowWidth, int windowHeight) {
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

glm::vec3 Transform::screenToSphere(double screenX, double screenY, int windowWidth, int windowHeight) {
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