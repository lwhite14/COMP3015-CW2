#ifndef CAMERA_H
#define CAMERA_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "motion.h"

#include <iostream>

class Camera
{
public:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;

	float deltaTime;
	float lastFrame;

	bool firstMouse;
	bool canFirstMouse;

	float yaw;
	float pitch;
	float lastX;
	float lastY;

	bool isActive;
	bool canChangeActive = true;

	Motion motion;

	Camera() {}

	Camera(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT)
	{
		isActive = true;

		deltaTime = 0.0f;
		lastFrame = 0.0f;

		firstMouse = true;
		canFirstMouse = true;
		yaw = -90.0f;
		pitch = 0.0f;
		lastX = SCR_WIDTH / 2.0;
		lastY = SCR_HEIGHT / 2.0;

		motion = { false,false,false,false };

		cameraPos = glm::vec3(0, 0, 0);
		cameraFront = glm::vec3(0.0, 0.0, 1.0);
		cameraUp = glm::vec3(0.0, 1.0, 0.0);
	}

	void UpdateDeltaTime() 
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}

	void KeyCallback(GLFWwindow* window)
	{
		int wState = glfwGetKey(window, GLFW_KEY_W);
		int sState = glfwGetKey(window, GLFW_KEY_S);
		int aState = glfwGetKey(window, GLFW_KEY_A);
		int dState = glfwGetKey(window, GLFW_KEY_D);
		int shiftState = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
		if (shiftState == GLFW_PRESS)
		{
			if (canChangeActive)
			{
				canChangeActive = false;
				canFirstMouse = true;
				if (isActive) 
				{
					isActive = false;
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				else if (!isActive) 
				{
					isActive = true;
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
			}
		}
		if (shiftState == GLFW_RELEASE)
		{
			if (canFirstMouse) 
			{
				canChangeActive = true;
				firstMouse = true;
			}
			canFirstMouse = false;
		}
		if (isActive) 
		{
			if (wState == GLFW_PRESS)
			{
				motion.Forward = true;
			}
			if (sState == GLFW_PRESS)
			{
				motion.Backward = true;
			}
			if (aState == GLFW_PRESS)
			{
				motion.Left = true;
			}
			if (dState == GLFW_PRESS)
			{
				motion.Right = true;
			}
			if (wState == GLFW_RELEASE)
			{
				motion.Forward = false;
			}
			if (sState == GLFW_RELEASE)
			{
				motion.Backward = false;
			}
			if (aState == GLFW_RELEASE)
			{
				motion.Left = false;
			}
			if (dState == GLFW_RELEASE)
			{
				motion.Right = false;
			}
		}
		else 
		{
			motion.Forward = false;
			motion.Backward = false;
			motion.Left = false;
			motion.Right = false;
		}
	}

	void Movement()
	{
		float cameraSpeed = 20.0f * deltaTime;
		//glm::vec3 frontAdjusted = glm::vec3(cameraFront.x, 0, cameraFront.z); 
		if (motion.Forward)
		{
			cameraPos += cameraSpeed * cameraFront;
		}
		if (motion.Backward)
		{
			cameraPos -= cameraSpeed * cameraFront;
		}
		if (motion.Left)
		{
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
		if (motion.Right)
		{
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
	}

	void MouseCallback(GLFWwindow* window)
	{
		if (isActive)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			if (firstMouse)
			{
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos;
			lastX = xpos;
			lastY = ypos;

			float sensitivity = 0.1f;
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			yaw += xoffset;
			pitch += yoffset;

			if (pitch > 89.0f)
			{
				pitch = 89.0f;
			}
			if (pitch < -89.0f)
			{
				pitch = -89.0f;
			}

			glm::vec3 front;
			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			cameraFront = glm::normalize(front);
		}
	}

	glm::mat4 ViewLookAt(glm::mat4 inputView) 
	{
		return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
};


#endif // CAMERA_H
