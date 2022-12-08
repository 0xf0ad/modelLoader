#pragma once

#include "libs/glad.h"
#include <assimp/material.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum cameraCmd {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	SLOW,
	FAST,
	GO_UP,
	GO_DOWN
};

// Default camera values
#define YAW          -90.0f
#define PITCH         0.00f
#define maxSPEED      07.5f
#define minSPEED      02.5f
#define SENSITIVITY   0.15f
#define ZOOM          60.0f


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera{
public:
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float maxSpeed = maxSPEED, minSpeed = minSPEED;
	float MouseSensitivity = SENSITIVITY;
	float Zoom = ZOOM;

	// constructor with vectors
	Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH){
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch){
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix(){
		return glm::lookAt(Position, Position + Front, Up);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(cameraCmd command, float deltaTime){
		float velocity = MovementSpeed * deltaTime;
		if (command == FORWARD)
			Position += Front * velocity;
		if (command == BACKWARD)
			Position -= Front * velocity;
		if (command == LEFT)
			Position -= Right * velocity;
		if (command == RIGHT)
			Position += Right * velocity;
		if (command == SLOW)
			MovementSpeed = minSpeed;
		if (command == FAST)
			MovementSpeed = maxSpeed;
		if (command == GO_UP)
			Position = WorldUp * velocity;
		if (command == GO_DOWN)
			Position = -WorldUp * velocity;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true){
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw   += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch){
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors(){
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up    = glm::normalize(glm::cross(Right, Front));
	}
};
