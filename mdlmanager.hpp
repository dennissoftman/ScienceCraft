#ifndef MDLMANAGER_HPP
#define MDLMANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "dist.hpp"

// vertCoord + normalCoord + texCoord
struct vertex_t
{
    glm::vec3 vert;
    glm::vec3 norm;
    glm::vec2 tex;
};

class Model3D
{
public:
    Model3D();
    Model3D(const std::string &path);
    ~Model3D();

    GLuint getVAO() const;
    GLuint getSize() const;
private:
    void loadOBJ(const std::string &path, std::vector<vertex_t> &out);

    GLuint VAO, VBO, m_size;
};

class MdlManager
{
public:
    MdlManager();
    ~MdlManager();

    Model3D *get(const std::string &id) const;

    void loadModel(const std::string &path, const std::string &id);
private:
    std::unordered_map<std::string, Model3D*> m_models;
};

#endif // MDLMANAGER_HPP
