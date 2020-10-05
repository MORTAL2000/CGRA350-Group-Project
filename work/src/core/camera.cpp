#include "camera.hpp"

// glm
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// project
#include "opengl.hpp"

using namespace glm;

mat4 Camera::Update()
{
	// Update current frame
	float currentFrame = glfwGetTime();
	m_deltaTime = currentFrame - m_lastFrame;
	m_lastFrame = currentFrame;

	return lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}

void Camera::Rotate(double xpos, double ypos, vec2& windowSize)
{
	float xoffset = xpos - m_lastX;
	float yoffset = ypos - m_lastY;

	xoffset *= (sensitivity/100);
	yoffset *= (sensitivity/100);

	m_yaw += xoffset;
	m_pitch -= yoffset;

	// Lock pitch to avoid confusing rotations
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;

	m_lastX = windowSize.x / 2;
	m_lastY = windowSize.y / 2;

	m_cameraDirection.x = cos(radians(m_yaw)) * cos(radians(m_pitch));
	m_cameraDirection.z = sin(radians(m_yaw)) * cos(radians(m_pitch));
	m_cameraDirection.y = sin(radians(m_pitch));
	m_cameraFront = normalize(m_cameraDirection);
}

void Camera::Move(CameraMove dir)
{
	switch (dir)
	{
	case CameraMove::forward:
		m_cameraPos += (camera_speed * m_deltaTime) * m_cameraFront;
		break;
	case CameraMove::backward:
		m_cameraPos -= (camera_speed * m_deltaTime) * m_cameraFront;
		break;
	case CameraMove::left:
		m_cameraPos -= normalize(cross(m_cameraFront, m_cameraUp)) * (camera_speed * m_deltaTime);
		break;
	case CameraMove::right:
		m_cameraPos += normalize(cross(m_cameraFront, m_cameraUp)) * (camera_speed * m_deltaTime);
		break;
	default:
		break;
	}
}
