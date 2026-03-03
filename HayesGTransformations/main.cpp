
#define _CRT_SECURE_NO_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#include "stb_image.h"
//Glut/GL libaries
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//Additional Standard libraries
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
//cem yuksul libraries
#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyGL.h"
//Custom include files
#include "Vertex.h"
#include "WorldTransform.h"
#include "Camera.h"
#include "LightInfo.h"
#include "ProgramInfo.h"
#include "ColorInfo.h"
#include "ImageLoader.h"
//Hayes Geldmacher - 2/18  /26
//CS 6610 - Project 4: redux

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

//the frame buffer object for the shadows
unsigned int depthMapFBO;
//the resolution for the shadows
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
//the texture for the shadows
unsigned int depthMap;



//should we render to a plane or not
bool renderToTexture = true;

//render either a sphere or teapot
bool renderSphere = false;

//should we use reflections or textures
bool useReflections = false;

bool useShadows = true;


//shadow map
cy::GLRenderDepth2D shadowMap;

//teapot mesh being loaded in
cy::TriMesh teapotMesh;

//cube mesh used for background
cy::TriMesh cubeMesh;

//sphere mesh used instead of teapot for testing
cy::TriMesh sphereMesh;

cy::TriMesh quadMesh;


//program info for the teapot rendered into lightview to generate shadow textures
ProgramInfo teapotInfoShadow;

//program info for the teapot rendered with shadows
ProgramInfo teapotInfo;

//progrma info for plane rendered with shadows
ProgramInfo planeInfo;

//program info for the plane rendered into lightview to generate shadow textures
ProgramInfo planeInfoShadow;

//program info for the environment cube
ProgramInfo cubeInfo;

ProgramInfo depthDisplayInfo;

ProgramInfo teapotSecondInfo;

ProgramInfo teapotSecondShadow;

//the render buffer used for render-to-texture
cy::GLRenderTexture2D renderBuffer;

ProgramInfo quadInfo;
ProgramInfo quadInfoShadow;

//number of vertices in mesh
float verticesNumber = 0.0f;


//screen width and height
const static int width = 800;
const static int height = 800;

//float used for idle animations of mesh teapot
static float angleInRadians = 0.0f;

//lighting data
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
float ambientLightIntensity = 0.5f;
static LightInfo lightInfo(lightColor, ambientColor, ambientLightIntensity);

//animation time-tracking
float currentTime = 0;
float previousTime = 0;
float animateSpeed = 0.05f;

//instance of world object transform class for the render plane
WorldTransform planeObject;

//test plane for checking if the depth map was displayed correctly
WorldTransform depthDisplayObject;


//instance of world object transform class, generates transformation matrix
WorldTransform teapotObject("teapotReflection.obj");

WorldTransform teapotObjectSecond("teapotReflection.obj");


WorldTransform cubeObject("cube.obj");

WorldTransform sphereObject("sphere.obj");

WorldTransform quadObject("PlaneMesh.obj");


glm::vec3 camPos(0.0f, 0.0f, 5.0f);
glm::vec3 camTarget(2.0f, 0.0f, -5.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

//boolean, determines if you are rotating teapot object or the rendered plane
bool rotatingPlane = false;

//instace of camera class, generates view matrix
Camera camera(camPos, camTarget, camUp);

//instance of plane-specific camera 
Camera planeCamera(camPos, camTarget, camUp);

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

static persProj lightProjection;


//image loader object for loading texture data from image files
ImageLoader imageLoader;

//create a color for R,G,B, and store in an array
Color red(0.5, true, true);
Color blue(1, false, true);
Color green(0.2, false, true);

Color* colors[] = { &red, &blue, &green };
glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);

