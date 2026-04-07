# 最低 CMake 版本（兼容主流系统）
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 生成 compile_commands.json 用于 IDE 智能提示
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

project(assimp_model VERSION 0.1 LANGUAGES C CXX)

if (UNIX AND NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

if (WIN32)
     set(GLFW_DIR ${CMAKE_SOURCE_DIR}/thirdparty/glfw)
     set(GLFW_LIB "${GLFW_DIR}/lib/libglfw3.a")
     
     message(STATUS "GLFW directory: ${GLFW_DIR}")
     message(STATUS "Looking for GLFW static library at: ${GLFW_LIB}")
     
     # 检查文件是否存在
     if(NOT EXISTS ${GLFW_LIB})
         message(FATAL_ERROR "GLFW static library not found at ${GLFW_LIB}")
     endif()
     
     # 链接 GLFW 静态库
     add_library(glfw STATIC IMPORTED)
     set_target_properties(glfw PROPERTIES
         IMPORTED_LOCATION "${GLFW_LIB}"
         INTERFACE_INCLUDE_DIRECTORIES "${GLFW_DIR}/include"
     )
elseif (UNIX)
     find_package(glfw3 REQUIRED)
endif()


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
    ${CMAKE_SOURCE_DIR}/src/Mesh.h
    ${CMAKE_SOURCE_DIR}/src/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Shader.h
    ${CMAKE_SOURCE_DIR}/src/Shader.cpp
    ${CMAKE_SOURCE_DIR}/src/Model.h
    ${CMAKE_SOURCE_DIR}/src/Model.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.h
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Transform.h
    ${CMAKE_SOURCE_DIR}/src/Transform.cpp
)

# 在编译时拷贝模型和纹理资源到构建目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resource/models
        ${CMAKE_BINARY_DIR}/resource/models
    COMMENT "Copying model and texture resources to build directory"
)

# 链接依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    assimp
    shaders
)

# 包含目录（让代码能找到 GLAD/GLFW 头文件）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty
)