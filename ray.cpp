#include "ray.hpp"

Ray::Ray(const glm::vec3 &src, const glm::vec3 &dir)
    : m_fDist(0.f), m_fDistLimit(FLT_MAX)
{
    m_curPos  = src;
    m_rayStep = glm::normalize(dir);
}

void Ray::setLimit(float dist)
{
    m_fDistLimit = dist;
}

bool Ray::step(float a)
{
    glm::vec3 dv = a * m_rayStep;
    m_curPos += dv;
    m_fDist += glm::length(dv);
    return (m_fDist < m_fDistLimit);
}

const glm::vec3 &Ray::getPos() const
{
    return m_curPos;
}
