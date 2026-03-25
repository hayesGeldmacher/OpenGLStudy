#pragma once
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

//simple struct for keeping GLints for seperate programs
struct ProgramInfo {

	GLuint programID;
	GLuint vbo;
	GLuint vao;
	GLint matLocation = 1;
	GLint worldTransformLocation;
	GLint viewTranformLocation;
	GLint projectionTransformLocation;
	GLuint texIDDiffuse = 0;
	//location of texture unit for specularity texture
	GLuint texIDSpec = 1;
	//location of texture unit for normal texture
	GLuint texIDNormal = 0;
	GLuint texIDDisplace = 2;


	//just for render-to-texture
	GLuint frameBuffer;
	GLuint renderedTexture;

	bool renderGeometryShader;
	bool renderTessellationsShader;

};
