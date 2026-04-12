
#define _CRT_SECURE_NO_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#include "stb_image.h"
//Glut/GL libaries
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
//Additional Standard libraries
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <random>
//cem yuksul libraries
#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyGL.h"
//Custom include files
#include "Vertex.h"
#include "Camera.h"
#include "LightInfo.h"
#include "ProgramInfo.h"
#include "ColorInfo.h"
#include "ImageLoader.h"
#include "Object.h"
#include "Material.h"
//Hayes Geldmacher - 4/3/26
//CS 6610 - Final Project WIP

//Instructions/controls:
/*
   
    press 'p' to toggle perspective/orthographic projection
    press 'f' to toggle freelook/constrained camera viewing modes

    when in constrained viewing mode:
        hold the left mouse button and drag to rotate around the teapot object
        hold the right mouse button and drag to zoom in or out of the teapot object

    when in freelook viewing mode:
        hold the left mouse button and drag to rotate the camera, like in an FPS game
        hold the right mouse button and drag to move the camera back or forth on the Z axis
        press/hold 'w', 's', 'a', 'd', to move forward, back, left, and right respectively
        press/hold 'q', 'e', to move up or down respectively
        press '+', '-', to increase or decrease the movement speed

    hold and release 'left alt' to switch active camera between teapot and render plane cameras

    hold and release 'let ctrl' to switch between camera mode and light rotation mode
        when in light rotation mode, moving the mouse will not spin the active camera
        Instead, it will rotate the light diriection 

    press 'z' and 'x' to increase or decrease the specularity power of the light, respectively 
    press 'r' to toggle the teapot and render plane idly spinning on all axes
    press 'f6' to recompile shaders

*/

//screen width and height
const static int width = 800;
const static int height = 800;

//declare objects and programs
#pragma region objectInformation
//instance of world object transform class for the screenspace render plane
Object planeObject;

//instance of world object transform class, generates transformation matrix
Object teapotObject("teapotReflection.obj");
ProgramInfo teapotInfo(&teapotObject);

Object teapotObjectSecond("teapotReflection.obj");
ProgramInfo teapotSecondInfo(&teapotObjectSecond);
//teapot mesh being loaded in
cy::TriMesh teapotMesh;

Object quadObject("PlaneMesh.obj");
ProgramInfo quadInfo(&quadObject);
//mesh used to load plane
cy::TriMesh quadMesh;

Object wallObject("PlaneMesh.obj");
ProgramInfo wallInfo(&wallObject);

//program  info for the model displaying the light
Object cubeObject("cube.obj");
ProgramInfo lightModelInfo(&cubeObject);
//mesh used to load light model
cy::TriMesh lightMesh;

#pragma endregion objectInformation

#pragma region shadowInformation

//struct containing shadow & depth map information
ShadowInfo shadowInfo;

//program info for the teapot rendered into lightview to generate shadow textures
ProgramInfo teapotInfoShadow;

//program info for plane rendered with shadows
ProgramInfo planeInfo;

//program info for the plane rendered into lightview to generate shadow textures
ProgramInfo planeInfoShadow;

//program info for testing the depth display on a plane
ProgramInfo depthDisplayInfo;

//program info for second teapot second pass using shadow map
ProgramInfo teapotSecondShadow;

//program info getting SSAO to the screen
ProgramInfo screenPlaneInfo;

ProgramInfo blurPlaneInfo;

//program info for getting final render to the scren
ProgramInfo renderPlaneInfo;

//the render buffer used for render-to-texture
cy::GLRenderTexture2D renderBuffer;
ProgramInfo quadInfoShadow;
#pragma endregion shadowInformation

#pragma region lightInformation
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
float ambientLightIntensity = 0.5f;
static LightInfo lightInfo(lightColor, ambientColor, ambientLightIntensity);
#pragma endregion lightInformation

#pragma region cameraInformation
glm::vec3 camPos(0.0f, 0.0f, 5.0f);
glm::vec3 camTarget(2.0f, 0.0f, -5.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

//instace of camera class, generates view matrix
Camera camera(camPos, camTarget, camUp);

//instance of plane-specific camera 
Camera planeCamera(camPos, camTarget, camUp);

#pragma endregion cameraInformation

#pragma region animationInformation
//create a color for R,G,B, and store in an array
Color red(0.5, true, true);
Color blue(1, false, true);
Color green(0.2, false, true);

Color* colors[] = { &red, &blue, &green };
glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);

//animation time-tracking
float currentTime = 0;
float previousTime = 0;
float animateSpeed = 0.05f;
//float used for idle animations of mesh teapot
static float angleInRadians = 0.0f;

#pragma endregion animationInformation

#pragma region deferredRenderingInformation
//deferred shading fields
unsigned int gBuffer;
unsigned int gPosition, gNormal, gColorSpec, depthMap;

std::vector<ProgramInfo*> drawObjects;
#pragma endregion deferredRenderingInformation

