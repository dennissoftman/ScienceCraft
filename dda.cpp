#include "dda.hpp"

DDA2D::DDA2D(const glm::vec2 &src, const glm::vec2 &dir)
    : m_fDist(0.f), m_fDistLimit(FLT_MAX)
{
    m_curPos = m_rayStart = src;
    m_rayDir   = glm::normalize(dir);
    m_unitSize = glm::vec2(sqrtf(1 + (m_rayDir.y / m_rayDir.x)*(m_rayDir.y / m_rayDir.x)),
                           sqrtf(1 + (m_rayDir.x / m_rayDir.y)*(m_rayDir.x / m_rayDir.y)));
    m_rayLen1D = glm::vec2(0, 0);

    if(m_rayDir.x < 0)
    {
        m_rayStep.x = -1;
        m_rayLen1D.x = (m_rayStart.x - float(m_curPos.x)) * m_unitSize.x;
    }
    else
    {
        m_rayStep.x = 1;
        m_rayLen1D.x = (float(m_curPos.x + 1) - m_rayStart.x) * m_unitSize.x;
    }

    if(m_rayDir.y < 0)
    {
        m_rayStep.y = -1;
        m_rayLen1D.y = (m_rayStart.y - float(m_curPos.y)) * m_unitSize.y;
    }
    else
    {
        m_rayStep.y = 1;
        m_rayLen1D.y = (float(m_curPos.y + 1) - m_rayStart.y) * m_unitSize.y;
    }
}

void DDA2D::setLimit(float dist)
{
    m_fDistLimit = dist;
}

bool DDA2D::step()
{
    if(m_rayLen1D.x < m_rayLen1D.y)
    {
        m_curPos.x += m_rayStep.x;
        m_fDist = m_rayLen1D.x;
        m_rayLen1D.x += m_unitSize.x;
    }
    else
    {
        m_curPos.y += m_rayStep.y;
        m_fDist = m_rayLen1D.y;
        m_rayLen1D.y += m_unitSize.y;
    }

    return (m_fDist < m_fDistLimit);
}

const glm::ivec2 &DDA2D::getPos() const
{
    return m_curPos;
}

DDA3D::DDA3D(const glm::vec3 &src, const glm::vec3 &dir)
{
    m_curPos = m_rayStart = src;
    m_rayDir = dir;
}

void DDA3D::setLimit(float dist)
{
    m_fDistLimit = dist;
}

bool DDA3D::step()
{
    return false;
}

const glm::ivec3 &DDA3D::getPos() const
{
    return m_curPos;
}
