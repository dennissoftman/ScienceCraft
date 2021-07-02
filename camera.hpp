#ifndef CAMERA_H
#define CAMERA_H

#define DEFAULT_CAM_HEIGHT 2.f
#define DEFAULT_CAM_PITCH 40.f
#define DEFAULT_SCROLL_SPEED 3.f
#define DEFAULT_CAM_FOV 90.f

#include "dist.hpp"

class Camera
{
public:
    Camera();
    Camera(float fov, float aspect);
    ~Camera();

    void move(float mx = 0.f, float my = 0.f, float mz = 0.f);
    void move(const glm::vec3 &mv);

    void setPos(const glm::vec3 &newpos);

    void rotate(const glm::vec3 &axis, float angle);
    void setRotation(const glm::vec3 &newRot);

    void restrict(const glm::vec3 &axis, float _min, float _max);

    void setFOV(float newfov);
    void setAspect(float newAspect);

    void setLookAt(const glm::vec3 &target);

    void update();
    glm::vec3& fwdVector();

    glm::mat4 &GetProjection();
    glm::mat4 &GetView();

    glm::vec3 &getPos();
    glm::vec3 &getRot();
    glm::vec3 getTarget();
    glm::vec3 &getUpAxis();
private:
    glm::vec3 pos, rot;

    glm::vec3 cam_front, cam_up;

    bool rotRestricted;
    glm::vec3 restrictAxis;
    float camMinRot, camMaxRot;

    float m_fov, m_aspect;
    glm::mat4 ProjectionMatrix, ViewMatrix;
};

#endif // CAMERA_H