//whether to render the screenspace quad for deferred rendering
bool renderScreenSpace = true;

//whether to render ambient occlusion
bool renderAO = true;

//struct, generates perpective and orthographic matrices
struct persProj {
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

    float orthoSize = 25.0f;
    float ar = (float)width / (float)height;
    float FOV = 90.0f;
    float nearZ = 0.1f;
    float farZ = 1000.0f;
    bool usePerspective = true;
};
//instance of projection info struct
static persProj projInfo;
//projection info for light matrix
static persProj lightProjection;

//image loader object for loading texture data from image files
ImageLoader imageLoader;

unsigned int AOFBO; //frame buffer objects for ssao buffer
unsigned int AOColorBuffer; //color buffer for storing occlusion information
unsigned int noiseTexture; //noise texture for tiling over screen with occlusion
unsigned int AOBlurFBO, AOColorBufferBlur; //frame buffer object for blurring AO
std::vector<glm::vec3> kernel; //list of kernel samples to send to SSAO.frag shader


//called during onDisplay, sets all uniform shader variables
void SetUniformAttributesLighting(GLuint &program, Camera &camera) {

    GLint uniformLocation;

    //update the uniform ambient strength in the frag shader
    uniformLocation = glGetUniformLocation(program, "ambientStrength");
    glUniform1f(uniformLocation, lightInfo.ambientStrength);

    //update the uniform light color in the frag shader
    glm::vec3 lightColor = lightInfo.lightColor;
    uniformLocation = glGetUniformLocation(program, "lightColor");
    glUniform3f(uniformLocation, lightColor.x, lightColor.y, lightColor.z);

    glm::vec3 lightPosition = lightInfo.lightPosition;
    uniformLocation = glGetUniformLocation(program, "lightPosition");
    glUniform3f(uniformLocation, lightPosition.x, lightPosition.y, lightPosition.z);

    glm::vec3 viewPos = camera.GetPosition();
    uniformLocation = glGetUniformLocation(program, "viewPos");
    glUniform3f(uniformLocation, viewPos.x, viewPos.y, viewPos.z);

    //set reflection very high for now
    uniformLocation = glGetUniformLocation(program, "reflectiveStrength");
    glUniform1f(uniformLocation, 0.65f);

}

//sets MVP-related uniform variables
void SetUniformAttributesTransformations(ProgramInfo &programInfo, Camera &camera, bool flipped) {
    
    //generate view matrix from camera
    glm::mat4 camViewMat = camera.GetMatrix();
    glm::mat4 worldMatrix = programInfo.object->GetMat();
    
    //generate perpsective/ortho projection matrix
    glm::mat4 projMat = projInfo.GetProjection();

    //flip the camera on the Y axis for planar reflections
    if (flipped) {
        
        camViewMat = camViewMat * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
    }

    GLint uniformLocation;
    //send the world transform variable
    uniformLocation = glGetUniformLocation(programInfo.programID, "world");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &worldMatrix[0][0]);

    //send the camera view variable
    uniformLocation = glGetUniformLocation(programInfo.programID, "view");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &camViewMat[0][0]);

    //send the projection variable
    uniformLocation = glGetUniformLocation(programInfo.programID, "projection");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &projMat[0][0]);
}

//generate perpsective projection matrix for object-to-light transformation
glm::mat4 GetLightMatrix() {
    glm::mat4 projMat = lightProjection.GetProjection();
    glm::vec3 lightPos = lightInfo.lightPosition;
    glm::mat4 lightView = glm::lookAt(lightPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 lightMat = projMat * lightView;
    return lightMat;
}

//renders mesh objects from start to finish
void RenderMeshObject(ProgramInfo &programInfo, Camera &camera, bool useTextures, bool useLight) {
    
    //use the desired shader program
    glUseProgram(programInfo.programID);
    glBindVertexArray(programInfo.vao);

    //get the object-to-light transform matrix
        glm::mat4 lightMat = GetLightMatrix();
        GLint uniformLocation;
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "lightMat");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &lightMat[0][0]);

        glm::mat4 worldMatrix = programInfo.object->GetMat();
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "world");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &worldMatrix[0][0]);

        uniformLocation = glGetUniformLocation(programInfo.programID, "specShine");
        glUniform1f(uniformLocation, lightInfo.shine);

        glm::vec3 objectColor = programInfo.object->GetColor();
        uniformLocation = glGetUniformLocation(programInfo.programID, "objectColor");
        glUniform3f(uniformLocation, objectColor.x, objectColor.y, objectColor.z);
 
        //if not just for depth map, assign general uniform attributes
        SetUniformAttributesTransformations(programInfo, camera, false);

    if(useLight){
        //set uniform lighting atttributes
        SetUniformAttributesLighting(programInfo.programID, camera);
    }

    if (useTextures) {
        glBindTexture(GL_TEXTURE_2D, programInfo.texIDDiffuse);
    }

    glDrawArrays(GL_TRIANGLES, 0, programInfo.object->facesNumber);
}

