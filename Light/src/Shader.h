#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    Shader(const char* vertexShaderCode, size_t vertexSize, const char* fragmentShaderCode, size_t fragmentSize);
    ~Shader();
    void use();

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void compileAndLink(const char* vShaderCode, const char* fShaderCode);

private:
    unsigned int m_shaderProgram;
};
