
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
//Hayes Geldmacher - 3/25/26
//CS 6610 - Project 8: tessellations

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

//teapot mesh being loaded in
cy::TriMesh teapotMesh;

//mesh used to load plane
cy::TriMesh quadMesh;

//mesh used to load light model
cy::TriMesh lightMesh;

//program  info for the model displaying the light
ProgramInfo lightModelInfo;

//program info for the teapot rendered into lightview to generate shadow textures
ProgramInfo teapotInfoShadow;

//program info for the teapot rendered with shadows
ProgramInfo teapotInfo;

//progrma info for plane rendered with shadows
ProgramInfo planeInfo;

//program info for the plane rendered into lightview to generate shadow textures
ProgramInfo planeInfoShadow;

//program info for testing the depth display on a plane
ProgramInfo depthDisplayInfo;

//program info for escond teapot 
ProgramInfo teapotSecondInfo;

//program info for second teapot second pass using shadow map
ProgramInfo teapotSecondShadow;

//the render buffer used for render-to-texture
cy::GLRenderTexture2D renderBuffer;

ProgramInfo quadInfo;
ProgramInfo quadInfoShadow;
ProgramInfo wireframeInfo;

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

WorldTransform quadObject("PlaneMesh.obj");

WorldTransform cubeObject("cube.obj");

glm::vec3 camPos(0.0f, 0.0f, 5.0f);
glm::vec3 camTarget(2.0f, 0.0f, -5.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

//boolean, determines if you are rotating teapot object or the rendered plane
bool rotatingPlane = false;

bool displayWireFrame = false;

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

//tessellation levels:
int minTessLevel = 1;
int maxTessLevel = 80;
int currentTessLevel = 5;


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
void RenderMeshObject(ProgramInfo &programInfo, Camera &camera, WorldTransform &object, bool useTextures, bool useLight, bool useNormalMap) {
    
    //use the desired shader program
    glUseProgram(programInfo.programID);
    glBindVertexArray(programInfo.vao);

    //get the object-to-light transform matrix
        glm::mat4 lightMat = GetLightMatrix();
        GLint uniformLocation;
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "lightMat");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &lightMat[0][0]);

        glm::mat4 worldMatrix = object.GetMat();
        //send the world transform variable
        uniformLocation = glGetUniformLocation(programInfo.programID, "world");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &worldMatrix[0][0]);
 
        //if not just for depth map, assign general uniform attributes
    if(!useLight){

        SetUniformAttributesTransformations(programInfo, object, camera, false);
        //set uniform lighting atttributes
        SetUniformAttributesLighting(programInfo.programID, camera, object);
    }

    if (useTextures) {
        glBindTexture(GL_TEXTURE_2D, programInfo.texIDDiffuse);
    }

    if (useNormalMap) {
        glBindTexture(GL_TEXTURE_2D, programInfo.texIDNormal);
    }


    glDrawArrays(GL_TRIANGLES, 0, object.facesNumber);
}

//called when GLUT draws something to screen
void OnDisplay() {

    //1. first render to depth map
  //  glCullFace(GL_FRONT);
 //   glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  //  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  //  glClear(GL_DEPTH_BUFFER_BIT);

    //render both teapots and the plane to the depth buffer
        //update object rotation
  //  teapotObject.SetRotation(angleInRadians, angleInRadians, angleInRadians);
  //  RenderMeshObject(teapotInfo, camera, teapotObject, false, true);
    //RenderMeshObject(teapotSecondInfo, camera, teapotObjectSecond, false, true);
 //   RenderMeshObject(quadInfo, camera, quadObject, false, true);
 //   glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    glCullFace(GL_BACK);
    
    //next render the scene like usual, using depth map as texture
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, depthMap);
    //sets the camera target to teapot
    camera.SetTarget(depthDisplayObject.GetPosition());
    //RenderMeshObject(teapotInfoShadow, camera, teapotObject, false, false);
    //RenderMeshObject(teapotSecondShadow, camera, teapotObjectSecond, false, false);
   //  RenderMeshObject(quadInfoShadow, camera, quadObject, false, false, true);

    //now try rendering the plane
     //use the desired shader program
   // glUseProgram(depthDisplayInfo.programID);
   // glBindVertexArray(depthDisplayInfo.vao);
   // SetUniformAttributesLighting(depthDisplayInfo.programID, camera, depthDisplayObject);
   // SetUniformAttributesTransformations(depthDisplayInfo, depthDisplayObject, camera, false);
   // glBindTexture(GL_TEXTURE_2D, depthDisplayInfo.texIDNormal);
   // int max;
    //glGetIntegerv(GL_MAX_PATCH_VERTICES, 3);
  //  glPatchParameteri(GL_PATCH_VERTICES, 3);
  //  glDrawArrays(GL_PATCHES, 0, 6);

        //render the wireframe
    if (displayWireFrame) {
            glUseProgram(wireframeInfo.programID);
            glBindVertexArray(wireframeInfo.vao);
            glBindTexture(GL_TEXTURE_2D, wireframeInfo.texIDDisplace);
            GLuint uniformLocation = glGetUniformLocation(wireframeInfo.programID, "tessLevel");
            glUniform1f(uniformLocation, currentTessLevel);
            SetUniformAttributesTransformations(wireframeInfo, depthDisplayObject, camera, false);
            glPatchParameteri(GL_PATCH_VERTICES, 3);
            glDrawArrays(GL_PATCHES, 0, 6);
    }

        //render the shadowed plane
        glUseProgram(planeInfo.programID);
        glBindVertexArray(planeInfo.vao);
        glBindTexture(GL_TEXTURE_2D, planeInfo.texIDNormal);
        glBindTexture(GL_TEXTURE_2D, planeInfo.texIDDisplace);
        GLuint uniformLocation = glGetUniformLocation(planeInfo.programID, "tessLevel");
        glUniform1f(uniformLocation, currentTessLevel);
        SetUniformAttributesTransformations(planeInfo, depthDisplayObject, camera, false);
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glDrawArrays(GL_PATCHES, 0, 6);

        
    //render the light model object
    glm::vec3 lightPos = lightInfo.lightPosition;
    cubeObject.SetPosition(lightPos.x, lightPos.y, lightPos.z);
    RenderMeshObject(lightModelInfo, camera, cubeObject, false, false, false);

    //swap buffers, end loop
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
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
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