//called during onDisplay, sets all uniform shader variables
void SetUniformAttributesLighting(GLuint &program, Camera &camera, WorldTransform &object) {

    GLint uniformLocation;

    //update the uniform color variable in frag shader
    glm::vec3 objectColor = object.GetColor();
    uniformLocation = glGetUniformLocation(program, "objectColor");
    glUniform3f(uniformLocation, objectColor.x, objectColor.y, objectColor.z);

    //update the uniform ambient strength in the frag shader
    uniformLocation = glGetUniformLocation(program, "ambientStrength");
    glUniform1f(uniformLocation, lightInfo.ambientStrength);

    //update the uniform light color in the frag shader
    glm::vec3 lightColor = lightInfo.lightColor;
    uniformLocation = glGetUniformLocation(program, "lightColor");
    glUniform3f(uniformLocation, lightColor.x, lightColor.y, lightColor.z);

    //update uniform specular shininess exponent in the frag shader
    uniformLocation = glGetUniformLocation(program, "specShine");
    glUniform1f(uniformLocation, lightInfo.shine);

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
void SetUniformAttributesTransformations(ProgramInfo &programInfo, WorldTransform &object, Camera &camera, bool flipped) {
    
    //generate view matrix from camera

    glm::mat4 camViewMat = camera.GetMatrix();
    glm::mat4 worldMatrix = object.GetMat();
    
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

//Sets uniform shader variables for the environment cubemap
void SetUniformEnvironment(ProgramInfo& programInfo, WorldTransform& object, Camera& camera, bool flipped) {
    
    
    //generate view matrix from camera
    //remove translation from the env cube matrix, so it only corresponds to rotation
    glm::mat4 camViewMat = glm::mat4(glm::mat3(camera.GetMatrix()));

    //generate perpsective/ortho projection matrix
    glm::mat4 projMat = projInfo.GetProjection();

    glm::mat4 worldMat = cubeObject.GetMat();

    GLint uniformLocation;

    //send the camera view variable
    uniformLocation = glGetUniformLocation(programInfo.programID, "view");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &camViewMat[0][0]);

    //send the projection variable
    uniformLocation = glGetUniformLocation(programInfo.programID, "projection");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &projMat[0][0]);

    //send the camera view variable
    uniformLocation = glGetUniformLocation(programInfo.programID, "world");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &worldMat[0][0]);

    uniformLocation = glGetUniformLocation(programInfo.programID, "flipped");
    if (flipped) {
        glUniform1i(uniformLocation, 1);
    }
    else {
        glUniform1i(uniformLocation, 0);
    }
}

glm::mat4 GetLightMatrix() {
    //generate perpsective/ortho projection matrix
  
   // glm::mat4 projMat = projInfo.GetProjection();
    lightProjection.usePerspective = true;
    glm::mat4 projMat = lightProjection.GetProjection();


    glm::vec3 lightPos = lightInfo.lightPosition;

    glm::mat4 lightView = glm::lookAt(lightPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 lightMat = projMat * lightView;
    return lightMat;

}


//renders the teapot object from start to finish
void RenderTeapotObject(ProgramInfo &programInfo, Camera &camera, WorldTransform &object, bool flipped, bool useLight) {
    


    
    //use the desired shader program
    glUseProgram(programInfo.programID);
    glBindVertexArray(programInfo.vao);

    //update object rotation
    object.SetRotation(angleInRadians, angleInRadians, angleInRadians);

  

    if (useLight) {
        
        glm::mat4 lightMat = GetLightMatrix();
        GLint uniformLocation;
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "lightMat");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &lightMat[0][0]);

        glm::mat4 worldMatrix = object.GetMat();
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "world");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &worldMatrix[0][0]);

    }
    else {

        glm::mat4 lightMat = GetLightMatrix();
        GLint uniformLocation;
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "lightMat");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &lightMat[0][0]);
        SetUniformAttributesTransformations(programInfo, object, camera, flipped);

        //set uniform lighting atttributes
        SetUniformAttributesLighting(programInfo.programID, camera, object);
    }

    if (useReflections) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeInfo.texIDDiffuse);
    }
    else {

        //bind both textures being used
        //glBindTexture(GL_TEXTURE_2D, programInfo.texIDDiffuse);
        //glBindTexture(GL_TEXTURE_2D, programInfo.texIDSpec);

    }
    glDrawArrays(GL_TRIANGLES, 0, object.facesNumber);
}

