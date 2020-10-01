#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

const float DEFAULT_PITCH = 0.0f;
const float DEFAULT_YAW = -90.0f;
const float DEFAULT_FOV = 45.0f;
const float DEFAULT_SENSITIVITY = 0.05f;
const float DEFAULT_SPEED = 2.5f;

enum class CameraDirection
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float pitch = DEFAULT_PITCH,
           float yaw = DEFAULT_YAW,
           float fov = 45.0f,
           float sensitivity = 0.05f,
           float speed = 2.5f,
           float invertVertical = false);
    Camera(float posX,
           float posY,
           float posZ,
           float upX,
           float upY,
           float upZ,
           float pitch,
           float yaw,
           float fov = 45.0f,
           float sensitivity = 0.05f,
           float speed = 2.5f,
           float invertVertical = false);
    virtual ~Camera();

    glm::mat4 calculateView() const;
    void zoom(float yOffset);
    void rotate(float xPos, float yPos);
    virtual void move(CameraDirection direction, float deltaTime);

    float getFov() const;
    const glm::vec3 &getPosition() const;

protected:
    const glm::vec3 &getFront() const;

    void moveInternal(const glm::vec3 &front, CameraDirection direction, float deltaTime);

private:
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 front;

    float pitch;
    float yaw;
    float fov;

    float sensitivity;
    float speed;
    bool invertVertical;

    bool isFirstMove;
    float lastX;
    float lastY;

    static glm::vec3 getFront(float pitch, float yaw);
};

#endif