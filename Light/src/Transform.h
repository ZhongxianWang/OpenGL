#pragma once
#include "Camera.h"
#include <glm/fwd.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct GLFWwindow;
class Transform {
public:
    Transform();
    Transform(const glm::vec3& viewCenter, const glm::mat4& view, const glm::mat4& projection);
    ~Transform();
    void setViewCenter(const glm::vec3& center);
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);
    glm::mat4 getTransformMatrix() const { return m_transformMat; }
    void handleMousePress(GLFWwindow* window, int button, int action);
    void handleMouseMove(GLFWwindow* window, double xpos, double ypos);
    void handleScroll(double yoffset);

private:
    glm::vec3 getMouseWorldPosition(double screenX, double screenY, int windowWidth, int windowHeight);
    glm::vec3 screenToWorld(double screenX, double screenY, float depth, int windowWidth, int windowHeight);
    glm::vec3 screenToSphere(double screenX, double screenY, int windowWidth, int windowHeight);

private:
    glm::vec3 m_viewCenter;
    glm::mat4 m_view;
    glm::mat4 m_projection;
    glm::mat4 m_invView;
    glm::mat4 m_invProjection;

    glm::mat4 m_transformMat;
    float m_scale = 1.0f;

    // 鼠标事件
    bool m_leftMousePressed = false;
    bool m_rightMousePressed = false;
    double m_lastMouseX;
    double m_lastMouseY;
};
