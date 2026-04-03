#pragma once
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Object.h"

//simple struct for keeping GLints for seperate programs
struct ProgramInfo {

	ProgramInfo(){}
	ProgramInfo(Object* objectRef) {
		object = objectRef;
	}
	
	GLuint programID;
	GLuint vbo;
	GLuint vao;
	GLint matLocation = 1;
	GLint ObjectLocation;
	GLint viewTranformLocation;
	GLint projectionTransformLocation;
	GLuint texIDDiffuse = 0;
	//location of texture unit for specularity texture
	GLuint texIDSpec = 1;

	//just for render-to-texture
	GLuint frameBuffer;
	GLuint rendereddTexture;

	Object* object; 

};

//struct for containing shadow and depth map information
struct ShadowInfo {
	//the frame buffer object for the shadows
	unsigned int depthMapFBO;
	//the resolution for the shadows
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	//the texture for the shadows
	unsigned int depthMap;
};
