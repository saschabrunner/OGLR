#ifndef FPSCAMERA_H
#define FPSCAMERA_H

#include "Camera.h"

class FpsCamera : public Camera
{
public:
    // inherit constructors
    using Camera::Camera;

    void move(CameraDirection direction, float deltaTime) override;
};

#endif