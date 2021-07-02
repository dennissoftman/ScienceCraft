#ifndef DDA_HPP
#define DDA_HPP

#include <glm/glm.hpp>

class DDA2D
{
public:
    DDA2D(const glm::vec2 &src, const glm::vec2 &dir);

    void setLimit(float dist);

    bool step();

    const glm::ivec2 &getPos() const;

private:
    glm::vec2 m_rayStart;
    glm::vec2 m_rayDir;
    glm::vec2 m_unitSize;

    glm::ivec2 m_rayStep;
    glm::ivec2 m_curPos;
    glm::vec2 m_rayLen1D;

    float m_fDist;
    float m_fDistLimit;
};

class DDA3D
{
public:
    DDA3D(const glm::vec3 &src, const glm::vec3 &dir);

    void setLimit(float dist);

    bool step();

    const glm::ivec3 &getPos() const;

private:
    glm::vec3 m_rayStart;
    glm::vec3 m_rayDir;
    glm::vec3 m_unitSize;

    glm::ivec3 m_rayStep;
    glm::ivec3 m_curPos;
    glm::ivec3 m_endPos;
    glm::vec3 m_rayLen1D;

    float m_fDist;
    float m_fDistLimit;
};

#endif // DDA_HPP
