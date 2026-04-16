
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
#include "ShaderCompiler.h"
#include "Camera.h"
#include "LightInfo.h"
#include "ProgramInfo.h"
#include "ImageLoader.h"
#include "Object.h"
#include "DrawObjects.h"
#include "AOInfo.h"
//Hayes Geldmacher - 4/16/26
//CS 6610 - Final Project

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

    hold and release 'let ctrl' to switch between camera mode and light rotation mode
        when in light rotation mode, moving the mouse will not spin the active camera
        Instead, it will change the light diriection and position

   Ambient Occlusion Controls:
        press 'm' to toggle between naive SSAO and mult-scale SSAO
        press 'b' to toggle AO blurring
        press 'l' to toggle lighting
        press the up and down keys to alter AO intensity
        press the right and left keys to alter the AO bias
        press the 'r' and 't' keys to alter the AO radius

    press 'f6' to recompile shaders
*/

//screen width and height
const static int width = 800;
const static int height = 800;

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

ProgramInfo quadInfoShadow;

#pragma endregion shadowInformation

#pragma region lightInformation
glm::vec3 lightColor = glm::vec3(0.961, 0.875, 0.616);
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

#pragma endregion cameraInformation

//animation time-tracking
float currentTime = 0;
float previousTime = 0;

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
float timeDifference = 0.0f;
float frameCurrentTime = 0.0f;
float framePreviousTime = 0.0f;
float frameTimeDifference = 0.0f;
unsigned int counter = 0.0f;

DrawObjectsContainer container;
ShaderCompiler compiler;
AOInfo AO;


void SetUniformEnvironment(ProgramInfo& programInfo, WorldTransform& object, Camera& camera) {


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
}


void RenderEnvironment() {
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    //render cube
    glUseProgram(container.CubeTarget()->programID);
    glBindVertexArray(container.CubeTarget()->vao);

    //SetUniformAttributesTransformations(cubeInfo, cubeObject, planeCamera);
    SetUniformEnvironment(*container.CubeTarget(), container.CubeTarget()->object, camera);


    glBindTexture(GL_TEXTURE_CUBE_MAP, container.CubeTarget()->texIDDiffuse);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    renderBuffer.BindTexture(0);
}

//set uniform lighting attributes
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

//sets MVP uniform variables
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

//draws screen space quad
void RenderScreenQuad(ProgramInfo& programInfo) {
    glBindVertexArray(programInfo.vao);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//render screenspace plane for deferred rendering
void RenderScreenSpacePlane(ProgramInfo& programInfo, bool includeNoiseTexture, bool includeAOTexture, bool includeLighting) {
   
    glUseProgram(programInfo.programID);
    
    //send projection information
    glm::mat4 projMat = projInfo.GetProjection();
    GLuint projectionLocation = glGetUniformLocation(programInfo.programID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projMat[0][0]);

    //send gbuffer data as uniform textures
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(programInfo.programID, "gPosition"), 0);
    glBindTexture(GL_TEXTURE_2D, compiler.gPosition);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(programInfo.programID, "gColorSpec"), 1);
    glBindTexture(GL_TEXTURE_2D, compiler.gColorSpec);

    glActiveTexture(GL_TEXTURE2);
    glUniform1i(glGetUniformLocation(programInfo.programID, "gNormal"), 2);
    glBindTexture(GL_TEXTURE_2D, compiler.gNormal);

    //does this pass need random noise? 
    if (includeNoiseTexture) {

        //send the kernel samples to the shader
        glUniform3fv(glGetUniformLocation(programInfo.programID, "samples"), compiler.kernelNumber, glm::value_ptr(compiler.kernel[0]));

        //acccess tiling noise texture info
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, compiler.noiseTexture);
        glUniform1i(glGetUniformLocation(programInfo.programID, "texNoise"), 3);

        //send uniform variables to control AO
        glUniform1i(glGetUniformLocation(programInfo.programID, "kernelSize"), compiler.kernelNumber);
        glUniform1i(glGetUniformLocation(programInfo.programID, "power"), AO.AOPower);
        glUniform1f(glGetUniformLocation(programInfo.programID, "bias"), AO.AObias);
        glUniform1f(glGetUniformLocation(programInfo.programID, "radius"), AO.AOradius);
    }
 
    if (includeAOTexture) {

        //send AO info as uniform texture
        glActiveTexture(GL_TEXTURE4);
        glUniform1i(glGetUniformLocation(programInfo.programID, "AO"), 4);
        glBindTexture(GL_TEXTURE_2D, compiler.AOColorBuffer);
    }

    //is this render calculating lighting
    if (includeLighting) {
        if (AO.renderAO) {
            glUniform1i(glGetUniformLocation(programInfo.programID, "renderAO"), 1);
            glActiveTexture(GL_TEXTURE4);
            glUniform1i(glGetUniformLocation(programInfo.programID, "AO"), 4);
            if(AO.blurAO){ glBindTexture(GL_TEXTURE_2D, compiler.AOColorBufferBlur); }
            else{ glBindTexture(GL_TEXTURE_2D, compiler.AOColorBuffer); }
            
        }
        else {
            glUniform1i(glGetUniformLocation(programInfo.programID, "renderAO"), 0);
        }
        
        if (AO.calculateLighting) {
            SetUniformAttributesLighting(programInfo.programID, camera);
            glm::mat4 camViewMat = camera.GetMatrix();
            GLint uniformLocation;
            //send the camera view variable
            uniformLocation = glGetUniformLocation(programInfo.programID, "view");
            glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &camViewMat[0][0]);
            glUniform1i(glGetUniformLocation(programInfo.programID, "calculateLighting"), 1);
        }
        else{
            glUniform1i(glGetUniformLocation(programInfo.programID, "calculateLighting"), 0);
        }
    }
    //draw screen space quad
    RenderScreenQuad(programInfo);
}