void SetDeferredLighting(ProgramInfo& programInfo, Camera& camera) {
    
    //set uniform attributes
    GLint uniformLocation;

    //update the uniform light color in the frag shader
    glm::vec3 lightColor = lightInfo.lightColor;
    uniformLocation = glGetUniformLocation(screenPlaneInfo.programID, "lightColor");
    glUniform3f(uniformLocation, lightColor.x, lightColor.y, lightColor.z);

    //update uniform specular shininess exponent in the frag shader
    uniformLocation = glGetUniformLocation(screenPlaneInfo.programID, "specShine");
    glUniform1f(uniformLocation, lightInfo.shine);

    glm::vec3 lightPosition = lightInfo.lightPosition;
    uniformLocation = glGetUniformLocation(screenPlaneInfo.programID, "lightPosition");
    glUniform3f(uniformLocation, lightPosition.x, lightPosition.y, lightPosition.z);

    glm::vec3 viewPos = camera.GetPosition();
    uniformLocation = glGetUniformLocation(screenPlaneInfo.programID, "viewPos");
    glUniform3f(uniformLocation, viewPos.x, viewPos.y, viewPos.z);

}

//render screenspace plane for deferred rendering
void RenderScreenSpacePlane(ProgramInfo& programInfo, bool includeNoiseTexture, bool includeAOTexture) {
   
    glUseProgram(programInfo.programID);
    SetDeferredLighting(programInfo, camera);

    glm::mat4 projMat = projInfo.GetProjection();
    GLuint projectionLocation = glGetUniformLocation(programInfo.programID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projMat[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(programInfo.programID, "gPosition"), 0);
    glBindTexture(GL_TEXTURE_2D, gPosition);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(programInfo.programID, "gColorSpec"), 1);
    glBindTexture(GL_TEXTURE_2D, gColorSpec);

    glActiveTexture(GL_TEXTURE2);
    glUniform1i(glGetUniformLocation(programInfo.programID, "gNormal"), 2);
    glBindTexture(GL_TEXTURE_2D, gNormal);

    if (includeNoiseTexture) {

        //send the kernel samples to the shader
        glUniform3fv(glGetUniformLocation(programInfo.programID, "samples"), 64, glm::value_ptr(kernel[0]));

        //acccess tiling noise texture info
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glUniform1i(glGetUniformLocation(programInfo.programID, "texNoise"), 3);
    }
 
    if (includeAOTexture) {

        glActiveTexture(GL_TEXTURE4);
        glUniform1i(glGetUniformLocation(programInfo.programID, "AO"), 4);
        glBindTexture(GL_TEXTURE_2D, AOColorBuffer);
    }

    glBindVertexArray(programInfo.vao);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//called when GLUT draws something to screen
void OnDisplay() {

    glEnable(GL_DEPTH_TEST);
    //first geometry pass - render data to gbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // keep black so no leaking into gbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.SetTarget(teapotInfo.object->GetPosition()); // set camera target
    
    //set position of the light cube
    glm::vec3 lightPos = lightInfo.lightPosition;
    cubeObject.SetPosition(lightPos.x, lightPos.y, lightPos.z);
  
    //draw all objects to the screen
    for (ProgramInfo* program : drawObjects) {
        RenderMeshObject(*program, camera, false, false);
    }

    //second pass: use G-Buffer to render SSAO texture
    glBindFramebuffer(GL_FRAMEBUFFER, AOFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderScreenSpacePlane(screenPlaneInfo, true, false);

    //third pass: blur SSAO texture
   // glBindFramebuffer(GL_FRAMEBUFFER, AOBlurFBO);
     glBindFramebuffer(GL_FRAMEBUFFER, 0); //NOTE - this line is ONLY for testing, use above line once this works
    glClear(GL_COLOR_BUFFER_BIT);
    RenderScreenSpacePlane(blurPlaneInfo, false, true);

    /* for testing, let's try without rendering the final buffer
    
    */
    //third pass: use g-buffer to calculate scene lighting
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderScreenSpacePlane(renderPlaneInfo, false, true);
    
    //end of test deferred shading pass
    glutSwapBuffers();
}

//called when we want to initialize a depth map for use 
bool CreateShadowMap(ShadowInfo* shadowInfo) {

    unsigned int* depthMap = &shadowInfo->depthMap;
    unsigned int* depthMapFBO = &shadowInfo->depthMapFBO;

    glGenFramebuffers(1, depthMapFBO);

    glGenTextures(1, depthMap);
    glBindTexture(GL_TEXTURE_2D, *depthMap);
    //create the texture image as a depth component
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        shadowInfo->SHADOW_WIDTH,  shadowInfo->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_COMPARE_MODE,
        GL_COMPARE_REF_TO_TEXTURE);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_COMPARE_FUNC,
        GL_LEQUAL);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


    //after generating the depth map, attach it to the fbo
    glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthMap, 0);
    glDrawBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

