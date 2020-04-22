#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

enum CameraDirection
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    Camera();

    glm::mat4 calculateView() const;
    void zoom(float yOffset);
    void rotate(float xPos, float yPos);
    void move(CameraDirection direction, float deltaTime);

    float getFov() const;

private:
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;

    float pitch;
    float yaw;
    float fov;

    bool isFirstMove;
    bool invertVertical;
    float lastX;
    float lastY;

    float sensitivity;
    float speed;
};

#endif