//GLUT callback for drawing to the screen
void OnDisplay() {

    
    glEnable(GL_DEPTH_TEST);
    //first geometry pass - render data to gbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, compiler.gBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // keep black so no leaking into gbuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.SetTarget(container.CameraTarget()->object->GetPosition()); // set camera target
    
    //set position of the light cube
    glm::vec3 lightPos = lightInfo.lightPosition;
    //cubeObject.SetPosition(lightPos.x, lightPos.y, lightPos.z);
  
    //draw all objects to the screen
    for (ProgramInfo* program : container.drawObjects) {
        RenderMeshObject(*program, camera, false, false);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (AO.renderAO) {

       //second pass: use G-Buffer to render SSAO texture
      glBindFramebuffer(GL_FRAMEBUFFER, compiler.AOFBO);
      glClear(GL_COLOR_BUFFER_BIT);
      if (AO.useMultiAO) { RenderScreenSpacePlane(AO.resolutionPlaneInfo, true, false, false); }
      else { RenderScreenSpacePlane(AO.screenPlaneInfo, true, false, false); }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      if (AO.blurAO) {
        //third pass: blur SSAO texture
        glBindFramebuffer(GL_FRAMEBUFFER, compiler.AOBlurFBO); 
        glClear(GL_COLOR_BUFFER_BIT);
        RenderScreenSpacePlane(AO.blurPlaneInfo, false, true, false);
      }
    }

    //fourth pass: use g-buffer to calculate scene lighting
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    RenderScreenSpacePlane(AO.renderPlaneInfo, false, false, true);

    //end of test deferred shading pass
    glutSwapBuffers();
}

//create shadow map FBO and depth attachment
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
        0, //at mipmap leveel 0 - highest resolution image
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
    //BindTexturesMTL(teapotInfo, diffuseFileName, teapotInfo.texIDDiffuse, "diffuseTex");
    

    if (spec) {
        const std::string specFileName(specTextureData.data);
        std::cout << "SPEC FILE NAME: " << specFileName << std::endl;
        //do the same thing now for the specular texture
      //  BindTexturesMTL(teapotInfo, specFileName, teapotInfo.texIDSpec, "specTex");
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

//creates buffers screen space plane with normalized device coordinates
void CreateScreenPlaneBuffers(GLuint& vbo) {
 
    //create quad from float array
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

//calculates and display current frames per second in window title
void CalculateFPS() {
   
    frameCurrentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    frameTimeDifference = frameCurrentTime - framePreviousTime;
    counter++;
    if (frameTimeDifference > 0.25f) {
        std::string FPS = std::to_string(counter/ frameTimeDifference);
        std::string ms = std::to_string((counter/ frameTimeDifference) * 1000);
        std::string newTitle = "It's Teapot Time! FPS: " + FPS + " ms: " + ms;
        glutSetWindowTitle(newTitle.c_str());
        //set previous time
        framePreviousTime = frameCurrentTime;
        counter = 0;
    }
}

//GLUT callback for idle 
void OnIdle() {

    CalculateFPS();

    //get the current time
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    //get the time since idle was last called
    timeDifference = currentTime - previousTime;
    //sets idle time for camera
    camera.SetDeltaSpeed(timeDifference);

    //set previous time
    previousTime = currentTime;
    //now that GLUT is idle, tell GLUT that it needs to draw again
    glutPostRedisplay();
}

//GLUT callback for mouse buttons
void OnMouse(int button, int state, int x, int y) {

    //send input to camera mouse button function
    camera.OnMouse(button, state, x, y);

    lightInfo.OnMouse(button, state, x, y);

    //tell glut to re-render
    glutPostRedisplay();
}

//GLUT callback for mouse motion
void OnMouseMotion(int x, int y) {

    lightInfo.OnMouseMotion(x, y);

    //send input to camera mouse motion function
    camera.OnMouseMotion(x, y);

    //tell glut to re-render
    glutPostRedisplay();
}

//GLUT callback for standard key input
void OnKeyPressed(unsigned char key, int x, int y) {

    //send input to the camera
    camera.OnKeyBoard(key);

    //send input to the light
    lightInfo.OnKeyBoard(key);

    if (key == 'p') {
        //toggle between perspective and ortho rotation
        projInfo.ToggleProjection();
    }
    else if (key == 'b') {
        AO.blurAO = !AO.blurAO;
        if (AO.blurAO) { std::cout << "Enabled ambient occlusion blurring!" << std::endl; }
        else { std::cout << "Disabled ambient occlusion blurring!" << std::endl; }
    }
    else if (key == 'o') {
        AO.renderAO = !AO.renderAO;
        if (AO.renderAO) { std::cout << "Enabled ambient occlusion!" << std::endl; }
        else { std::cout << "Disabled ambient occlusion!" << std::endl; }
    }
    else if (key == 'l') {
        AO.calculateLighting = !AO.calculateLighting;
        if (AO.calculateLighting) { std::cout << "Enabled lighting!" << std::endl; }
        else { std::cout << "Disabled lighting!" << std::endl; }
    }
    else if (key == 'm') {
        AO.useMultiAO = !AO.useMultiAO;
        if (AO.useMultiAO) { std::cout << "Enabled MSSSAO!" << std::endl; }
        else { std::cout << "Disabled MSSSAO!" << std::endl; }
    }
    if (key == 'r') {
        AO.SetAORadius(false);
    }
    else if (key == 't') {
        AO.SetAORadius(true);
    }
}

//GLUT callback for special key input
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

    if (key == GLUT_KEY_DOWN) {
        AO.SetAOPower(false);
    }
    else if (key == GLUT_KEY_UP) {
        AO.SetAOPower(true);
    }

    if (key == GLUT_KEY_LEFT) {
       AO.SetAOBias(false);
    }
    else if (key == GLUT_KEY_RIGHT) {
       AO.SetAOBias(true);
    }
    //tell glut to re-render
    glutPostRedisplay();
}

//GLUT callback for special key input
void OnSpecialKeyPressedUp(int key, int x, int y) {

    //set mouse to control main light instead of camera
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

   
    //set up cubemap shaders and buffer
    container.InitializeCubemap();

    //create array of texture face filenames
    std::vector<std::string> faceNames = {
         "cubemap_posx.png",
        "cubemap_negx.png",
        "cubemap_posy.png",
        "cubemap_negy.png",
        "cubemap_posz.png",
        "cubemap_negz.png",
    };

    BindCubeMapTextures(container.CubeTarget()->texIDDiffuse, faceNames);

    //set up list of all standard objects to rendered in the scene
    container.InitializeObjects();

    //compile screen space plane for naive ssao 
    compiler.CompileShaders("screenPlane.vert", "SSAO.frag", AO.screenPlaneInfo.vao, AO.screenPlaneInfo.programID);
    CreateScreenPlaneBuffers(AO.screenPlaneInfo.vbo);

    //compile screen space plane for mutli scale ssao 
    compiler.CompileShaders("screenPlane.vert", "MSSAOPass.frag", AO.resolutionPlaneInfo.vao, AO.resolutionPlaneInfo.programID);
    CreateScreenPlaneBuffers(AO.resolutionPlaneInfo.vbo);

    //compile screen space plane for SSAO blurring shader
    compiler.CompileShaders("screenPlane.vert", "AOBlur.frag", AO.blurPlaneInfo.vao, AO.blurPlaneInfo.programID);
    CreateScreenPlaneBuffers(AO.blurPlaneInfo.vbo);

    //compile screen space plane for final lighting render
    compiler.CompileShaders("screenPlane.vert", "screenPlane.frag", AO.renderPlaneInfo.vao, AO.renderPlaneInfo.programID);
    CreateScreenPlaneBuffers(AO.renderPlaneInfo.vbo);

    //create gBuffers for deferrred shading 
    compiler.CreateDeferredBuffer(width, height);

    //create frame buffer objects for ssao
    compiler.CreateSSAOBuffer( width, height);

    //create the kernals for sampling random depth values for SSAO
    compiler.CreateKernel();

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