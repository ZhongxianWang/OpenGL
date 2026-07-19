
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 创建纹理
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 800, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 绑定到帧缓冲
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
    

    // 创建渲染缓冲区
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float rectVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    // 
    unsigned int rectVAO, rectVBO;
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), &rectVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // 创建相机
    Camera camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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

    auto rectVertShaderFile = fs.open("resource/glsl/rectVertexShader.glsl");
    auto rectFragShaderFile = fs.open("resource/glsl/rectFragmentShader.glsl");
    
    Shader shader(
        vertShaderFile.begin(), vertShaderFile.size(),
        fragShaderFile.begin(), fragShaderFile.size()
    );

    Shader rectShader(
        rectVertShaderFile.begin(), rectVertShaderFile.size(),
        rectFragShaderFile.begin(), rectFragShaderFile.size()
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    rectShader.use();
    rectShader.setInt("screenTexture", 0);

    // 加载 obj 模型
    std::vector<Model> models;
    std::string modelsDir = "resource/models/backpack";
    fs::path modelPath = modelsDir;
    for (const auto& entry : fs::directory_iterator(modelPath.make_preferred())) {
        if (entry.path().extension() == ".obj"
            || entry.path().extension() == ".pmx"
            || entry.path().extension() == ".FBX"
            || entry.path().extension() == ".dae") {
            std::cout << "Loading model: " << entry.path().string() << std::endl;
            try {
                models.emplace_back(entry.path().string());
            } catch (const std::exception& e) {
                std::cerr << "Error loading model " << entry.path().string() << ": " << e.what() << std::endl;
            }
        }
    }

    while (!glfwWindowShouldClose(window))
    {
        processInput(window, &camera);
        glViewport(0, 0, 800, 800);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = transform.getTransformMatrix() * model;
        glm::mat4 view = camera.getViewMat();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);

        transform.setViewCenter(camera.getViewCenter());
        transform.setViewMatrix(view);
        transform.setProjectionMatrix(projection);

        shader.setMat4("model", model);
        shader.setMat4("invModel", glm::inverse(model));
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // 设置光照参数
        glm::vec3 lightPos(2.0f, 2.0f, 20.0f);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.getPosition());

        float xOffset = 0.0f;
        float zOffset = 0.0f;
        int modelsPerRow = 5;
        float spacing = 3.0f;
        for (size_t i = 0; i < models.size(); ++i) {
            models[i].draw(shader);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, 800, 800);
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        rectShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(rectVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

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
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    window_width = width;
    window_height = height;
}


