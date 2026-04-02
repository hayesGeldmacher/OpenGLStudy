#pragma once
#pragma once
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
//cem yuksul libraries
#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyGL.h"
#include "Vertex.h"
#include "Material.h"
//custom include

//Hayes Geldmacher
//world transform class for CS 6610 project 2
//generates a transform matrix for a specified object
//centers object, applies rotation, scale, translation, before returning transformation matrix

class Object {
public:
	//default constructor
	Object();

	//fileName contsructor
	Object(const char* fileName);

	//sets uniform scale value
	void SetScale(float scale);

	//sets rotation vec3
	void SetRotation(float x, float y, float z);

	//sets position vec3
	void SetPosition(float x, float y, float z);

	//adds to current rotation vec3
	void Rotate(float x, float y, float z);

	//stores the center of object using bounding box
	void SetCenter(glm::vec3);

	//returns object world position
	glm::vec3 GetPosition();

	//returns color of object
	glm::vec3 GetColor();

	void SetColor(float x, float y, float z);

	void InitializeObject();

	//returns matrix for world transformations
	glm::mat4 GetMat();


	//toggles idle rotations for object
	void ToggleRotating();
	bool rotating = false;

	//stores the number of faces in the object
	int facesNumber = 0;

	//the mesh this object is loading
	cy::TriMesh* mesh;

	Material* mat; 

	const char* objectFileName;


	void SetVertArrayToPlane();

	//should this object use a cy::mesh or a vert array for its buffer
	bool useMesh = true;
	//defaults to an array of planes


	float scale = 1.0f;
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 center = glm::vec3(0.f, 0.0f, 0.0f);

	//stores color
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);


	float planeArrayFlipped[48] = {

		//vertex position	   //normals			//tex coordinates
		 5.0f,  0.0f, 5.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
		 5.0f, 0.0f, -5.0f,	   0.0f, 1.0f, 0.0f,	 1.0f, 0.0f,
		-5.0f, 0.0f, -5.0f,	   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,

		-5.0f,  0.0f, -5.0f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
		-5.0f,  0.0f, 5.0f,	   0.0f, 1.0f, 0.0f,	 0.0f, 1.0f,
		 5.0f,  0.0f, 5.0f,	   0.0f, 1.0f, 0.0f,	 1.0f, 1.0f
	};


	//vector of vertex structs, stores pos, normal, and coord data in loaded mesh
	static std::vector<Vertex> vertexData;

	int faceNumber = 0;

};