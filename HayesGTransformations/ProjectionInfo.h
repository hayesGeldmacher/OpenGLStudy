

#define _CRT_SECURE_NO_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
//Glut/GL libaries
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//Additional Standard libraries
#include <stdlib.h>
#include <iostream>
//Custom include files
#include <glm/gtc/matrix_transform.hpp>


//struct, generates perpective and orthographic matrices
struct persProj {
    
    //standard contsructor
    persProj(int height, int width) {
        this->height = height;
        this->width = width;
        ar = (float)width / (float)height;
    }
    
    
    glm::mat4 GetProjection() {

        glm::mat4 proj;
        if (usePerspective) {
            proj = glm::perspective(glm::radians(FOV), (float)width / (float)height, nearZ, farZ);
        }
        else {
            proj = glm::ortho(-orthoSize * ar, orthoSize * ar, -orthoSize, orthoSize, 0.1f, 100.0f);

        }
        return proj;

    }

    void ToggleProjection() {
        if (usePerspective) {
            usePerspective = false;
            std::cout << "Switched to orthographic projection" << std::endl;
        }
        else {
            usePerspective = true;
            std::cout << "Switched to perspective projection" << std::endl;
        }
    }

    
    int width;
    int height;
    float orthoSize = 25.0f;
    float ar;
    float FOV = 90.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;
    bool usePerspective = true;
};