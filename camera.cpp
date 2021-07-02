#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

Camera::Camera()
    : Camera(90.f, 1.6f)
{
}


Camera::Camera(float fov, float aspect) :
    pos(glm::vec3(0, 0, 0)),
    rot(glm::vec3(0, 0, 0)),
    cam_front(glm::vec3(0,0,-1)),
    cam_up(glm::vec3(0,1,0)),
    rotRestricted(false),
    m_fov(fov),
    m_aspect(aspect)
{
    ProjectionMatrix = glm::perspective(glm::radians(m_fov), m_aspect, 0.5f, 1000.f);
}

Camera::~Camera()
{

}

void Camera::move(float mx, float my, float mz)
{
    move(glm::vec3(mx, my, mz));
}

void Camera::move(const glm::vec3 &mv)
{
    pos += mv;
}

void Camera::setPos(const glm::vec3 &newpos)
{
    pos = newpos;
}

void Camera::restrict(const glm::vec3 &axis, float _min, float _max)
{
    rotRestricted = true;
    restrictAxis = glm::normalize(axis);
    camMinRot = _min;
    camMaxRot = _max;
}

void Camera::rotate(const glm::vec3 &axis, float angle)
{
    glm::vec3 old = rot;
    rot += glm::normalize(axis) * glm::radians(angle);
    if(rotRestricted)
    {
        glm::vec3 r = (rot * restrictAxis);
        float rv = (r.x + r.y + r.z);
        if(rv <= camMinRot || rv >= camMaxRot)
            rot = old;
    }
}

void Camera::setRotation(const glm::vec3 &newRot)
{
    rot = glm::vec3(glm::radians(newRot.x), glm::radians(newRot.y), glm::radians(newRot.z));
}

void Camera::setFOV(float newfov)
{
    m_fov = newfov;
}

void Camera::setAspect(float newAspect)
{
    m_aspect = newAspect;
}

void Camera::setLookAt(const glm::vec3 &target)
{
    float ax = glm::angle(glm::vec2(pos.x, pos.y), glm::vec2(target.x, target.y));
    float ay = glm::angle(glm::vec2(pos.z, pos.x), glm::vec2(target.z, target.x));
    float az = glm::angle(glm::vec2(pos.z, pos.y), glm::vec2(target.z, target.y));

    rot = glm::vec3(ax, ay, az);
}

void Camera::update()
{
    ViewMatrix = glm::mat4();

    cam_front.x = glm::cos(rot.y) * glm::cos(rot.x);
    cam_front.y = glm::sin(rot.x);
    cam_front.z = glm::sin(rot.y) * glm::cos(rot.x);
    cam_front   = glm::normalize(cam_front);

    ViewMatrix = glm::lookAt(pos, pos+cam_front, cam_up);
}

glm::vec3 &Camera::fwdVector()
{
    return cam_front;
}

glm::mat4 &Camera::GetProjection()
{
    return ProjectionMatrix;
}

glm::mat4 &Camera::GetView()
{
    return ViewMatrix;
}

glm::vec3 &Camera::getPos()
{
    return pos;
}

glm::vec3 &Camera::getRot()
{
    return rot;
}

glm::vec3 Camera::getTarget()
{
    return glm::normalize(glm::vec3(pos+cam_front));
}

glm::vec3 &Camera::getUpAxis()
{
    return cam_up;
}
