#ifndef RAY_HPP
#define RAY_HPP

#include <glm/glm.hpp>

class Ray
{
public:
    Ray(const glm::vec3 &src, const glm::vec3 &dir);

    void setLimit(float dist);
    bool step(float a);

    const glm::vec3 &getPos() const;
private:
    glm::vec3 m_curPos;
    glm::vec3 m_rayStep;
    float m_fDist, m_fDistLimit;
};

#endif // RAY_HPP
