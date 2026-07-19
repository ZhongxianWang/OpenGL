#pragma once
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

    glm::vec3 getViewCenter() const { return m_center; }
    glm::vec3 getPosition() const { return m_position; }
    glm::mat4 getViewMat() const { return m_view; }

    // 移动相机位置和视觉中心
    void move(float dx, float dy, float dz);
    // 旋转相机视角（围绕视觉中心旋转）
    void rotate(float yaw, float pitch);

private:
    // 相机位置、视觉中心、上向量
    glm::vec3 m_position;
    glm::vec3 m_center;
    glm::vec3 m_up;
    glm::mat4 m_view;

};