//loads and returns an object file using string for text file name
cy::TriMesh LoadObjectFile(const char* fileName) {
    //create the mesh from obj data
    cy::TriMesh mesh;

    //check if mesh was opened correctly
    bool couldOpenPot = mesh.LoadFromFileObj(fileName);
    if (couldOpenPot) {
        std::cout << "opened teapot!" << std::endl;
    }
    else {
        std::cout << "Could not open the teapot!" << std::endl;
    }

    return mesh;
}

//centers the object, sets initial pos, rot, and scale
void InitializeObject(cy::TriMesh &mesh, Object &object) {
    //compute the bounding box to center the object in local space
    mesh.ComputeBoundingBox();
    cy::Vec3f boundMin = mesh.GetBoundMin();
    cy::Vec3f boundMax = mesh.GetBoundMax();
    cy::Vec3f centerPoint;
    centerPoint.x = (boundMin.x + boundMax.x) / 2;
    centerPoint.y = (boundMin.y + boundMax.y) / 2;
    centerPoint.z = (boundMin.z + boundMax.z) / 2;

    //set object starting position, rotation, scale
    object.SetCenter(glm::vec3(centerPoint.x, centerPoint.y, centerPoint.z)); //centers object in local space
    object.SetPosition(0.0, 8.0f, 0.0f);
    object.SetScale(1.0f);
}

//creates and binds a texture, given a specified filename and uniform variable
//currenlty used for both diffuse and specularity
void BindTexturesMTL(ProgramInfo &programInfo, const std::string& fileName, GLuint& texID, const GLchar* uniformName) {

    //create the texture variable
    glGenTextures(1, &texID);
    //this will hold the image raw data
    std::vector<unsigned char> image;

    //width and height of the texture image being loaded
    unsigned int textureWidth = 0;
    unsigned int textureHeight = 0;

    //load the image using lodePNG and output to image vector
     unsigned int imageSuccess = imageLoader.loadImage(image, textureWidth, textureHeight, fileName);

    //bind texture before filling with image data
    glActiveTexture(GL_TEXTURE0 + texID); //define unit zero, is also default unit
    glBindTexture(GL_TEXTURE_2D, texID);

    //create the actual image, fill with data from read file
    glTexImage2D(

        GL_TEXTURE_2D, //define as 2D texture type
        0, //at mipmap leveel 0 - highest resolution iamge
        GL_RGBA, //the internal formatting
        textureWidth, //image width
        textureHeight, //image height
        0, //border - this HAS to be 0
        GL_RGBA, //format - from image provided
        GL_UNSIGNED_BYTE, //data type = RGBA, RGBA, et cet in scanline format - 8 bits per channel
        &image[0] //pixel array data
    );

    //create mipmap levels
    //the order of the below function doesn't matter - 
    //we can generate mipmaps whenever as long as its before we use them and send to GPU!
    glGenerateMipmap(GL_TEXTURE_2D);

    //do trilinear filtering with mipmaps
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        //interpolates between two mipmap levels to avoid jittering
        GL_LINEAR_MIPMAP_LINEAR
    );

    //allows for magnification - when we get very close to texture,
    //individual texels can be larger than a single pixel!
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    //then we need to tell what to do when texture coord is outside 0-1

    //use repeated wrapping in t (vertical) direction
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    //use clamped wrapping in s (horizontal) direction
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    //setting the uniform sampler variable
    GLint sampler = glGetUniformLocation(programInfo.programID, uniformName);
    glUseProgram(programInfo.programID);
    glUniform1i(sampler, texID); //set to match the texture unit from glActiveTexture

}

//intializes the render buffer object
bool RenderToTexture() {
    
    //bind the render buffer object before anything else
    
    renderBuffer.Bind();

    //initialize render buffer object
    renderBuffer.Initialize(
        true, //create depth buffer
        4, //RGBA
        width, //texture width
        height //texture height
    );
    
    //build mipmaps
    renderBuffer.BuildTextureMipmaps();

    //sets bilinear filtering for magnification 
    //and trilinear filtering with mipmaps for minification
    renderBuffer.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

    //setting anisotropy value
     renderBuffer.SetTextureAnisotropy(16.0);

    //check that the render buffer is ready, if not, set a warning
    if (!renderBuffer.IsReady()) {
        std::cout << "WARNING! Render buffer is not ready for use!" << std::endl;
        return false;
    }

    std::cout << "Successfully initialized render texture!" << std::endl;

    //unbind render bufffer once we are done initializing it
    renderBuffer.Unbind();
    return true;
}

float Lerp(float a, float b, float f) {
    return a + f * (b - a);
}

