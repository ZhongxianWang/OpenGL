#pragma once
#include "glm/fwd.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct GLFWwindow;
class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 center, glm::vec3 up);
    ~Camera();

    void setProjectionMatrix(glm::mat4x4 projection);

    glm::vec3 getViewCenter() const { return m_center; }
    glm::vec3 getPosition() const { return m_position; }
    glm::mat4 getViewMat() const { return m_view; }

    // 移动相机位置和视觉中心
    void move(float dx, float dy, float dz);
    // 旋转相机视角（围绕视觉中心旋转）
    void rotate(float yaw, float pitch);

    void handleMousePress(GLFWwindow* window, int button, int action);
    void handleMouseMove(GLFWwindow* window, double xpos, double ypos);
    void handleScroll(double yoffset);

private:
    glm::vec3 rayCastFromScreen(double screenX, double screenY, int windowWidth, int windowHeight);
    glm::vec3 screenToWorld(double screenX, double screenY, float depth, int windowWidth, int windowHeight);
    glm::vec3 screenToSphere(double screenX, double screenY, int windowWidth, int windowHeight);

    void updateViewMatrix();

private:
    // 相机位置、视觉中心、上向量
    glm::vec3 m_position;
    glm::vec3 m_center;
    glm::vec3 m_up;

    glm::mat4 m_view;
    glm::mat4 m_projection;

    glm::mat4 m_invView;
    glm::mat4 m_invProjection;

    float m_scale = 1.0f;
    // 鼠标事件
    bool m_leftMousePressed = false;
    bool m_rightMousePressed = false;
    double m_lastMouseX;
    double m_lastMouseY;
};