void RenderPlaneObject(ProgramInfo &programInfo, WorldTransform &object, bool useLight) {
    glUseProgram(programInfo.programID);
    glBindVertexArray(programInfo.vao);

    GLuint uniformLocation;
    planeObject.SetRotation(0, angleInRadians, 0);

    if (useLight) {
        glm::mat4 lightMat = GetLightMatrix();
        GLint uniformLocation;
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "lightMat");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &lightMat[0][0]);

        glm::mat4 worldMatrix = object.GetMat();
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "world");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &worldMatrix[0][0]);
    }
    else {

        SetUniformAttributesTransformations(programInfo, object, camera, false);

        //set uniform lighting atttributes
        SetUniformAttributesLighting(programInfo.programID, camera, object);
    }


    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//renders the environment cube map
void RenderEnvironment(bool flipped) {
   
    //disable depth mask so we can render just environment
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    //render cube
    glUseProgram(cubeInfo.programID);
    glBindVertexArray(cubeInfo.vao);

    //set uniform shader variables
    SetUniformAttributesTransformations(cubeInfo, cubeObject, planeCamera, true);
    SetUniformEnvironment(cubeInfo, cubeObject, camera, flipped);

    //bind cube map and draw the cube vertices
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeInfo.texIDDiffuse);
    glDrawArrays(GL_TRIANGLES, 0, cubeObject.facesNumber);

    //re-enable depth mask when we're done
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

}