void LoadNormalImage(const std::string fileName, ProgramInfo& programInfo, const GLchar* uniformName) {

    glGenTextures(1, &programInfo.texIDDiffuse);

    glActiveTexture(GL_TEXTURE0 + programInfo.texIDDiffuse); //define unit zero, is also default unit
    glBindTexture(GL_TEXTURE_2D, programInfo.texIDDiffuse);

    //for loop to generate texture images for all 6 faces
    int width = 0;
    int height = 0;
    int colorChannels = 0;

    const char* newNormalFile = "cubemap_negz.png";
        unsigned char* image = imageLoader.loadImageFromPNG(newNormalFile, width, height, colorChannels);
        if (image) {
            std::cout << "WIDTH: " << width << std::endl;
            std::cout << "HEIGHT: " << height << std::endl;
            std::cout << "ColorChannels " << colorChannels << std::endl;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(
                //iterating the enum eahc time to move through each face
                GL_TEXTURE_2D,
                0,        //mipmap level 0
                GL_RGBA,  //internal format
                width,    //image width
                height,   //image height
                0,        //borderr (must be 0)
                GL_RGBA,  //format
                GL_UNSIGNED_BYTE,  //data type
                image //pixel array data
            );


        }
        else {
            std::cout << "failed to load image at path: " << newNormalFile << std::endl;
            stbi_image_free(image);
        }

     //setting the uniform sampler variable
    GLint sampler = glGetUniformLocation(programInfo.programID, uniformName);
    glUseProgram(programInfo.programID);
    glUniform1i(sampler, programInfo.texIDDiffuse); //set to match the texture unit from glActiveTexture

}

