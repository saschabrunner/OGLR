#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position,
               glm::vec3 up,
               float pitch,
               float yaw,
               float fov,
               float sensitivity,
               float speed,
               float invertVertical)
    : position(position),
      up(up),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      pitch(pitch),
      yaw(yaw),
      fov(fov),
      sensitivity(sensitivity),
      speed(speed),
      invertVertical(invertVertical)
{
    isFirstMove = true;
    lastX = 0.0f;
    lastY = 0.0f;
}

Camera::Camera(float posX,
               float posY,
               float posZ,
               float upX,
               float upY,
               float upZ,
               float pitch,
               float yaw,
               float fov,
               float sensitivity,
               float speed,
               float invertVertical)
    : position(glm::vec3(posX, posY, posZ)),
      up(glm::vec3(upX, upY, upZ)),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      pitch(pitch),
      yaw(yaw),
      fov(fov),
      sensitivity(sensitivity),
      speed(speed),
      invertVertical(invertVertical)
{
    isFirstMove = true;
    lastX = 0.0f;
    lastY = 0.0f;
}

glm::mat4 Camera::calculateView() const
{
    // the view transformation matrix is calculated by
    // 1. subtracting the target vector we're looking at from the camera position (cameraPos - cameraTarget)
    //      this will result in a vector that points in the opposite direction of where the camera is pointing at
    // 2. calculating the cross product of the direction vector from step 1. and a vector pointing straight up in world space
    //      this will result in a vector that's orthogonal to both input vectors, which happens to be the right axis of the camera
    // 3. calculating the cross product of the direction vector from step 1. and the right axis vector from step 2.
    //      this will result in another vector orthogonal to both inputs, which is the up axis of the camera
    // 4. calculate the view matrix by putting the camera directions in a rotation matrix and multiplying it with a
    //    translation matrix (lookAt = rotation x translation) - note the order of the multiplication is different!
    // glm::lookAt will do all of that for us, by just providing the initial three vectors: camera position, target, up
    return glm::lookAt(position, position + front, up);
}

void Camera::zoom(float yOffset)
{
    fov += yOffset;
    if (fov <= 1.0f)
    {
        fov = 1.0f;
    }
    else if (fov >= 90.0f)
    {
        fov = 90.0f;
    }
}

void Camera::rotate(float xPos, float yPos)
{
    if (isFirstMove)
    {
        lastX = xPos;
        lastY = yPos;
        isFirstMove = false;
    }

    float xOffset = (xPos - lastX) * sensitivity;
    float yOffset = (yPos - lastY) * sensitivity;
    lastX = xPos;
    lastY = yPos;

    if (!invertVertical)
    {
        // yOffset is inverted by default, invert it again
        yOffset = -yOffset;
    }

    yaw += xOffset;
    pitch += yOffset;

    // constrain pitch
    if (pitch > 89.0f)
    {
        pitch = 89.0f;
    }
    else if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    // calculate camera direction vector
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
}

void Camera::move(CameraDirection direction, float deltaTime)
{
    moveInternal(front, direction, deltaTime);
}

void Camera::moveInternal(const glm::vec3 &forward, CameraDirection direction, float deltaTime)
{
    if (direction == FORWARD)
    {
        position += speed * deltaTime * forward;
    }
    else if (direction == BACKWARD)
    {
        position -= speed * deltaTime * forward;
    }
    else if (direction == LEFT)
    {
        // normalize the cross product so that movement speed is not dependent on forward which changes with rotation
        position -= glm::normalize(glm::cross(forward, up)) * speed * deltaTime;
    }
    else if (direction == RIGHT)
    {
        // normalize the cross product so that movement speed is not dependent on forward which changes with rotation
        position += glm::normalize(glm::cross(forward, up)) * speed * deltaTime;
    }
}

float Camera::getFov() const
{
    return fov;
}

const glm::vec3 &Camera::getFront() const
{
    return front;
}