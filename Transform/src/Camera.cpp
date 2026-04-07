#include "Camera.h"
#include "GLFW/glfw3.h"

Camera::Camera(const glm::vec3& cameraPos, const glm::vec3& viewCenter, const glm::vec3& cameraUp)
    : m_cameraPos(cameraPos), m_viewCenter(viewCenter), m_cameraUp(cameraUp),
      m_isLeftButtonPressed(false), m_isRightButtonPressed(false)
{
    m_view = glm::lookAt(m_cameraPos, m_viewCenter, m_cameraUp);
}

Camera::~Camera()
{
}

void Camera::processMousePress(int button, int action, glm::vec2 screenPos)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_isLeftButtonPressed = (action == GLFW_PRESS);
        if (m_isLeftButtonPressed) {
            m_lastMousePos = screenPos;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_isRightButtonPressed = (action == GLFW_PRESS);
        if (m_isRightButtonPressed) {
            m_lastMousePos = screenPos;
        }
    }
}

void Camera::processMouseMove(glm::vec2 screenPos, int width, int height)
{
    glm::vec2 delta = screenPos - m_lastMousePos;
    m_lastMousePos = screenPos;

    if (m_isLeftButtonPressed) {
        // 左键平移：鼠标移动距离等于视角移动距离
        // 计算相机到观察中心的距离作为参考深度
        glm::vec3 forward = glm::normalize(m_viewCenter - m_cameraPos);
        float distance = glm::length(m_viewCenter - m_cameraPos);

        // 获取观察中心在屏幕空间的深度值
        glm::mat4 viewProj = m_projection * m_view;
        glm::vec4 viewCenterClip = viewProj * glm::vec4(m_viewCenter, 1.0f);
        float depth = viewCenterClip.z / viewCenterClip.w;  // NDC depth [-1, 1]

        // 将鼠标起点和终点转换到世界空间
        glm::vec3 worldPos1 = screenToWorld(m_lastMousePos - delta, (depth + 1.0f) * 0.5f, width, height);
        glm::vec3 worldPos2 = screenToWorld(m_lastMousePos, (depth + 1.0f) * 0.5f, width, height);

        // 计算世界空间的移动向量
        glm::vec3 worldDelta = worldPos2 - worldPos1;

        // 更新相机位置和观察中心
        m_cameraPos -= worldDelta;
        m_viewCenter -= worldDelta;

        // 更新视图矩阵
        m_view = glm::lookAt(m_cameraPos, m_viewCenter, m_cameraUp);
    } else if (m_isRightButtonPressed) {
        // 右键旋转：使用轨道球算法
        float sphereRadius = glm::min(width, height) * 0.5f;
        glm::vec2 center(width * 0.5f, height * 0.5f);

        // 将屏幕坐标映射到虚拟球体
        auto mapToSphere = [&](glm::vec2 pos) -> glm::vec3 {
            glm::vec2 normalized = (pos - center) / sphereRadius;
            float length = glm::length(normalized);

            if (length > 1.0f) {
                normalized = glm::normalize(normalized);
                return glm::vec3(normalized, 0.0f);
            } else {
                float z = glm::sqrt(1.0f - length * length);
                return glm::vec3(normalized, z);
            }
        };

        // 获取起点和终点的球体坐标
        glm::vec3 startSphere = mapToSphere(m_lastMousePos - delta);
        glm::vec3 endSphere = mapToSphere(m_lastMousePos);

        // 计算旋转轴和角度
        glm::vec3 cross = glm::cross(startSphere, endSphere);
        float dot = glm::dot(startSphere, endSphere);
        float angle = glm::acos(glm::clamp(dot, -1.0f, 1.0f));

        if (angle > 0.00001f) {
            // 创建旋转矩阵
            glm::vec3 axis = glm::normalize(cross);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);

            // 绕观察中心旋转相机位置
            glm::vec3 offset = m_cameraPos - m_viewCenter;
            offset = glm::vec3(rotation * glm::vec4(offset, 1.0f));
            m_cameraPos = m_viewCenter + offset;

            // 更新上方向
            m_cameraUp = glm::vec3(rotation * glm::vec4(m_cameraUp, 1.0f));
            m_cameraUp = glm::normalize(m_cameraUp);

            // 更新视图矩阵
            m_view = glm::lookAt(m_cameraPos, m_viewCenter, m_cameraUp);
        }
    }
}

glm::vec3 Camera::screenToWorld(glm::vec2 screenPos, float depth, int width, int height)
{
    // 1. 将屏幕坐标转换为标准化设备坐标 (NDC)
    // 屏幕坐标原点在左上角，NDC坐标原点在中心
    // depth 参数应该是读取的深度缓冲区值，范围在 [0, 1] 之间
    float ndcX = (2.0f * screenPos.x) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * screenPos.y) / height;
    float ndcZ = 2.0f * depth - 1.0f;  // 将 [0,1] 映射到 [-1,1]

    // 2. 创建NDC坐标向量
    glm::vec4 ndc(ndcX, ndcY, ndcZ, 1.0f);

    // 3. 通过逆投影矩阵将NDC转换为观察空间坐标
    glm::vec4 eyeCoords = glm::inverse(m_projection) * ndc;

    // 4. 透视除法
    // 注意：正向变换时 w_clip = -z_view，所以反向变换时需要除以 w
    eyeCoords /= eyeCoords.w;

    // 5. 通过逆视图矩阵将观察空间坐标转换为世界空间坐标
    glm::vec4 worldCoords = glm::inverse(m_view) * eyeCoords;

    return glm::vec3(worldCoords.x, worldCoords.y, worldCoords.z);
}

glm::vec3 Camera::screenToWorldRayDirection(glm::vec2 screenPos, int width, int height)
{
    // 将屏幕坐标转换为NDC
    float ndcX = (2.0f * screenPos.x) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * screenPos.y) / height;

    // 创建NDC坐标（z = -1 表示在近裁剪面）
    glm::vec4 ndc(ndcX, ndcY, -1.0f, 1.0f);

    // 逆投影变换到观察空间
    glm::vec4 eyeCoords = glm::inverse(m_projection) * ndc;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);  // w=0 表示方向向量

    // 逆视图变换到世界空间
    glm::vec4 worldCoords = glm::inverse(m_view) * eyeCoords;
    glm::vec3 worldDir = glm::normalize(glm::vec3(worldCoords.x, worldCoords.y, worldCoords.z));

    return worldDir;
}