//creates buffer for vertex pos and normal info, sets related attributes
void CreateBuffers(GLuint &vbo, WorldTransform &object, cy::TriMesh &mesh, bool hasNormals, bool hasTexCoords, bool generateTextures, bool generateNormalMap) {
    

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
    if(generateTextures){ GenerateTextures(mesh, false); }

    if(generateNormalMap){
        std::string teapotTextureName = "teapotNormal.png";
        const GLchar* uniformName = "normalMap";
        //LoadNormalImage(teapotTextureName, quadInfoShadow, uniformName);
        BindTexturesMTL(quadInfoShadow, teapotTextureName, quadInfoShadow.texIDNormal, uniformName);
        
    }
   
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
void CreatePlaneBuffers(GLuint &vbo, WorldTransform &object, bool createNormalTexture, bool createDisplaceTextures, ProgramInfo& programInfo) {
    
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

    if (createNormalTexture) {
        std::string teapotTextureName = "teapotNormal.png";
        const GLchar* uniformName = "normalTexture";
        //LoadNormalImage(teapotTextureName, quadInfoShadow, uniformName);
        BindTexturesMTL(programInfo, teapotTextureName, programInfo.texIDNormal, uniformName);
    }
    if (createDisplaceTextures) {
        std::string textureName = "teapotDisplace.png";
        const GLchar* uniformName = "displaceTexture";
        BindTexturesMTL(programInfo, textureName, programInfo.texIDDisplace, uniformName);
    }
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

//compiles shaders with given program and file information
void CompileShadersWithGeo(const char* vertName, const std::string& fragName, const std::string& geoName, const std::string& controlName, const std::string& evalName, GLuint& vaoID, ProgramInfo& program) {

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
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::FRAG::COMPILATION_FAILED FOR: " << fragName <<
            infoLog << std::endl;
    }
    else {
        std::cout << "Fragment shader compilation successful for " << fragName << std::endl;
    }

    GLuint es = 0;
    GLuint cs = 0;
    if (program.renderTessellationsShader) {

        //manually compile tessellation control shader
        std::ifstream fControl(controlName);
        std::string file_contentsControl{ std::istreambuf_iterator<char>(fControl), std::istreambuf_iterator<char>() };
        //create char array of correct length, copy string into char array
        char* csSource = new char[file_contentsControl.length() + 1];
        std::strcpy(csSource, file_contentsControl.c_str());
        cs = glCreateShader(GL_TESS_CONTROL_SHADER);
        const GLchar* controlChar = csSource;
        glShaderSource(cs, 1, &controlChar, nullptr);
        glCompileShader(cs);

        //get control shader compilation success
        glGetShaderiv(cs, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(cs, 512, NULL, infoLog);
            std::cout << "ERROR:SHADER::TESC::COMPILATION_FAILED FOR: " << controlName <<
                infoLog << std::endl;
        }
        else {
            std::cout << "Tess Control shader compilation successful for " << controlName << std::endl;
        }

        //manually compile tessellation evaluation shader
        std::ifstream fEval(evalName);
        std::string file_contentsEval{ std::istreambuf_iterator<char>(fEval), std::istreambuf_iterator<char>() };
        //create char array of correct length, copy string into char array
        char* esSource = new char[file_contentsEval.length() + 1];
        std::strcpy(esSource, file_contentsEval.c_str());
        es = glCreateShader(GL_TESS_EVALUATION_SHADER);
        const GLchar* evalChar = esSource;
        glShaderSource(es, 1, &evalChar, nullptr);
        glCompileShader(es);

        //get control shader compilation success
        glGetShaderiv(es, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(es, 512, NULL, infoLog);
            std::cout << "ERROR:SHADER::TESC::COMPILATION_FAILED FOR: " << evalName <<
                infoLog << std::endl;
        }
        else {
            std::cout << "Tess Control shader compilation successful for " << evalName << std::endl;
        }

    }


    GLuint gs = 0;
    if (program.renderGeometryShader) {

        //manually compile geometry shader
        std::ifstream fGeo(geoName);
        std::string file_contentsGeo{ std::istreambuf_iterator<char>(fGeo), std::istreambuf_iterator<char>() };
        //create char array of correct length, copy string into char array
        char* gsSource = new char[file_contentsGeo.length() + 1];
        std::strcpy(gsSource, file_contentsGeo.c_str());
        gs = glCreateShader(GL_GEOMETRY_SHADER);
        const GLchar* geoChar = gsSource;
        glShaderSource(gs, 1, &geoChar, nullptr);
        glCompileShader(gs);

        //get geo shader compilation success
        glGetShaderiv(gs, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(gs, 512, NULL, infoLog);
            std::cout << "ERROR:SHADER::GEO::COMPILATION_FAILED FOR: " << geoName <<
                infoLog << std::endl;
        }
        else {
            std::cout << "Geometry shader compilation successful for " << geoName << std::endl;
        }

    }

    //create and link program
    program.programID = glCreateProgram();
    glAttachShader(program.programID, vs);
    glAttachShader(program.programID, fs);
    if (program.renderTessellationsShader) {

        glAttachShader(program.programID, cs);
        glAttachShader(program.programID, es);

    }
    if (program.renderGeometryShader) {
    glAttachShader(program.programID, gs);

    }
    glLinkProgram(program.programID);

    glGetProgramiv(program.programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program.programID, 512, NULL, infoLog);
        std::cout << "ERROR: PROGRAM LINKING FAILED: " << infoLog << std::endl;
    }
    else {
        std::cout << "program linking successful" << std::endl;
    }

    //use program and delete shader objects
    glUseProgram(program.programID);
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
    teapotObjectSecond.SetColor(colors[0]->value, colors[1]->value, colors[2]->value);

    //set previous time
    previousTime = currentTime;

    //now that GLUT is idle, tell GLUT that it needs to draw again
    glutPostRedisplay();
}

