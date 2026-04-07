# 最低 CMake 版本（兼容主流系统）
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

project(texture VERSION 0.1 LANGUAGES C CXX)

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

add_executable(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

# 链接依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
)

# 包含目录（让代码能找到 GLAD/GLFW 头文件）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GLFW_DIR}/include
    ${CMAKE_SOURCE_DIR}/thirdparty
)