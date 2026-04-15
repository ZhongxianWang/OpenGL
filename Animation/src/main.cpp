
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmrc/cmrc.hpp>
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "Transform.h"
#include "Animator.h"

namespace fs = std::filesystem;

CMRC_DECLARE(shaders);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera* camera);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
int window_width = SCR_WIDTH;
int window_height = SCR_HEIGHT;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    std::cout << "Creating GLFW window..." << std::endl;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "GLFW window created successfully" << std::endl;
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // 先获取实际的窗口尺寸
    glfwGetFramebufferSize(window, &window_width, &window_height);
    // 确保 window_width 和 window_height 不为 0
    window_width = (window_width > 0) ? window_width : SCR_WIDTH;
    window_height = (window_height > 0) ? window_height : SCR_HEIGHT;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    std::cout << "Window size initialized: " << window_width << "x" << window_height << std::endl;

    // 创建相机
    //Camera camera(glm::vec3(0.0f, 0.8f, 5.0f), glm::vec3(0.0f, 0.8f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Camera camera(glm::vec3(0.0f, 100.0f, 350.0f), glm::vec3(0.0f, 100.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Transform transform;
    glfwSetWindowUserPointer(window, &transform);

    // 设置回调
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // 从 cmrc 资源加载着色器
    auto fs = cmrc::shaders::get_filesystem();
    auto vertShaderFile = fs.open("resource/glsl/vertexShader.glsl");
    auto fragShaderFile = fs.open("resource/glsl/fragmentShader.glsl");
    Shader shader(
        vertShaderFile.begin(), vertShaderFile.size(),
        fragShaderFile.begin(), fragShaderFile.size()
    );
    
    // 加载 obj 模型
    Model ourModel("resource/models/vampire/dancing_vampire.dae");
    Animator animator("resource/models/vampire/dancing_vampire.dae", &ourModel);
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window, &camera);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        animator.UpdateAnimation(deltaTime);

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_MULTISAMPLE);
        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = transform.getTransformMatrix() * model;
        glm::mat4 view = camera.getViewMat();

        // 防止 window_height 或 window_width 为 0 导致 aspect ratio 错误
        float safeWidth = (window_width > 0) ? (float)window_width : (float)SCR_WIDTH;
        float safeHeight = (window_height > 0) ? (float)window_height : (float)SCR_HEIGHT;
        float aspectRatio = safeWidth / safeHeight;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 2000.0f);

        transform.setViewCenter(camera.getViewCenter());
        transform.setViewMatrix(view);
        transform.setProjectionMatrix(projection);

        shader.setMat4("model", model);
        shader.setMat4("invModel", glm::inverse(model));
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i) {
           shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]); 
        }

        // 设置光照参数
        glm::vec3 lightPos(2.0f, 2.0f, 20.0f);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.getPosition());
        ourModel.draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, Camera* camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 检查是否按下了Ctrl键
    bool ctrlPressed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

    // 移动速度和旋转速度
    const float moveSpeed = 0.02f;
    const float rotateSpeed = 0.002f;

    if (ctrlPressed) {
        // Ctrl + 方向键/WASD: 旋转相机视角
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera->rotate(rotateSpeed, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera->rotate(-rotateSpeed, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera->rotate(0.0f, rotateSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera->rotate(0.0f, -rotateSpeed);
        }
    } else {
        // 方向键/WASD: 移动相机位置和视觉中心
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera->move(-moveSpeed, 0.0f, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera->move(moveSpeed, 0.0f, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera->move(0.0f, 0.0f, moveSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera->move(0.0f, 0.0f, -moveSpeed);
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Transform* transform = static_cast<Transform*>(glfwGetWindowUserPointer(window));
    if (transform) {
        transform->handleMousePress(window, button, action);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    Transform* transform = static_cast<Transform*>(glfwGetWindowUserPointer(window));
    if (transform) {
        transform->handleMouseMove(window, xpos, ypos);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    Transform* transform = static_cast<Transform*>(glfwGetWindowUserPointer(window));
    if (transform) {
        transform->handleScroll(yoffset);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    window_width = width;
    window_height = height;
}


