
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

project(triangle VERSION 0.1 LANGUAGES C CXX)

if (UNIX AND NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# 添加 GLFW 子项目（使用本地源码）
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw)

# 引入 GLAD 代码
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
    ${GLFW_DIR}/include       # 手动链接时的 GLFW 头文件路径
)

# -------------------------- 安装（可选） --------------------------
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)