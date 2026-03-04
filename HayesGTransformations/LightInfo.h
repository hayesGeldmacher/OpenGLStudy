#include <glm/glm.hpp>
#include <iostream>
#pragma once

class LightInfo {

public:


	//default constructor
	LightInfo();

	//elaborate constructor
	LightInfo(glm::vec3 lightColor, glm::vec3 ambientColor, float ambientStrength);

	//sets the fields related to rotating and position the light
	void SetTransform(const glm::vec3& pos, const glm::vec3& target, const glm::vec3 up);

	//callback for registering key input
	void OnKeyBoard(unsigned char key);

	//callback for registering mouse input
	void OnMouse(int button, int state, int x, int y);

	//callback for registering active mouse motion
	void OnMouseMotion(int x, int y);

	//this is where the actual rotation, position of the light is changed
	void UpdateRotation();

	//returns light ambient color
	glm::vec3 GetAmbientColor();

	//returns current distance between light and object
	void DetermineDistance();

	//sets the target that the light rotates around
	void SetTarget(const glm::vec3& target);

	//sets frame-independent speed
	void SetDeltaSpeed(float newSpeed);

	//checks if user is pressing CTRL_L key before moving light
	void SetPressingButton(bool pressing);

	//sets the specularity shine of light
	void IncreaseShine(bool increase);

	//color-related fields
	float ambientStrength = 0.1f;
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);

	//specular shine fields
	float shine = 64.0f;
	float maxShine = 256.0f;
	float minShine = 8.0f;

	//index vaue for accessing below array
	int shineIndex;

	//array of possible shine values
	float shineValues[7] = {
		4.0f, 8.0f, 16.0f, 32.0f,
		64.0f, 128.0f, 256.0f
	};


	//position, control fields
	glm::vec3 lightPosition;
	//the point that the light rotates around
	glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);

	//vec 3's handling rotation and position
	glm::vec3 lightFront = glm::normalize(lightPosition - lightTarget);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 lightRight = glm::normalize(glm::cross(up, lightFront));
	glm::vec3 lightUp = glm::cross(lightFront, lightRight);
	glm::vec3 lightDirection;

	//vector two for tracking mouse position during input
	glm::vec2 mousePos;

	//rotation speed
	float speed = 1.0f;

	//rotation speed after making frame-independent
	float finalSpeed = 1.0f;

	//rotation values
	float yaw = -90.0f;
	float pitch = 0.0f;

	//tracks whether light should be swinging around the scene or not
	bool swinging = false;
	bool swingingLastFrame = false;

	bool zooming = false;
	bool zoomingLastFrame = false;

	bool pressingButton = false;

	//zoom-fields variables
	float zoomSpeed = 0.4f;
	float currentZoom = 100.0f;
	float maxZoom = 150.0f;
	float minZoom = 50.0f;



};