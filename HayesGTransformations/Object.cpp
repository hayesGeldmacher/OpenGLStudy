#include "Object.h"

//Hayes Geldmacher
//world transform class for CS 6610 project 2
//generates a transform matrix for a specified object
//centers object, applies rotation, scale, translation, before returning transformation matrix

Object::Object() {
}

Object::Object(const char* fileName) {
	this->objectFileName = fileName;
}


void Object::SetScale(float scale) {
	this->scale = scale;
}

void Object::SetRotation(float x, float y, float z) {
	this->rotation.x = x;
	this->rotation.y = y;
	this->rotation.z = z;
}

void Object::SetPosition(float x, float y, float z) {
	this->position.x = x;
	this->position.y = y;
	this->position.z = z;
}

void Object::Rotate(float x, float y, float z) {
	this->rotation.x += x;
	this->rotation.y += y;
	this->rotation.z += z;
}

glm::vec3 Object::GetPosition() {
	return position;
}

void Object::SetCenter(glm::vec3 center) {
	this->center = center;
}


glm::vec3 Object::GetColor() {
	return color;
}

void Object::SetColor(float x, float y, float z) {
	color = glm::vec3(x, y, z);
}

void Object::ToggleRotating() {
	rotating = !rotating;
	if (rotating) {
		std::cout << "object is now rotating" << std::endl;
	}
	else {
		std::cout << "object is no longer rotating" << std::endl;
	}
}

//defaults object to use vertex array plane instead of mesh
void Object::SetVertArrayToPlane() {

	useMesh = false;
}

glm::mat4 Object::GetMat() {

	//start with identity matrix
	glm::mat4 modelMat = glm::mat4(1.0f);

	//apply the world transformation
	modelMat = glm::translate(modelMat, position);

	//apply constant animating rotation
	float angleInRadians = glm::radians(rotation.x);
	modelMat = glm::rotate(modelMat, angleInRadians, glm::vec3(1.0f, 0.0f, 0.0f));

	angleInRadians = glm::radians(rotation.y);
	modelMat = glm::rotate(modelMat, angleInRadians, glm::vec3(0.0f, 1.0f, 0.0f));

	angleInRadians = glm::radians(rotation.z);
	modelMat = glm::rotate(modelMat, angleInRadians, glm::vec3(0.0f, 0.0f, 1.0f));

	//applay scaling
	modelMat = glm::scale(modelMat, glm::vec3(scale, scale, scale));

	//finally translation for local coordinates to center object
	modelMat = glm::translate(modelMat, glm::vec3(-center.x, -center.y, -center.z));

	return modelMat;
}