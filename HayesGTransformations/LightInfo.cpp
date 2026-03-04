#include "LightInfo.h"




LightInfo::LightInfo() {

}


LightInfo::LightInfo(glm::vec3 lightColor, glm::vec3 ambientColor, float ambientStrength) {
	this->lightColor = lightColor;
	this->ambientColor = ambientColor;
	this->ambientStrength = ambientStrength;
}

void LightInfo::SetTransform(const glm::vec3& pos, const glm::vec3& target, const glm::vec3 up) {
	this->lightPosition = pos;
	this->lightDirection = target;
	this->lightTarget = glm::normalize(target);
	this->up = up;
	this->up = glm::normalize(up);

	//call one update pass to initialize cam position
	UpdateRotation();
}


//gets delta from main script, used for frame-independent camera movement
void LightInfo::SetDeltaSpeed(float newSpeed) {
	finalSpeed = newSpeed * speed;
}


//callback for mouse button presses
void LightInfo::OnMouse(int button, int state, int x, int y) {

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



//callback for mouse being moved, only active after a button press
void LightInfo::OnMouseMotion(int x, int y) {




	//if not zooming or swinging, no reason to be here
	if (!zooming && !swinging) { return; }

	if (!pressingButton) { return; }


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
		pitch += (float)deltaY * speed;
		yaw -= (float)deltaX * speed;

		//clamp pitch so user can't spin in full vertical circle, disorientating
		if (pitch > 80.0f) {
			pitch = 80.0f;
		}
		if (pitch < -80.0f) {
			pitch = -80.0f;
		}
	}
	else if (zooming) {

		//in constrained mode, zooming determines the radius of the camera around target object
		currentZoom += deltaY * zoomSpeed;
		//clamp zoom to avoid getting really close or far away
		if (currentZoom > maxZoom) { currentZoom = maxZoom; }
		if (currentZoom < minZoom) { currentZoom = minZoom; }
	}
	UpdateRotation();
}


//set shininess of specularity based on user input
void LightInfo::IncreaseShine(bool increase) {
	
		
		if (increase) {
			shineIndex++;
		}
		else {
			shineIndex -= 1;
		}
		int indexLength = sizeof(shineValues) / sizeof(shineValues[0]);

		if (shineIndex > indexLength - 1) {
			shineIndex = indexLength - 1;
		}
		if (shineIndex < 0) {
			shineIndex = 0;
		}

		shine = shineValues[shineIndex];

		std::cout << " spec shine to: " << shine << std::endl;
	
}

void LightInfo::OnKeyBoard(unsigned char key){

	if (key == 'x') {
		IncreaseShine(true);
	}
	else if(key == 'z') {
		IncreaseShine(false);
	}
}

void LightInfo::UpdateRotation() {

	lightDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	lightDirection.y = sin(glm::radians(pitch));
	lightDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	//normalize camera direction
	lightFront = glm::normalize(lightDirection);

	//use cross products to determine the right and up vectors for camera
	lightRight = glm::normalize(glm::cross(lightFront, up));
	lightUp = glm::cross(lightRight, lightFront);


	lightPosition = lightTarget - glm::normalize(lightDirection) * currentZoom;
}


void LightInfo::SetPressingButton(bool pressing) {
	pressingButton = !pressingButton;
	if (pressingButton) {
		std::cout << "rotating light" << std::endl;
	}
	else {
		std::cout << "no longer rotating light!" << std::endl;
	}
}


glm::vec3 LightInfo::GetAmbientColor() {
	return ambientColor * ambientStrength;
}