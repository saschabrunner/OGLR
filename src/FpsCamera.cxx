#include "FpsCamera.h"

#include <glm/gtc/matrix_transform.hpp>

FpsCamera::~FpsCamera() {}

void FpsCamera::move(CameraDirection direction, float deltaTime)
{
    const glm::vec3 &cameraFront = getFront();

    // player can not fly, so don't allow any vertical movement
    glm::vec3 playerFront(cameraFront.x, 0.0f, cameraFront.z);

    moveInternal(playerFront, direction, deltaTime);
}