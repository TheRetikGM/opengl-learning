#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 worldUp, glm::vec3 worldFront, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), FOV(45.0f), Mode(CAM_FLOAT)
{
	this->Position = position;
	this->WorldUp = worldUp;
	this->Yaw = yaw;
	this->Pitch = pitch;
	this->WorldFront = worldFront;
	this->WorldRight = glm::normalize(glm::cross(this->WorldFront, this->WorldUp));

	updateCameraVectors();
}
void Camera::updateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);

	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));	
}
glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(Position, Position + Front, WorldUp);
}
void Camera::ProccessKeyboard(camera_movement direction, float deltaTime)
{
	switch (direction)
	{
	case CAM_FORWARD:
		Position += deltaTime * MovementSpeed * Front;
		break;
	case CAM_BACKWARD:
		Position -= deltaTime * MovementSpeed * Front;
		break;
	case CAM_LEFT:
		Position -= deltaTime * MovementSpeed * Right;
		break;
	case CAM_RIGHT:
		Position += deltaTime * MovementSpeed * Right;
		break;
	case CAM_UP:
		Position += deltaTime * MovementSpeed * WorldUp;
		break;
	case CAM_DOWN:
		Position -= deltaTime * MovementSpeed * WorldUp;
		break;
	}
}
void Camera::ProccessMouse(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;
		
	Yaw = fmodf(Yaw + xoffset, 360.0f);	
	Pitch += yoffset;

	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		else if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	updateCameraVectors();
}
void Camera::ProccessScroll(float yoffset)
{
	FOV -= yoffset;
	if (FOV < 1.0f)
		FOV = 1.0f;
	else if (FOV > 45.0f)
		FOV = 45.0f;
}
void Camera::SetCameraMode(camera_mode mode)
{
	Mode = mode;
}