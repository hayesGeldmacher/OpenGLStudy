#pragma once
//standard libraries
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <random>
//Glut/GL libaries
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
//custom includes
#include "ProgramInfo.h"
#include "cyTriMesh.h"
#include "ShaderCompiler.h"


class ShaderCompiler {
public:
	void CompileShaders(const char*, const std::string&, GLuint&, GLuint&);

	void CreateSSAOBuffer(int, int);

	void CreateDeferredBuffer(int, int);

	void CreateKernal();

	void CreateBuffers(ProgramInfo&, cy::TriMesh&);

	void InitializeObject(cy::TriMesh& mesh, Object& object);

	cy::TriMesh LoadObjectFile(const char*);

	float Lerp(float, float, float);

	unsigned int gBuffer; //frame buffer object for deferred rendering
	unsigned int gPosition, gNormal, gColorSpec, depthMap; //stores fragment info for deferred rendering
	unsigned int AOFBO; //frame buffer objects for ssao buffer
	unsigned int AOColorBuffer; //color buffer for storing occlusion information
	unsigned int noiseTexture; //noise texture for tiling over screen with occlusion
	unsigned int AOBlurFBO, AOColorBufferBlur; //frame buffer object for blurring AO
	int kernelNumber = 64;
	std::vector<glm::vec3> kernel; //list of kernel samples to send to SSAO.frag shader
	
};