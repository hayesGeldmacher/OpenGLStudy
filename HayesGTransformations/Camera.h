#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

//Hayes Geldmacher
//camera class for CS 6610 project 2
//generates a view matrix, allows for both freelook and constrained control methods
//see instructions at top of main.cpp

class Camera {
public:
	
	//default constructor
	Camera();

	//elaborate constructor
	Camera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3 up);
	
	void OnKeyBoard(unsigned char key);

	void OnMouse(int button, int state, int x, int y);

	void OnMouseMotion(int x, int y);

	void UpdateRotation();

	glm::mat4 GetMatrix();

	glm::vec3 GetPosition();

	void ToggleFreeLook();

	void DetermineDistance();

	void SetTarget(const glm::vec3& target);

	void SetDeltaSpeed(float newSpeed);

	bool enabled = false;

	void SetEnabled(bool activate) {
		enabled = activate;
	}

	void Flip();

	void GetYPosition();

private:
	
	bool useFreeLook = false;
	
	//vec 3's for transformation
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraFront = glm::normalize(cameraPos - cameraTarget);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraFront));
	glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);
	glm::vec3 cameraDirection;


	glm::vec2 mousePos;

	//returned matrix for view matrix transformation
	glm::mat4 view;

	//movement speed
	float speed = 1.0f;

	//movement speed after making frame-independent
	float finalSpeed = 1.0f;

	//rotations for camera
	float yaw = -90.0f;
	float pitch = 0.0f;



	bool zooming = false;
	bool swinging = false;

	bool swingingLastFrame = false;
	bool zoomingLastFrame = false;

	//camera rotation speed
	float rotateSpeed = 1.0f;
	
	//zoom-related variables
	float zoomSpeed = 0.4f;
	float currentZoom = 30.0f;
	float maxZoom = 80.0f;
	float minZoom = 10.0f;


};