//creates the hemisphere kernal for sampling points around AO frag
void CreateKernal() {

    //create sample kernal of points in hemisphere oriented along z tanget vector
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f); //creates random floats between 0 and 1
    std::default_random_engine randGenerator; //declare instance of random num generator
    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(
            randomFloats(randGenerator) * 2.0f - 1.0f,
            randomFloats(randGenerator) * 2.0f - 1.0f,
            randomFloats(randGenerator)//dont offset z, would create sphere instead of hemisphere
        );

        //weigh points more heavily as they are closer to the center fragmnent
        float size = (float)i / 64.0;
        size = Lerp(0.1f, 1.0f, size * size);
        sample *= size;
        kernel.push_back(sample);
    }

    //create random rotation noise data
    std::vector<glm::vec3> AONoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            randomFloats(randGenerator) * 2.0f - 1.0f,
            randomFloats(randGenerator) * 2.0f - 1.0f,
            0.0f); //leave z at zero to rotate around z axis
        AONoise.push_back(noise);
    }

    //create tiling 4x4 noise texture to overlay on the screen
    
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    //fill texture with noise data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &AONoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

//creats the SSAO frame buffer object with color attachments
void GenerateSSAOBuffer() {
    
    //first create the frame buffer for storing AO color
    glGenFramebuffers(1, &AOFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, AOFBO);

    glGenTextures(1, &AOColorBuffer);
    glBindTexture(GL_TEXTURE_2D, AOColorBuffer);
    //set to red because AO is greyscale component, only need the one color channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, AOColorBuffer, 0);

    //next create the frame buffer for blurring AO to remove random noise artifacts
    glGenFramebuffers(1, &AOBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, AOBlurFBO);

    glGenTextures(1, &AOColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, AOColorBufferBlur);
    //also only need one color channel for ao blurring
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //set min and mag texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, AOColorBufferBlur, 0);
}

//generates color and depth attachments for deferred rendering gbuffer
void GenerateDeferredBuffers() {

    //generate and bind the gbuffers
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // setting up position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL); //creating the texture space
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //setting nearest neighbor filtering for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // setting nearest neighbor filtering for magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //clamps so we dont oversample
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // setting up normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // setting up specular and color buffer
    glGenTextures(1, &gColorSpec);
    glBindTexture(GL_TEXTURE_2D, gColorSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

    //position is 0 buffer, normal is 1 buffer, color is 2
    // define which color attachments will be used for rendering;
    unsigned int atttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, atttachments);

    //generate depth map
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    glGenerateMipmap(GL_TEXTURE_2D);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "warning! gbuffer did not finish compiling!" << std::endl;
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//generates textures for given object
void GenerateTextures(cy::TriMesh &mesh, bool spec) {

    //load imageData with the pixel image data we get from the file
    int matNum = mesh.NM();
    std::cout << "Number of materials: " << matNum << std::endl;

    cy::TriMesh::Str diffuseTextureData;
    cy::TriMesh::Str specTextureData;

    for (int i = 0; i < matNum; i++) {
        const cy::TriMesh::Mtl& mat = mesh.M(0);
        diffuseTextureData = mat.map_Kd;
        if(spec){
            specTextureData = mat.map_Ks;
        }
    }

    const std::string diffuseFileName(diffuseTextureData.data);
    std::cout << "DIFFUSE FILE NAME: " << diffuseFileName << std::endl;
    //test with PNG to see if this works!
    BindTexturesMTL(teapotInfo, diffuseFileName, teapotInfo.texIDDiffuse, "diffuseTex");
    

    if (spec) {
        const std::string specFileName(specTextureData.data);
        std::cout << "SPEC FILE NAME: " << specFileName << std::endl;
        //do the same thing now for the specular texture
        BindTexturesMTL(teapotInfo, specFileName, teapotInfo.texIDSpec, "specTex");
    }

}

//environment cube mapping
void BindCubeMapTextures(GLuint &texID, std::vector<std::string> faceNames){

    glGenTextures(1, &texID);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    //for loop to generate texture images for all 6 faces
    int width = 0;
    int height = 0;
    int colorChannels = 0;
    
    for (unsigned int i = 0; i < faceNames.size(); i++) {
        
        unsigned char* image = imageLoader.loadImageFromPNG(faceNames[i].c_str(), width, height, colorChannels);
        if (image) {
            std::cout << "WIDTH: " << width << std::endl;
            std::cout << "HEIGHT: " << height << std::endl;
            std::cout << "ColorChannels " << colorChannels << std::endl;
            std::cout << "Generated face number: " << i << std::endl;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(
                //iterating the enum eahc time to move through each face
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,        //mipmap level 0
                GL_RGB,  //internal format
                width,    //image width
                height,   //image height
                0,        //borderr (must be 0)
                GL_RGB,  //format
                GL_UNSIGNED_BYTE,  //data type
                image //pixel array data
            );


        }
        else {
            std::cout << "failed to load image at path: " << faceNames[i] << std::endl;
            stbi_image_free(image);
        }

    }
    
    //next, generate a few mipmaps
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );
}

