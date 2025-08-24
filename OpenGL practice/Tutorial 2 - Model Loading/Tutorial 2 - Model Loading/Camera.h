#pragma once

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f;

class Camera
{
private:
	glm::vec3 eye; //camera position
	glm::vec3 worldUp; //The world's up vector

	//camera's coordinate system axes
	glm::vec3 forward; //where the camera is looking at
	glm::vec3 up; //camera up vector
	glm::vec3 right; //camera right vector

	//Euler angles
	float yaw;
	float pitch;

	// other camera parameters
	float movementSpeed;
	float mouseSensitivity;
	float fov;

	//updates the axes of the camera's coordinate system using the updated Euler angles
	void updateAxes();

public:
	Camera(glm::vec3 eyeValue = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 worldUpValue = glm::vec3(0.0f, 1.0f, 0.0f),
		float yawValue = YAW, float pitchValue = PITCH);

	const glm::vec3& getEye() const { return eye; }
	const glm::vec3& getForward() const { return forward; }
	const glm::vec3& getUp() const { return up; }
	float getFOV() const { return fov; }


	/* returns the view matrix using the lookAt matrix, which takes in the
	eye (camera position) vector, target vector (eye + forward) and the camera up vector */
	glm::mat4 getViewMatrix() const { return glm::lookAt(eye, eye + forward, up); }

	//camera movement
	void moveForward(float distanceMoved);
	void moveBackward(float distanceMoved);
	void moveLeft(float distanceMoved);
	void moveRight(float distanceMoved);


	/** process mouse input
	*   parameters:
	*		xOffset, yOffset : Difference between current mouse position and last mouse position
	*		float constrainPitch : bool value for whether to constrain the pitch
	* */
	void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
	

	/**	processes mouse scroll-wheel input. 
	*	Parameters:
	*		yOffset: offset from vertical movement of the scroll
	* */
	void processMouseScroll(float yOffset);
};

