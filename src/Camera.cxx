#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 lookAt(glm::vec3 cameraPosition, glm::vec3 target, glm::vec3 worldUp);

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
      front(getFront(pitch, yaw)),
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
      front(getFront(pitch, yaw)),
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

Camera::~Camera() {}

glm::mat4 Camera::calculateView() const
{
    // the view transformation matrix is calculated by
    // 1. subtracting the target vector we're looking at from the camera position (cameraPos - cameraTarget)
    //      this will result in a vector that points in the opposite direction of where the camera is pointing at
    // 2. calculating the cross product of a vector pointing straight up in world space and the direction vector from step 1.
    //      this will result in a vector that's orthogonal to both input vectors, which happens to be the right axis of the camera
    // 3. calculating the cross product of the direction vector from step 1. and the right axis vector from step 2.
    //      this will result in another vector orthogonal to both inputs, which is the up axis of the camera
    // 4. calculate the view matrix by putting the camera directions in a rotation matrix and multiplying it with a
    //    translation matrix (lookAt = rotation * translation) - note the order of the multiplication is different!
    // glm::lookAt will do all of that for us, by just providing the initial three vectors: camera position, target, up
    // return glm::lookAt(position, position + front, up);
    return lookAt(position, position + front, up);
}

/**
 * Exemplary manual implementation, imitating glm::lookAt
 */
glm::mat4 lookAt(glm::vec3 cameraPosition, glm::vec3 target, glm::vec3 worldUp)
{
    // 1. subtracting the target vector we're looking at from the camera position
    glm::vec3 cameraFront = glm::normalize(cameraPosition - target);

    // 2. calculating the cross product of a vector pointing straight up in world space and the direction vector from step 1.
    glm::vec3 cameraRight = glm::normalize(glm::cross(glm::normalize(worldUp), cameraFront));

    // 3. calculating the cross product of the direction vector from step 1. and the right axis vector from step 2.
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraFront, cameraRight));

    // 4.1. rotation matrix
    // the direction vectors can be directly placed inside the rotation matrix
    glm::mat4 rotationMatrix(1.0f);
    rotationMatrix[0][0] = cameraRight.x;
    rotationMatrix[1][0] = cameraRight.y;
    rotationMatrix[2][0] = cameraRight.z;
    rotationMatrix[0][1] = cameraUp.x;
    rotationMatrix[1][1] = cameraUp.y;
    rotationMatrix[2][1] = cameraUp.z;
    rotationMatrix[0][2] = cameraFront.x;
    rotationMatrix[1][2] = cameraFront.y;
    rotationMatrix[2][2] = cameraFront.z;

    // 4.2. translation matrix
    // we move things away from the camera, so the values need to be negated
    glm::mat4 translationMatrix(1.0f);
    translationMatrix[3][0] = -cameraPosition.x;
    translationMatrix[3][1] = -cameraPosition.y;
    translationMatrix[3][2] = -cameraPosition.z;

    // 4.3. view matrix = rotation * translation
    return rotationMatrix * translationMatrix;
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
    front = getFront(pitch, yaw);
}

void Camera::move(CameraDirection direction, float deltaTime)
{
    moveInternal(front, direction, deltaTime);
}

void Camera::moveInternal(const glm::vec3 &forward, CameraDirection direction, float deltaTime)
{
    if (direction == CameraDirection::FORWARD)
    {
        position += speed * deltaTime * forward;
    }
    else if (direction == CameraDirection::BACKWARD)
    {
        position -= speed * deltaTime * forward;
    }
    else if (direction == CameraDirection::LEFT)
    {
        // normalize the cross product so that movement speed is not dependent on forward which changes with rotation
        position -= glm::normalize(glm::cross(forward, up)) * speed * deltaTime;
    }
    else if (direction == CameraDirection::RIGHT)
    {
        // normalize the cross product so that movement speed is not dependent on forward which changes with rotation
        position += glm::normalize(glm::cross(forward, up)) * speed * deltaTime;
    }
}

float Camera::getFov() const
{
    return fov;
}

const glm::vec3 &Camera::getPosition() const
{
    return position;
}

const glm::vec3 &Camera::getFront() const
{
    return front;
}

glm::vec3 Camera::getFront(float pitch, float yaw)
{
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return glm::normalize(direction);
}