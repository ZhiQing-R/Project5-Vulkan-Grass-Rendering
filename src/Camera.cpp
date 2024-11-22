#include <iostream>

#define GLM_FORCE_RADIANS
// Use Vulkan depth range of 0.0 to 1.0 instead of OpenGL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "BufferUtils.h"

Camera::Camera(Device* device, float aspectRatio) : device(device) {
    r = 10.0f;
    theta = 00.0f;
    phi = 0.0f;
	glm::vec3 eye = glm::vec3(0.0f, 30.0f, 50.0f);
	center = glm::vec3(100.0f, 10.0f, 100.0f);
	r = glm::distance(eye, center);
	theta = glm::degrees(glm::atan((eye.z - center.z) / (eye.x - center.x))) + 180;
	phi = -glm::degrees(glm::asin((eye.y - center.y) / r));

    cameraBufferObject.viewMatrix = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
    cameraBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 500.0f);
    cameraBufferObject.projectionMatrix[1][1] *= -1; // y-coordinate is flipped
	cameraBufferObject.viewMatrixInverse = glm::inverse(cameraBufferObject.viewMatrix);
	//cameraBufferObject.projectionMatrixInverse = glm::inverse(glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 500.0f));
    cameraBufferObject.projectionMatrixInverse = glm::inverse(cameraBufferObject.projectionMatrix);
    cameraBufferObject.eye = glm::vec4(eye, 1.f);

    BufferUtils::CreateBuffer(device, sizeof(CameraBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
    vkMapMemory(device->GetVkDevice(), bufferMemory, 0, sizeof(CameraBufferObject), 0, &mappedData);
    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

VkBuffer Camera::GetBuffer() const {
    return buffer;
}

void Camera::UpdateOrbit(float deltaX, float deltaY, float deltaZ) {
    theta += deltaX;
    phi += deltaY;
    r = glm::clamp(r - deltaZ, 1.0f, 400.0f);

    float radTheta = glm::radians(theta);
    float radPhi = glm::radians(phi);

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radTheta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), radPhi, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 finalTransform = glm::translate(glm::mat4(1.0f), center) * 
        rotation * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, r));
    cameraBufferObject.eye = finalTransform[3];
    cameraBufferObject.viewMatrix = glm::inverse(finalTransform);
    cameraBufferObject.viewMatrixInverse = glm::inverse(cameraBufferObject.viewMatrix);

    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::UpdatePosition(float deltaX, float deltaY, float deltaZ)
{
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 forward = glm::normalize(center - glm::vec3(cameraBufferObject.eye));
	glm::vec3 right = glm::normalize(glm::cross(forward, up));
	up = glm::cross(right, forward);

	glm::vec3 eye = glm::vec3(cameraBufferObject.eye);
    glm::vec3 offset = deltaX * right + deltaY * up;;
	eye += offset;
	center += offset;
	cameraBufferObject.eye = glm::vec4(eye, 1.0f);
	cameraBufferObject.viewMatrix = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
    cameraBufferObject.viewMatrixInverse = glm::inverse(cameraBufferObject.viewMatrix);

	memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

Camera::~Camera() {
  vkUnmapMemory(device->GetVkDevice(), bufferMemory);
  vkDestroyBuffer(device->GetVkDevice(), buffer, nullptr);
  vkFreeMemory(device->GetVkDevice(), bufferMemory, nullptr);
}