//called when GLUT draws something to screen
void OnDisplay() {



    //overall process for shadow maps looks like this:

    //1. first render to depth map
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //configure shaders and matrices here
    //do light transform stuff
    RenderTeapotObject(teapotInfo, camera, teapotObject, false, true);

   RenderTeapotObject(teapotSecondInfo, camera, teapotObjectSecond, false, true);
   RenderPlaneObject(planeInfo, planeObject, true);
   RenderTeapotObject(quadInfo, camera, quadObject, false, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    //next render the scene with shadow mapping with the depth map
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /*
    */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    //sets the camera target to teapot
    camera.SetTarget(teapotObject.GetPosition());
    RenderTeapotObject(teapotInfoShadow, camera, teapotObject, false, false);
    RenderTeapotObject(teapotSecondShadow, camera, teapotObjectSecond, false, false);
    RenderPlaneObject(planeInfoShadow, planeObject, false);
    RenderTeapotObject(quadInfoShadow, camera, quadObject, false, false);

    //finally, render the new depthTestPlane to ensure it works!
   
    glUseProgram(depthDisplayInfo.programID);
    glBindVertexArray(depthDisplayInfo.vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    SetUniformAttributesTransformations(depthDisplayInfo, depthDisplayObject, camera, false);
    //set uniform values to linearize 
    GLint uniformLocation;
    //update uniform specular shininess exponent in the frag shader
    uniformLocation = glGetUniformLocation(depthDisplayInfo.programID, "near_plane");
    glUniform1f(uniformLocation, projInfo.nearZ);

    uniformLocation = glGetUniformLocation(depthDisplayInfo.programID, "far_plane");
    glUniform1f(uniformLocation, projInfo.farZ);


    glDrawArrays(GL_TRIANGLES, 0, 6);
       
   /*
   */
    glutSwapBuffers();
    return;






    
    
    renderBuffer.Bind();
    //create mipmaps each frame
     renderBuffer.BuildTextureMipmaps();
    glClearColor(0, 0, 0, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    


        //render teapot once for render buffer reflections

     //   RenderTeapotObject(teapotInfo, camera, teapotObject, true);
       // RenderEnvironment(true);
    

    glBindVertexArray(0);
    renderBuffer.Unbind();



    // clear all relevant buffers
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderBuffer.BindTexture(0);

    //render teapot again to actually display in scene
   // RenderTeapotObject(teapotInfo, camera, teapotObject, false);
  
    /*
    
    */

     
   // RenderPlaneObject();

    // RenderEnvironment(false);
    
    //swap buffers, signifies that we are done rendering this frame
    glutSwapBuffers();
}

//called when we want to initialize a depth map for use 
bool CreateShadowMap() {

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    //create the texture image as a depth component
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH,  SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    //after generating the depth map, attach it to the fbo
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


   
    /*
    
    shadowMap.Initialize(
        true, 
        width, 
        height
    );

    shadowMap.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
    return true;
    
    */
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
void InitializeObject(cy::TriMesh &mesh, WorldTransform &object) {
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

//function for creating depth texture, following along with Cem's lecture
bool CreateDepthTexture() {

    //create and bind new texture
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    int shadowWidth = 0;
    int shadowHeight = 0;

    //initialize texture image for depth map
    glTexImage2D(GL_TEXTURE_2D, 
        0, 
        GL_DEPTH_COMPONENT,   //specifying that we are storing depth component, not rgba
        shadowWidth, shadowHeight, 
        0, 
        GL_DEPTH_COMPONENT, 
        GL_FLOAT, 0);
    
    //tell gpu that we want to do depth comparisons for shadow filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
        GL_COMPARE_REF_TO_TEXTURE);

    //set dpeth comparison mode to less than or equal
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC,
        GL_LEQUAL);

    //set nearest neighbor filtering for magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //set nearest neighbor for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //next, configure frame buffer
    GLuint frameBuffer = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //set depth map that we generated to be a depth attachment
    glFramebufferTexture(GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT, depthMap, 0);

    //just a depth map so no drawing to scene
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //check if the buffer is actually ready before attempting to use
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)
        != GL_FRAMEBUFFER_COMPLETE) return false;

    //below few lines is initialization, happens in main before entering loop
    GLuint shadowProgram = glCreateProgram();
    //uses a frag shader that ouputs just a constant color, doesnt matter what
    //we need a frag shader but it doesn't actually do anything


    //below is part tof render loop, called in OnDisplay

    //render to the depth map
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, shadowWidth, shadowHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadowProgram);
    //then set the MVP for light camera (the MLP)
    //bogus placeholder matrix
    glm::mat4 matrixMLP = glm::mat4(1.0f);
    glUniformMatrix4fv(
        glGetUniformLocation(shadowProgram, "mvp"),
        1, GL_FALSE, &matrixMLP[0][0]
    );
   // glDrawArrays(...);

    //render the camera view
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);//zero could also be another buffer, whatever
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glUseProgram(program) // use whatever program to render
   
    //below we are sending THIRD matrix
   //neither MVP NOR MLP
    /*
    glUniformMatrix4fv(
        glGetUniformLocation(program, "matrixShadow"),
        1, GL_FALSE, matrixShadow);
    
    */
    //glDrawArrays(...);


    //below is formula for transforming MLP to MSHADOW
    /*
        MSHADOW = T * S * MLP
        S is uniform scale matrix (0.5, 0.5, 0.5)

        //add just a tad of bias to z component to stop self-shadowing errors
        T is uniform translatiton matrix (0.5, 0.5f, 0.5f - bias)
     */

    //When SAMPLING from depth map texture, use MShadow
    //When RENDERING to depth map texture, use MLP

    /*
    
        BELOW: RENDER with cyGL.h helper functions!

        //initialization
        cy::GLRenderDepth2D shadowMap;

        shadowMap.Initialize(
            true,  //use depth comparison texturer
            with,
            height
        );

        shadowMap.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);

        //onDisplay

        shadowMap.Bind();
        clClear(GL_DEPTH_BUFFER_BIT);
        glDrawArrays(...);
        shadowMap.UnBind();
    */
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

