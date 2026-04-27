# 最低 CMake 版本（兼容主流系统）
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# 生成 compile_commands.json 用于 IDE 智能提示
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

project(light VERSION 0.1 LANGUAGES C CXX)

if (UNIX AND NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# 添加 GLFW 子项目（使用本地源码）
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw)

add_library(glad STATIC
    ${CMAKE_SOURCE_DIR}/thirdparty/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/thirdparty/glad/include
)

add_subdirectory(thirdparty/cmrc)
cmrc_add_resource_library(shaders
    NAMESPACE shaders
    WHENCE ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/resource/glsl/vertexShader.glsl
    ${CMAKE_SOURCE_DIR}/resource/glsl/fragmentShader.glsl
)

add_executable(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/Shader.h
    ${CMAKE_SOURCE_DIR}/src/Shader.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.h
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Transform.h
    ${CMAKE_SOURCE_DIR}/src/Transform.cpp
    ${CMAKE_SOURCE_DIR}/src/Mesh.h
    ${CMAKE_SOURCE_DIR}/src/Mesh.cpp
)

# 设置主项目的编译选项（解决中文输出乱码问题）
if (WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

# 链接依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    shaders
)

# 包含目录（让代码能找到 GLAD/GLFW/GLM 头文件）
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SOURCE_DIR}/thirdparty")