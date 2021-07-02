#ifndef SHADERMANAGER_HPP
#define SHADERMANAGER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "dist.hpp"

class Shader
{
public:
    Shader();
    Shader(const std::string &v_path, const std::string &f_path, const std::string &g_path="");
    ~Shader();

    void use();

    void setInt(const std::string &prop, int a);
    void setFloat(const std::string &prop, float a);
    void setMat4(const std::string &prop, const glm::mat4 &mat);

    void setVec2(const std::string &prop, const glm::vec2 &vec);
    void setVec3(const std::string &prop, const glm::vec3 &vec);

    void setIntArray(const std::string &prop, const int *arr, int size);
private:
    GLuint shp; // shader program id
};

class ShaderManager
{
public:
    ShaderManager();

    Shader *get(const std::string &id) const;

    void loadShader(const std::vector<std::string> &paths, const std::string &id);
private:
    std::unordered_map<std::string, Shader*> m_shaders;
};

#endif // SHADERMANAGER_HPP
