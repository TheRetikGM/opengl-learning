#pragma once
#include <glm/glm.hpp>

enum camera_movement {
	CAM_FORWARD,
	CAM_BACKWARD,
	CAM_LEFT,
	CAM_RIGHT,
	CAM_UP,
	CAM_DOWN
};

enum camera_mode {
	CAM_FLOAT,
	CAM_MINECRAFT
};

class Camera
{
public:
	float FOV;
	float MovementSpeed;
	float MouseSensitivity;
	float Yaw;		// vlevo vpravo
	float Pitch;		// nahoru dolu

	camera_mode Mode;

	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Right;
	glm::vec3 Up;
	glm::vec3 WorldUp;
	glm::vec3 WorldRight;
	glm::vec3 WorldFront;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3 worldFront = glm::vec3(0.0f, 0.0f, -1.0),
		float yaw = -90.0f, float pitch = 0);	

	glm::mat4 getViewMatrix();
	void ProccessKeyboard(camera_movement direction, float deltaTime);
	void ProccessMouse(float xoffset, float yoffset, bool constrainPitch = true);
	void ProccessScroll(float yoffset);
	void SetCameraMode(camera_mode mode);

private:
	float lastY;
	float lastX;	

	void updateCameraVectors();
};

