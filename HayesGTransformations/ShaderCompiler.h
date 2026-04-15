#pragma once
//standard libraries
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
//Glut/GL libaries
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>

class ShaderCompiler {
	void CompileShaders(const char*, const std::string&, GLuint&, GLuint&);
};