//generates textures for given object
void GenerateTextures(cy::TriMesh &mesh) {

    //load imageData with the pixel image data we get from the file
    int matNum = mesh.NM();
    std::cout << "Number of materials: " << matNum << std::endl;

    cy::TriMesh::Str diffuseTextureData;
    cy::TriMesh::Str specTextureData;

    for (int i = 0; i < matNum; i++) {
        const cy::TriMesh::Mtl& mat = mesh.M(0);
        diffuseTextureData = mat.map_Kd;
        specTextureData = mat.map_Ks;
    }

    const std::string diffuseFileName(diffuseTextureData.data);
    //const char* diffuseFileName = diffuseTextureData.data;
    const std::string specFileName(specTextureData.data);
    std::cout << "DIFFUSE FILE NAME: " << diffuseFileName << std::endl;
    std::cout << "SPEC FILE NAME: " << specFileName << std::endl;

    //create and bind texture diffuse image
    //lets test with the other files to see if they work out the box
    //BindTextures(teapotInfo, diffuseFileName, teapotInfo.texIDDiffuse, "diffuseTex");
    //BindTexturesMTL(teapotInfo, diffuseFileName, teapotInfo.texIDDiffuse, "diffuseTex", false);

    //test with PNG to see if this works!
    BindTexturesMTL(teapotInfo, diffuseFileName, teapotInfo.texIDDiffuse, "diffuseTex");

    //do the same thing now for the specular texture
    BindTexturesMTL(teapotInfo, specFileName, teapotInfo.texIDSpec, "specTex");

}

//cube mapping for next week's project!
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

    //global flag, can be enabled
    //allows bilinear filtering between image seams
    //but a bit more expensive!
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //finally, bind cube map texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);


    //below will go in main render loop, just writing here for now
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //draw scene objectts
   //...

    //background is drawn second so that we don't waste
    //GPU memory by rendering pixels twice
    //such as background pixels that are later covered
    //by object pixels

    //we should use 0.9 or a similarly high number (but less than 1!)
    //as the z place to render background
    glDepthMask(GL_FALSE);
    
    //draw background
    //...

    glDepthMask(GL_TRUE);
}