void ChangeTessLevel(bool up) {

    if (up) {
        currentTessLevel += 1;
        if (currentTessLevel > maxTessLevel) { currentTessLevel = maxTessLevel; }
    }
    else {
        currentTessLevel -= 1;
        if (currentTessLevel < minTessLevel) { currentTessLevel = minTessLevel; }
    }
        std::cout << "Current Tessellation level: " << currentTessLevel << std::endl;
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
    else if (key == 32) { //space bar 
        
        //toggle wireframe
        displayWireFrame = !displayWireFrame;
        if (displayWireFrame) { std::cout << "Now displaying wireframe!" << std::endl; }
        else {
            std::cout << "No longer displaying wireframe!" << std::endl;
        }
    
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

    if (key == GLUT_KEY_LEFT) {

        ChangeTessLevel(false);
    }
    else if (key == GLUT_KEY_RIGHT) {
        ChangeTessLevel(true);
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

       //compile teapot for shadow map
    // CompileShaders("shadowMap.vert", "shadowMap.frag", teapotInfo.vao, teapotInfo.programID);
    // CreateBuffers(teapotInfo.vbo, teapotObject, teapotMesh, true, true, false);
        
       //center teapot, set rot, pos, and scale
     // InitializeObject(teapotMesh, teapotObject);

       //compile teapot for actual rendering
       //CompileShaders("shadowObject.vert", "shadowObject.frag", teapotInfoShadow.vao, teapotInfoShadow.programID);
       //CreateBuffers(teapotInfoShadow.vbo, teapotObject, teapotMesh, true, true, false);

       //compile second teapot for shadow map
       //CompileShaders("shadowMap.vert", "shadowMap.frag", teapotSecondInfo.vao, teapotSecondInfo.programID);
       //CreateBuffers(teapotSecondInfo.vbo, teapotObjectSecond, teapotMesh, true, true, false);

       //compile  second teapot for actual rendering
       //CompileShaders("shadowObject.vert", "shadowObject.frag", teapotSecondShadow.vao, teapotSecondShadow.programID);
       //CreateBuffers(teapotSecondShadow.vbo, teapotObjectSecond, teapotMesh, true, true, false);

       //set second teapot scale and position in worldspace
      // teapotObjectSecond.SetScale(1.0f);
      // teapotObjectSecond.SetPosition(0.0, -15, -20.0f);

       //compile plane for shadow map
      // CompileShaders("shadowMap.vert", "shadowMap.frag", quadInfo.vao, quadInfo.programID);
       //CreateBuffers(quadInfo.vbo, quadObject, quadMesh, true, true, false, false);

       //compile plane for actual rendering
       //CompileShaders("shadowObject.vert", "shadowObject.frag", quadInfoShadow.vao, quadInfoShadow.programID);
      // CreateBuffers(quadInfoShadow.vbo, quadObject, quadMesh, true, true, false, true);

       //set plane position, scale, and color for the scene
     //  quadObject.SetScale(20.0f);
     //  quadObject.SetPosition(0.0f, -15, 5.0f);
     //  quadObject.SetRotation(90.0f, 0.0f, 0.0f);
    //   quadObject.SetColor(0.1f, 1.0f, 0.6f);
       //compile shaders for the wireframe plane with geometry shader

        //render the wireframe overlay
          wireframeInfo.renderGeometryShader = true;
          wireframeInfo.renderTessellationsShader = true;
          CompileShadersWithGeo("wireframePlane.vert", "wireframePlane.frag", "geoShader.geom", "tescControl.tesc", "tesEval.tese", wireframeInfo.vao, wireframeInfo);
          CreatePlaneBuffers(wireframeInfo.vbo, depthDisplayObject, false, true, wireframeInfo);

       //render the actual shadowed plane
       planeInfo.renderGeometryShader = false;
       planeInfo.renderTessellationsShader = true;

       CompileShadersWithGeo("TessellatedPlane.vert", "TessellatedPlane.frag", "geoShader.geom", "tescControl.tesc", "tesEval.tese", planeInfo.vao, planeInfo);
       CreatePlaneBuffers(planeInfo.vbo, depthDisplayObject, true, true, planeInfo);

       //compile testing display depth plane
      // CompileShaders("wireframePalne.vert", "shadowObject.frag", depthDisplayInfo.vao, depthDisplayInfo.programID);
      // CreatePlaneBuffers(depthDisplayInfo.vbo, depthDisplayObject, true);


       depthDisplayObject.SetScale(5.0f);
       depthDisplayObject.SetPosition(0.0f, -15, 5.0f);
       depthDisplayObject.SetColor(0.1f, 1.0f, 0.6f);

       glGetError();


       //Compile shaders for the light model
         CompileShaders("lightModel.vert", "lightModel.frag", lightModelInfo.vao, lightModelInfo.programID);
        CreateBuffers(lightModelInfo.vbo, cubeObject, lightMesh, false, false, false, false);

       //set light model scale
        cubeObject.scale = (0.3f);

       //initialize shadow/depth map texture
      // CreateShadowMap();

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