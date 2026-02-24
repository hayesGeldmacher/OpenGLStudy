#include "Camera.h"
#include <iostream>


//Hayes Geldmacher
//camera class for CS 6610 project 2
//generates a view matrix, allows for both freelook and constrained control methods
//see instructions at top of main.cpp


//default constructor
Camera::Camera() {
	//initialize camera position to be back a few z units in sceen
	this->cameraPos = glm::vec3(0.0f, 0.0f, -3.0f);

	//initalize target to be straight forward
	this->cameraTarget = glm::vec3(0.0f, 0.0f, 1.0f);

	//intialize up to be world up 
	this->up = glm::vec3(0.0f, 1.0f, 0.0f);

	//call one update pass to initialize cam position
	UpdateRotation();
}

//elaborate constructor
Camera::Camera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3 up) {
	
	this->cameraPos = pos;
	this->cameraTarget = target;
	this->cameraTarget = glm::normalize(target);
	this->up = up;
	this->up = glm::normalize(up);

	//call one update pass to initialize cam position
	UpdateRotation();
}

//gets delta from main script, used for frame-independent camera movement
void Camera::SetDeltaSpeed(float newSpeed) {
	finalSpeed = newSpeed * speed;
}


glm::vec3 Camera::GetPosition() {
	return this->cameraPos;
}

//callback for mouse button presses
void Camera::OnMouse(int button, int state, int x, int y) {


	//if camera is disabled, do not register input
	if (!enabled) { return; }


	switch (button) {
	case 0:
		//when left mouse button pressed, we are swinging the camera
		if (state == 0) {
			swinging = true;
		}
		else {
			swinging = false;
		}
		break;
	
	case 2:
		//when right mouse button pressed, we are zooming the camera
		if (state == 0) {
			zooming = true;
		}
		else {
			zooming = false;
		}
		break;
	}

	//set the new mouse position
	mousePos.x = x;
	mousePos.y = y;
}

void Camera::GetYPosition() {
	std::cout << "Current Y Pos: " << cameraPos.y << std::endl;
}

//callback for mouse being moved, only active after a button press
void Camera::OnMouseMotion(int x, int y) {
		
	//if not zooming or swinging, no reason to be here
	if (!zooming && !swinging) { return; }

	//if camera is disabled, do not rotate
	if (!enabled) { return; }

		//get the difference in mouse position from last time we checked
		int deltaX = x - mousePos.x;
		int deltaY = y - mousePos.y;

		//if this is the first frame we started swinging camera, set deltas to 0 to avoid big camera jump
		if (swingingLastFrame) {
			deltaX = 0;
			deltaY = 0;
		}

		//set new mouse position
		mousePos.x = x;
		mousePos.y = y;

	
	if (swinging) {

		//if swinging, increase the camera pitch and yaw based on deltaTime
			pitch -= (float)deltaY * rotateSpeed;
			yaw += (float)deltaX * rotateSpeed;

			//clamp pitch so user can't spin in full vertical circle, disorientating
			if (pitch > 89.0f) {
				pitch = 89.0f;
			}
			if (pitch < -89.0f) {
				pitch = -89.0f;
			}	
	}
	else if (zooming) {
		
		//if in freelook mode, zooming simply moves the camera forward and back
		if (useFreeLook) {
			cameraPos += cameraFront * (float)deltaY;
		}
		else {
		//in constrained mode, zooming determines the radius of the camera around target object
			currentZoom += deltaY * zoomSpeed;
		//clamp zoom to avoid getting really close or far away
			if (currentZoom > maxZoom) { currentZoom = maxZoom; }
			if (currentZoom < minZoom) { currentZoom = minZoom; }
		}
	}

	UpdateRotation();
}

void Camera::Flip() {
	
	//camera was flipped!
	std::cout << "CAMERA WAS FLIPPED!" << std::endl;
	cameraPos.y = -cameraPos.y;
	cameraFront.y = -cameraFront.y;

	cameraRight = glm::normalize(glm::cross(cameraFront, up));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

	UpdateRotation();
}

void Camera::UpdateRotation() {
	

	//start with identity matrix
	view = glm::mat4(1.0f);

	//set camera direrction based on pitch and yaw
	cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraDirection.y = sin(glm::radians(pitch));
	cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	//normalize camera direction
	cameraFront = glm::normalize(cameraDirection);

	//use cross products to determine the right and up vectors for camera
	cameraRight = glm::normalize(glm::cross(cameraFront, up));
	cameraUp = glm::cross(cameraRight, cameraFront);

	//if freelook mode, just look straight ahead based on current direction
	if (useFreeLook) {

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
	//if constrained mode, always look at target object, with currentZoom being the radius
	else {
		cameraPos = cameraTarget - glm::normalize(cameraFront) * currentZoom;
		view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	}

}

//called when swapping between freelook and constrained to maintain smooth changes in distance from target object
void Camera::DetermineDistance() {
	float distance = glm::distance(cameraPos, cameraTarget);
	currentZoom = distance;
	UpdateRotation();
}

//called whenever a new target is selected for constrained mode
void Camera::SetTarget(const glm::vec3& target) {

	//if the target is new, set new zoom value and update rotation
	if (this->cameraTarget != target) {
		this->cameraTarget = target;
		std::cout << "Camera has new target!" << std::endl;
		float distance = glm::distance(cameraPos, cameraTarget);
		currentZoom = distance;
		UpdateRotation();
	}
}

//callbacks for keyboard presses
void Camera::OnKeyBoard(unsigned char key) {
	
	//press f to toggle between constrained and freelook modes
	if (!enabled) { return; }
	if (key == 'f') {
		ToggleFreeLook();
		DetermineDistance();
		UpdateRotation();
		return;
	}

	//below inputs are only to move camera in freelook mode
	if (!useFreeLook) { return; }

	switch (key) {
	case 'w':
		//move cam forward (Z)
		cameraPos += glm::normalize(finalSpeed * cameraFront);
		break;

	case 's':
		//move cam backward (-Z)
		cameraPos -= glm::normalize(finalSpeed * cameraFront);
		break;

	case 'a':
		//move cam left (-X)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp) * finalSpeed);
		break;

	case 'd':
		//move cam rigth (X)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp) * finalSpeed);
		break;

	case 'q':
		//move cam up (Y)
		cameraPos += glm::normalize(cameraUp * finalSpeed);
		break;

	case 'e':
		//move cam down(-Y)
		cameraPos -= glm::normalize(cameraUp * finalSpeed);
		break;

	case '+':
		//increase movement speed
		speed += 0.1f;
		std::cout << "Speed changed to: " << speed << std::endl;
		break;

	case '-':
		//decrease movement speed, clamp at 0.1
		speed -= 0.1f;
		if (speed < 0.1f) {
			speed = 0.1f;
		}
		std::cout << "Speed changed to: " << speed << std::endl;
		break;
	}

	UpdateRotation();
}

//swaps between freelook and constrained modes
void Camera::ToggleFreeLook() {
	useFreeLook = !useFreeLook;
	if (useFreeLook) {
		std::cout << "entered free look mode" << std::endl;
	}
	else {
		std::cout << "entered targeted look mode" << std::endl;
	}
}

//returns view matrix
glm::mat4 Camera::GetMatrix() {
		
	return view;
}
