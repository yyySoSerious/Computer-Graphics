#include "Camera.h"

Camera::Camera(glm::vec3 eyeValue, glm::vec3 worldUpValue, float yawValue, float pitchValue) :
    eye(eyeValue), worldUp(worldUpValue), forward(glm::vec3(0.0f, 0.0f, -1.0f)), up(worldUp),
    yaw(yawValue), pitch(pitchValue),
    movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), fov(FOV) {

    updateAxes();
}


void Camera::moveForward(float distanceMoved) {
	eye += distanceMoved * forward;
   // eye.y = 0.0f; //immitating fps
}

void Camera::moveBackward(float distanceMoved) {
	eye -= distanceMoved * forward;
   // eye.y = 0.0f; //immitating fps
}

void Camera::moveLeft(float distanecMoved) {
	glm::vec3 cameraRight = glm::normalize(glm::cross(forward, up));
	eye -= distanecMoved * cameraRight;
}

void Camera::moveRight(float distanecMoved) {
	glm::vec3 cameraRight = glm::normalize(glm::cross(forward, up));
	eye += distanecMoved * cameraRight;
}

/** process mouse input
*   parameters:
*		xOffset, yOffset : Difference between current mouse position and last mouse position
*		float constrainPitch : bool value for whether to constrain the pitch
* */
void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch){
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateAxes();
}

/**	processes mouse scroll-wheel input.
    *	Parameters:
    *		yOffset: offset from vertical movement of the scroll
    * */
void Camera::processMouseScroll(float yOffset){
    fov -= (float)yOffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

//updates the axes of the camera's coordinate system using the updated Euler angles
void Camera::updateAxes() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    forward = glm::normalize(direction); 
    right = glm::normalize(glm::cross(forward, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    up = glm::normalize(glm::cross(right, forward));
}