//creates buffer for vertex pos and normal info, sets related attributes
void CreateBuffers(ProgramInfo &programInfo, cy::TriMesh &mesh) {
    
    Object* object = programInfo.object;
    mesh = LoadObjectFile(object->objectFileName);

    //each mesh face has 3 associated vertices, store that number for later
    object->facesNumber = mesh.NF() * 3;

    //create a vector of vertices for vertex and normal buffering
    std::vector<Vertex> vertexData = std::vector<Vertex>(object->facesNumber);

    bool hasNormals = object->hasNormals;
    bool hasTexCoords = object->hasTexCoords;

    //takes position and normal values from vertices in the mesh, stores in vertexData vector above
    int vertexIndex = 0;
    for (int i = 0; i < mesh.NF(); i++) {
        // Get face indices for positions and normals
        const cy::TriMesh::TriFace& face = mesh.F(i);
        const cy::TriMesh::TriFace* faceNormal = hasNormals ? &mesh.FN(i) : nullptr;
        const cy::TriMesh::TriFace* faceTex = hasTexCoords ? &mesh.FT(i) : nullptr;

        if (i == 0) { std::cout << "DOES HAVE NORMALS? "<< object->objectFileName << "" << hasNormals << std::endl; }

        for (int c = 0; c < 3; c++) {
            // store position data
            const auto& point = mesh.V(face.v[c]);
            vertexData[vertexIndex].position[0] = point.x;
            vertexData[vertexIndex].position[1] = point.y;
            vertexData[vertexIndex].position[2] = point.z;

            if (hasNormals) {
                // store normal data
                const auto& norm = mesh.VN(faceNormal->v[c]);
                vertexData[vertexIndex].normals[0] = norm.x;
                vertexData[vertexIndex].normals[1] = norm.y;
                vertexData[vertexIndex].normals[2] = norm.z;
            }

            if (hasTexCoords) {
                //store texture coordinates
                const auto& tex = mesh.VT(faceTex->v[c]);
                const float flippedUV = 1.0f - tex.y;
                vertexData[vertexIndex].texCords[0] = tex.x;
                vertexData[vertexIndex].texCords[1] = flippedUV;
            }

            vertexIndex++;
        }
    }

    //then generate texture image
    if(object->hasTextures){ GenerateTextures(mesh, false); }
   
    //create buffer for holding mesh vertex data
    glGenBuffers(1, &programInfo.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, programInfo.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_STATIC_DRAW);

    //interpet position data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

    if (hasNormals) {
        //interpret normal data
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Vertex::normals));
    }
    if (hasTexCoords) {

        //interpret texture data
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Vertex::texCords));
    }
}

//creates buffers specifically for the plane object, different logic than above function due to lack of mesh obj file
void CreatePlaneBuffers(GLuint &vbo, Object &object) {
    
    //create buffer for holding mesh vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(float), &object.planeArrayFlipped[0], GL_STATIC_DRAW);

    //interpet position data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)0);

    //interpret normal data
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)(sizeof(float) * 3));

    //interpret tex coords data
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)(sizeof(float) * 6));
}

//creates buffers for 2D screenspace plane
void CreateScreenPlaneBuffers(GLuint& vbo) {
 
    float quadVertices[24] = {
        //positions //texCoords
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    
    //create buffer for holding mesh vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), &quadVertices[0], GL_STATIC_DRAW);

    //interpet position data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)0);

    //interpret tex coords data
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)(sizeof(float) * 2));
}

//compiles shaders with given program and file information
void CompileShaders(const char* vertName, const std::string &fragName, GLuint &vaoID, GLuint &programID) {

    //manually compile vertex shader
    std::ifstream fVert(vertName);
    std::string file_contentsVert{ std::istreambuf_iterator<char>(fVert), std::istreambuf_iterator<char>() };
    //create char array of correct length, copy string into char array
    char* vsSource = new char[file_contentsVert.length() + 1];
    std::strcpy(vsSource, file_contentsVert.c_str());
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* sourceChar = vsSource;
    glShaderSource(vs, 1, &sourceChar, nullptr);
    glCompileShader(vs);

    //check vertex compilation success
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::VERTEX::COMPILATION_FAILED FOR:" << vertName <<
            infoLog << std::endl;
    }
    else {
        std::cout << "Vertex shader compilation successful for " << vertName << std::endl;
    }

    //compile fragment shader
    std::ifstream fFrag(fragName);
    std::string file_contentsFrag{ std::istreambuf_iterator<char>(fFrag), std::istreambuf_iterator<char>() };
    char* fsSource = new char[file_contentsFrag.length() + 1];
    std::strcpy(fsSource, file_contentsFrag.c_str());
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragChar = fsSource;
    glShaderSource(fs, 1, &fragChar, nullptr);
    glCompileShader(fs);

    //get frag shader compilation success
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::FRAG::COMPILATION_FAILED FOR: " << fragName <<
            infoLog << std::endl;
    }
    else {
        std::cout << "Fragment shader compilation successful for " << fragName << std::endl;
    }

    //create and link program
    programID = glCreateProgram();
    glAttachShader(programID, vs);
    glAttachShader(programID, fs);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "ERROR: PROGRAM LINKING FAILED: " << infoLog << std::endl;
    }
    else {
        std::cout << "program linking successful" << std::endl;
    }

    //use program and delete shader objects
    glUseProgram(programID);
    glDeleteShader(vs);
    glDeleteShader(fs);

    //create vertex array object before the buffer,
    //stores the connections betwen a buffer and attributes for a particular object
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
}

