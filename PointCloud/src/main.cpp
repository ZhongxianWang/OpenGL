

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <limits>
#include <pcl/io/ply_io.h>
#include <pcl/io/io.h>
#include "Camera.h"
#include "Transform.h"
#include "Shader.h"

void processInput(GLFWwindow *window);
int g_windowWidth = 800;
int g_windowHeight = 800;

// 回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// 点云数据结构
struct PointCloudData {
    std::vector<float> vertices; // x, y, z, intensity
    float minIntensity;
    float maxIntensity;
    float minX, maxX, minY, maxY, minZ, maxZ;
    glm::vec3 center;
    float radius;
    size_t pointCount;
};

// 函数声明
PointCloudData loadPointCloud(const std::string& filename);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// 加载点云数据函数 - 使用PCL库加载
bool loadPointCloud(const std::string& filename, PointCloudData& data) {
    data.minIntensity = std::numeric_limits<float>::max();
    data.maxIntensity = std::numeric_limits<float>::lowest();
    
    // 使用PCL库加载点云数据
    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZI>);
    
    // 尝试不同的文件格式
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    
    if (extension == "pcd") {
        if (pcl::io::loadPCDFile<pcl::PointXYZI>(filename, *cloud) == -1) {
            std::cout << "Could not read PCD file, creating sample data..." << std::endl;
            return false;
        }
    } else if (extension == "ply") {
        if (pcl::io::loadPLYFile<pcl::PointXYZI>(filename, *cloud) == -1) {
            return false;
        }
    } else {
        return false;
        
    }
    // 计算点云的边界框
    data.minX = std::numeric_limits<float>::max();
    data.maxX = std::numeric_limits<float>::lowest();
    data.minY = std::numeric_limits<float>::max();
    data.maxY = std::numeric_limits<float>::lowest();
    data.minZ = std::numeric_limits<float>::max();
    data.maxZ = std::numeric_limits<float>::lowest();
    
    // 将PCL点云数据转换到我们的数据结构
    for (const auto& point : *cloud) {
        data.vertices.push_back(point.x);
        data.vertices.push_back(point.y);
        data.vertices.push_back(point.z);
        data.vertices.push_back(point.intensity);
        
        data.minIntensity = std::min(data.minIntensity, point.intensity);
        data.maxIntensity = std::max(data.maxIntensity, point.intensity);
        
        data.minX = std::min(data.minX, point.x);
        data.maxX = std::max(data.maxX, point.x);
        data.minY = std::min(data.minY, point.y);
        data.maxY = std::max(data.maxY, point.y);
        data.minZ = std::min(data.minZ, point.z);
        data.maxZ = std::max(data.maxZ, point.z);
    }
    
    data.pointCount = cloud->size();
    
    // 计算点云中心和包围球半径
    data.center = glm::vec3(
        (data.minX + data.maxX) * 0.5f,
        (data.minY + data.maxY) * 0.5f,
        (data.minZ + data.maxZ) * 0.5f
    );
    
    data.radius = std::max({
        data.maxX - data.minX,
        data.maxY - data.minY,
        data.maxZ - data.minZ
    }) * 0.5f;
    
    std::cout << "Loaded " << data.pointCount << " points using PCL" << std::endl;
    std::cout << "Intensity range: [" << data.minIntensity << ", " << data.maxIntensity << "]" << std::endl;
    std::cout << "Bounding box: X[" << data.minX << ", " << data.maxX << "], Y[" 
              << data.minY << ", " << data.maxY << "], Z[" << data.minZ << ", " << data.maxZ << "]" << std::endl;
    std::cout << "Center: (" << data.center.x << ", " << data.center.y << ", " << data.center.z << ")" << std::endl;
    std::cout << "Radius: " << data.radius << std::endl;
    
    return true;
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Point Cloud Viewer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // 启用深度
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SPRITE);
    // 加载点云数据
    PointCloudData pointCloud;
    if (!loadPointCloud("/home/xian/下载/00241_000568_罗湖莲塘_鹏兴路_202603311656_0/label_map/global_map_dense_imu.pcd", pointCloud)) {
        std::cerr << "Failed to load point cloud or no points found" << std::endl;
        return -1;
    }

    Shader shader("./resource/glsl/vertexShader.glsl", "./resource/glsl/fragmentShader.glsl");
    Camera camera(glm::vec3(pointCloud.center.x, pointCloud.center.y, pointCloud.center.z + pointCloud.radius * 2.0f)
    , pointCloud.center, glm::vec3(0.0f, 1.0f, 0.0f));
    Transform transform;
    glfwSetWindowUserPointer(window, &transform);

    // 设置回调
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    
    // 使用加载的点云数据
    const std::vector<float>& vertices = pointCloud.vertices;

    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 位置属性 (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 强度属性
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        shader.setFloat("minIntensity", pointCloud.minIntensity);
        shader.setFloat("maxIntensity", pointCloud.maxIntensity);
        
        glBindVertexArray(VAO);
        glm::mat4 model = glm::mat4(1.0f);
        model = transform.getTransformMatrix() * model;
        glm::mat4 view = camera.getViewMat();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f)
        , (float)g_windowWidth / (float)g_windowHeight, 0.01f, 1000.0f + camera.getDistance());

        transform.setViewCenter(camera.getViewCenter());
        transform.setViewMatrix(view);
        transform.setProjectionMatrix(projection);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        // 绘制点云
        glDrawArrays(GL_POINTS, 0, pointCloud.pointCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    g_windowWidth = width;
    g_windowHeight = height;
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


