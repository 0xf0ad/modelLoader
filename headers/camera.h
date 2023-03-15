#pragma once

#include "libs/glad/glad.h"
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI      3.141592654f
#define HALF_PI 1.570796327f

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
#define YAW        -HALF_PI // -90.0 degrees
#define PITCH         0.00f
#define maxSPEED      07.5f
#define minSPEED      02.5f
#define SENSITIVITY   0.25f
#define FOV         HALF_PI //  90.0 degrees


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera{
public:

	// camera Attributes
	glm::vec3 mPosition;
	glm::vec3 mFrontDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 mUpDirection;
	glm::vec3 mRightDirection;
	glm::vec3 mWorldUp;

	// euler Angles
	float mYaw;
	float mPitch;

	// camera options
	float mMovementSpeed;
	float maxSpeed = maxSPEED, minSpeed = minSPEED;
	float mMouseSensitivity = SENSITIVITY;
	float mFieldOfView = FOV;
	float minPitch = (-HALF_PI) + 0.00001;
	float maxPitch =   HALF_PI  - 0.00001;

	// constructor with vectors
	Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
	       const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH){
		mPosition = position;
		mWorldUp = up;
		mYaw = yaw;
		mPitch = pitch;
		updateCameraVectors();
	}
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch){
		mPosition = glm::vec3(posX, posY, posZ);
		mWorldUp = glm::vec3(upX, upY, upZ);
		mYaw = yaw;
		mPitch = pitch;
		updateCameraVectors();
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	const glm::mat4 GetViewMatrix(){
		return glm::lookAt(mPosition, mPosition + mFrontDirection, mUpDirection);
	}

	// processes input received from any keyboard-like input system. Accepts input
	// parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(cameraCmd command, float deltaTime){
		float velocity = mMovementSpeed * deltaTime;
		if (command == FORWARD)
			mPosition += mFrontDirection * velocity;
		if (command == BACKWARD)
			mPosition -= mFrontDirection * velocity;
		if (command == LEFT)
			mPosition -= mRightDirection * velocity;
		if (command == RIGHT)
			mPosition += mRightDirection * velocity;
		if (command == SLOW)
			mMovementSpeed = minSpeed;
		if (command == FAST)
			mMovementSpeed = maxSpeed;
		if (command == GO_UP)
			mPosition += mUpDirection * velocity;
		if (command == GO_DOWN)
			mPosition -= mUpDirection * velocity;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(double xoffset, double yoffset, bool constrainPitch = true){
		xoffset *= mMouseSensitivity;
		yoffset *= mMouseSensitivity;

		mYaw   += glm::radians(xoffset);
		mPitch += glm::radians(yoffset);

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch){
			if (mPitch > maxPitch)	mPitch = maxPitch;
			if (mPitch < minPitch)	mPitch = minPitch;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors(){
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cosf(mYaw) * cosf(mPitch);
		front.y = sinf(mPitch);
		front.z = sinf(mYaw) * cosf(mPitch);
		mFrontDirection = glm::normalize(front);
		// also re-calculate the Right and Up vector
		mRightDirection = glm::normalize(glm::cross(mFrontDirection, mWorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		mUpDirection    = glm::normalize(glm::cross(mRightDirection, mFrontDirection));
	}
};