//idle callback 
void OnIdle() {


    //get the current time
    currentTime = glutGet(GLUT_ELAPSED_TIME);

    //get the time since idle was last called
    float timeDifference = currentTime - previousTime;

    //sets idle time for camera
    camera.SetDeltaSpeed(timeDifference);

    //create an equalizing increment to effect animation, so that speed is constant
    float animateIncrement = animateSpeed * timeDifference;

    //if object is idly rotation, increase rotation angle
    if (teapotObject.rotating) {
        angleInRadians += animateIncrement;
    }

    //slowly increment color values for each vertex on the mesh object over time
    float colorSpeed = 0.0004f * timeDifference;
    for (int i = 0; i < 3; i++) {
        Color* color = colors[i];
        if (color->active)
        {
            if (color->goingUp) {
                color->value += colorSpeed;
                if (color->value >= 1) {
                    color->goingUp = false;
                }
            }
            else {
                color->value -= colorSpeed;
                if (color->value <= 0) {
                    color->goingUp = true;
                }
            }

        }
    }

    //assign updated color values to the mesh teapot object
    teapotInfo.object->SetColor(colors[0]->value, colors[1]->value, colors[2]->value);
    teapotObjectSecond.SetColor(colors[0]->value, colors[1]->value, colors[2]->value);

    //set previous time
    previousTime = currentTime;

    //now that GLUT is idle, tell GLUT that it needs to draw again
    glutPostRedisplay();
}

//mouse button callback
void OnMouse(int button, int state, int x, int y) {

    //send input to camera mouse button function
    camera.OnMouse(button, state, x, y);

    planeCamera.OnMouse(button, state, x, y);

    lightInfo.OnMouse(button, state, x, y);

    //tell glut to re-render
    glutPostRedisplay();
}

//mouse moving callback
void OnMouseMotion(int x, int y) {


    lightInfo.OnMouseMotion(x, y);

    //send input to camera mouse motion function
    camera.OnMouseMotion(x, y);

    planeCamera.OnMouseMotion(x, y);

    //tell glut to re-render
    glutPostRedisplay();
}

//keyboard input callback
void OnKeyPressed(unsigned char key, int x, int y) {

    //send input to the camera
    camera.OnKeyBoard(key);

    planeCamera.OnKeyBoard(key);

    //send input to the light
    lightInfo.OnKeyBoard(key);

    if (key == 'p') {
        //toggle between perspective and ortho rotation
        projInfo.ToggleProjection();
    }
    else if (key == 'r') {
        //toggle object idle rotation
         teapotInfo.object->ToggleRotating();
    }
    else if (key == 'u') {
        renderScreenSpace = !renderScreenSpace;
    }
}

void OnSpecialKeyPressed(int key, int x, int y) {

    //recompile shaders if 'f6' key is pressed
    if (key == GLUT_KEY_F6) {
        std::cout << "Compiled shaders!" << std::endl;
        
       // CompileShaders("cubeMap.vert", "cubeMap.frag", cubeInfo.vao, cubeInfo.programID);
       // CompileShaders("reflection.vert", "reflection.frag", teapotInfo.vao, teapotInfo.programID);
       // CompileShaders("renderedReflections.vert", "renderedReflections.frag", planeInfo.vao, planeInfo.programID);
    }

    //sets active rotation 
    if (key == GLUT_KEY_CTRL_L) {
        lightInfo.SetPressingButton(true);
        camera.enabled = false;
    }

    //sets active rotating camera to render plane instead of teapot
    if (key == GLUT_KEY_ALT_L) {
        std::cout << "Now rotating the plane!" << std::endl;
        planeCamera.SetEnabled(true);
        camera.SetEnabled(false);
    }

    //tell glut to re-render
    glutPostRedisplay();
}

void OnSpecialKeyPressedUp(int key, int x, int y) {

    if (key == GLUT_KEY_CTRL_L) {
        lightInfo.SetPressingButton(false);
        camera.enabled = true;
    }

    //tell glut to re-render
    glutPostRedisplay();
}

//create OpenGL callbacks
void CreateCallbacks() {
    //callback whenever GLUT draws something to the screen
    glutDisplayFunc(OnDisplay);

    //callback whenever a key is pressed
    glutKeyboardFunc(OnKeyPressed);

    //callback when a special key is pressed
    glutSpecialFunc(OnSpecialKeyPressed);

    //callback for when special key is let up
    glutSpecialUpFunc(OnSpecialKeyPressedUp);

    //callback when glut is idle
    glutIdleFunc(OnIdle);

    //callback when mouse button is pressed
    glutMouseFunc(OnMouse);

    //callback when mouse is idly moved
    glutMotionFunc(OnMouseMotion);
}

