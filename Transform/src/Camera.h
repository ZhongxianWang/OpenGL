#pragma once
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
    Camera(const glm::vec3& cameraPos, const glm::vec3& viewCenter, const glm::vec3& cameraUp);
    ~Camera();
    void setProjectMatrix(const glm::mat4& projection) { m_projection = projection; };

    glm::mat4 getViewMatrix() { return m_view; };
    glm::vec3 getCameraPos() { return m_cameraPos; };

    void processMousePress(int button, int action, glm::vec2 screenPos);
    void processMouseMove(glm::vec2 screenPos, int width, int height);

private:
    glm::vec3 screenToWorld(glm::vec2 screenPos, float depth, int width, int height);
    glm::vec3 screenToWorldRayDirection(glm::vec2 screenPos, int width, int height);


private:
    glm::vec3 m_cameraPos;
    glm::vec3 m_viewCenter;
    glm::vec3 m_cameraUp;
    glm::mat4 m_view;

    glm::mat4 m_projection;

    bool m_isLeftButtonPressed;
    bool m_isRightButtonPressed;
    glm::vec2 m_lastMousePos;
};