//creates buffer for vertex pos and normal info, sets related attributes
void CreateBuffers(GLuint &vbo, WorldTransform &object, cy::TriMesh &mesh, bool hasNormals, bool hasTexCoords, bool generateTextures) {
    

    mesh = LoadObjectFile(object.objectFileName);

    //each mesh face has 3 associated vertices, store that number for later
    object.facesNumber = mesh.NF() * 3;

    //create a vector of vertices for vertex and normal buffering
    std::vector<Vertex> vertexData = std::vector<Vertex>(object.facesNumber);

    //takes position and normal values from vertices in the mesh, stores in vertexData vector above
    int vertexIndex = 0;
    for (int i = 0; i < mesh.NF(); i++) {
        // Get face indices for positions and normals
        const cy::TriMesh::TriFace& face = mesh.F(i);
        const cy::TriMesh::TriFace* faceNormal = hasNormals ? &mesh.FN(i) : nullptr;
        const cy::TriMesh::TriFace* faceTex = hasTexCoords ? &mesh.FT(i) : nullptr;

        if (i == 0) { std::cout << "DOES HAVE NORMALS? "<< object.objectFileName << "" << hasNormals << std::endl; }

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
    if(generateTextures){ GenerateTextures(mesh); }
   
    //create buffer for holding mesh vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
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
void CreatePlaneBuffers(GLuint &vbo, WorldTransform &object) {
    
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
    teapotObject.SetColor(colors[0]->value, colors[1]->value, colors[2]->value);

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
        teapotObject.ToggleRotating();
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
        rotatingPlane = true;
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

    //sets active rotating camera to teapot instead of render plane
    if (key == GLUT_KEY_ALT_L) {
        rotatingPlane = false;
        std::cout << "Now rotating the teapot!" << std::endl;
        planeCamera.SetEnabled(false);
        camera.SetEnabled(true);
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

    //compile shaders for the environment cubemap
  //  CompileShaders("cubeMap.vert", "cubeMap.frag", cubeInfo.vao, cubeInfo.programID);

    //create buffers for env cube
 //   CreateBuffers(cubeInfo.vbo, cubeObject, cubeMesh, false, false, false);

    //create array of texture face fileNames
    /*
    std::vector<std::string> faceNames = {
        "cubemap_posx.png",
        "cubemap_negx.png",
        "cubemap_posy.png",
        "cubemap_negy.png",
        "cubemap_posz.png",
        "cubemap_negz.png",
    };

    
    */
    //create and bind 6 texture faces for the cubemap
  //  BindCubeMapTextures(cubeInfo.texIDDiffuse, faceNames);


       //compile teapot for shadow map
       CompileShaders("shadowMap.vert", "shadowMap.frag", teapotInfo.vao, teapotInfo.programID);
       CreateBuffers(teapotInfo.vbo, teapotObject, teapotMesh, true, true, false);
        
       //center teapot, set rot, pos, and scale
       InitializeObject(teapotMesh, teapotObject);



       //compile teapot for actual rendering
       CompileShaders("shadowObject.vert", "shadowObject.frag", teapotInfoShadow.vao, teapotInfoShadow.programID);
       CreateBuffers(teapotInfoShadow.vbo, teapotObject, teapotMesh, true, true, false);

       //compile teapot for actual rendering
       CompileShaders("shadowMap.vert", "shadowMap.frag", teapotSecondInfo.vao, teapotSecondInfo.programID);
       CreateBuffers(teapotSecondInfo.vbo, teapotObjectSecond, teapotMesh, true, true, false);

       //compile teapot for actual rendering
       CompileShaders("shadowObject.vert", "shadowObject.frag", teapotSecondShadow.vao, teapotSecondShadow.programID);
       CreateBuffers(teapotSecondShadow.vbo, teapotObjectSecond, teapotMesh, true, true, false);

       teapotObjectSecond.SetScale(1.0f);
       teapotObjectSecond.SetPosition(0.0, -15, -20.0f);

       CompileShaders("shadowMap.vert", "shadowMap.frag", quadInfo.vao, quadInfo.programID);
       CreateBuffers(quadInfo.vbo, quadObject, quadMesh, true, true, false);

       CompileShaders("shadowObject.vert", "shadowObject.frag", quadInfoShadow.vao, quadInfoShadow.programID);
       CreateBuffers(quadInfoShadow.vbo, quadObject, quadMesh, true, true, false);


       quadObject.SetScale(3.0f);
       quadObject.SetPosition(0.0f, -6.0f, 5.0f);

       //compile plane for shadow map
       CompileShaders("shadowMap.vert", "shadowMap.frag", planeInfo.vao, planeInfo.programID);
       CreatePlaneBuffers(planeInfo.vbo, planeObject);

       //compile plane for actual rendering
       CompileShaders("shadowObject.vert", "shadowObject.frag", planeInfoShadow.vao, planeInfoShadow.programID);
       CreatePlaneBuffers(planeInfoShadow.vbo, planeObject);


       //compile testing display depth plane
       CompileShaders("depthDisplay.vert", "depthDisplay.frag", depthDisplayInfo.vao, depthDisplayInfo.programID);
       CreatePlaneBuffers(depthDisplayInfo.vbo, depthDisplayObject);

       //intialize render texure, set filtering and bind
       //RenderToTexture();

       //set up shadow map!
       CreateShadowMap();

       //set up scale + position for object, don't need rotation
       planeObject.SetScale(10.0f);
       planeObject.SetPosition(0.0, -20, 0.0f);
       


       depthDisplayObject.SetScale(3.0f);
       depthDisplayObject.SetPosition(0.0f, 8.0f, 10.0f);
       depthDisplayObject.Rotate(-90.0f, 0.0f, 0.0f);


    //set the teapot camera to active by default
    camera.SetEnabled(true);

    //enable depth z buffer
   glEnable(GL_DEPTH_TEST);

    //Main loop glut operates in
    glutMainLoop();
    return 0;
}