int main(int argc, char** argv)
{

    //initialize GLUT
    glutInit(&argc, argv);

    //initialize size & position of glut window
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);

    //initialize display mode with zbuffer, double buffering, and rgb colors
    glutInitDisplayMode(
        GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    //create glut debug context
    glutInitContextFlags(GLUT_DEBUG);

    //create rendering window
    glutCreateWindow("Teapot Time!");

    //create viewport
    glViewport(0, 0, width, height);

    //check version of glut
    const unsigned char* versionGL = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << versionGL << std::endl;

    //initialize glut callbacks
    CreateCallbacks();

    //initialize the glew extension wrangler
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cout << "did not initalize Glew correctly!" << std::endl;
        return 1;
    }

    //clear any colors, set the background to black transparent
    glClearColor(0, 0, 0, 0);

       //center teapot, set rot, pos, and scale
       InitializeObject(teapotMesh, *teapotInfo.object);

       //compile teapot for gBuffer
       CompileShaders("ambientObject.vert", "AOBuffer.frag", teapotInfo.vao, teapotInfo.programID);
       CreateBuffers(teapotInfo, teapotMesh);
       drawObjects.push_back(&teapotInfo);
 
       //compile  second teapot for gBuffer
         CompileShaders("ambientObject.vert", "AOBuffer.frag", teapotSecondInfo.vao, teapotSecondInfo.programID);
         CreateBuffers(teapotSecondInfo, teapotMesh);
         drawObjects.push_back(&teapotSecondInfo);

       //set second teapot scale and position in worldspace
        teapotObjectSecond.SetScale(0.65f);
        teapotObjectSecond.SetPosition(0.0, -15, -20.0f);

       //compile quad floor
        CompileShaders("ambientObject.vert", "AOBuffer.frag", quadInfo.vao, quadInfo.programID);
        CreateBuffers(quadInfo, quadMesh);
        drawObjects.push_back(&quadInfo);
       //set plane position, scale, and color for the scene
        quadObject.SetScale(200.0f);
        quadObject.SetPosition(0.0f, -15.0f, 5.0f);
        quadObject.SetColor(0.1f, 1.0f, 0.6f);

        //compile wall
        CompileShaders("ambientObject.vert", "AOBuffer.frag", wallInfo.vao, wallInfo.programID);
        CreateBuffers(wallInfo, quadMesh);
        drawObjects.push_back(&wallInfo);
        //set wall position, scale, and color
        wallObject.SetScale(25.0f);
        wallObject.SetPosition(0.0f, 38.0f, -27.0f);
        wallObject.SetRotation(90.0f, 0.0f, 0.0f);
        wallObject.SetColor(0.1f, 0.6f, 0.8f);

       //compile plane for SSAO render
        CompileShaders("screenPlane.vert", "SSAO.frag", screenPlaneInfo.vao, screenPlaneInfo.programID);
        CreateScreenPlaneBuffers(screenPlaneInfo.vbo);

        //compile plane for SSAO blurring shader
        CompileShaders("screenPlane.vert", "AOBlur.frag", blurPlaneInfo.vao, blurPlaneInfo.programID);
        CreateScreenPlaneBuffers(blurPlaneInfo.vbo);

       //compile plane for final render - uses same object as above, just renders colors differently
         CompileShaders("screenPlane.vert", "screenPlane.frag", renderPlaneInfo.vao, renderPlaneInfo.programID);
         CreateScreenPlaneBuffers(renderPlaneInfo.vbo);
         planeObject.SetScale(20.0f);
         planeObject.Rotate(90.0f, 0.0f, 0.0f);

       //compile plane for final screenspace lighting render

       //Compile shaders for the light model
       lightModelInfo.object->hasNormals = false;
       lightModelInfo.object->hasTexCoords = false;
       lightModelInfo.object->hasTextures = false;
        
       CompileShaders("lightModel.vert", "lightModel.frag", lightModelInfo.vao, lightModelInfo.programID);
       CreateBuffers(lightModelInfo, lightMesh);
        drawObjects.push_back(&lightModelInfo);
        cubeObject.scale = (0.3f);

       //create gBuffers for deferrred shading 
       GenerateDeferredBuffers();

       //generate SSAO buffer for storing occlusion information
        GenerateSSAOBuffer();

       //create the kernals for sampling depth values for SSAO
        CreateKernal();

       //initialize light position
        lightInfo.lightPosition = camera.GetPosition();

    //set the teapot camera to active by default
    camera.SetEnabled(true);

    //enable depth z buffer
   glEnable(GL_DEPTH_TEST);

    //Main loop glut operates in
    glutMainLoop();
    return 0;
}