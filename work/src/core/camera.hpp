#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"

enum class CameraMove {
	forward, backward, left, right
};


class Camera {
private:
	glm::vec3 m_cameraPos = glm::vec3(-15.0f, 150.0f, 200.0f);
	glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_cameraDirection;

	float m_pitch = 0;
	float m_yaw = -90;

	float m_lastX = 400;
	float m_lastY = 300;

	float m_xOffset = 0;
	float m_yOffset = 0;

	float m_deltaTime = 0.0f;
	float m_lastFrame = 0.0f;
public:
	float sensitivity = 10.0f;
	float camera_speed = 100.0f;

	glm::mat4 Update();
	void Rotate(double xpos, double ypos, glm::vec2& windowSize);
	void Move(CameraMove dir);
};