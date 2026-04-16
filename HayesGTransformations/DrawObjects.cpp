#include "DrawObjects.h"

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

//program info the loading the pillar
Object pillarObject("pillar.obj");
ProgramInfo pillarInfo(&pillarObject);
cy::TriMesh pillarMesh;

//program info for load the dais
Object daisObject("dais.obj");
ProgramInfo daisInfo(&daisObject);
cy::TriMesh daisMesh;

Object ceilingObject("ceiling.obj");
ProgramInfo ceilingInfo(&ceilingObject);
cy::TriMesh ceilingMesh;

Object angelObject("angel.obj");
ProgramInfo angelInfo(&angelObject);
cy::TriMesh angelMesh;

Object angelObjectSecond("angel.obj");
ProgramInfo angelInfoSecond(&angelObjectSecond);

Object rockObject("rocks.obj");
ProgramInfo rockInfo(&rockObject);
cy::TriMesh rockMesh;

Object vaseObject("vases.obj");
ProgramInfo vaseInfo(&vaseObject);
cy::TriMesh vaseMesh;

Object cubeObject("cube.obj");
ProgramInfo cubeInfo;
cy::TriMesh cubeMesh;

#pragma endregion objectInformation

void DrawObjectsContainer::InitializeObjects() {
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", pillarInfo.vao, pillarInfo.programID);
    compiler.CreateBuffers(pillarInfo, pillarMesh);
    drawObjects.push_back(&pillarInfo);
    pillarObject.SetScale(5.0f);
    pillarObject.SetPosition(0, -15.0f, 0.0f);

    //compile dais 
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", daisInfo.vao, daisInfo.programID);
    compiler.CreateBuffers(daisInfo, daisMesh);
    drawObjects.push_back(&daisInfo);
    daisObject.SetScale(5.0f);
    daisObject.SetPosition(0.0f, -15.0f, 0.0f);
    daisObject.Rotate(0.0f, 90.0f, 0.0f);

    //compile ceiling
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", ceilingInfo.vao, ceilingInfo.programID);
    compiler.CreateBuffers(ceilingInfo, ceilingMesh);
    drawObjects.push_back(&ceilingInfo);
    ceilingObject.SetScale(5.0f);
    ceilingObject.SetPosition(0, -15, 0);

    //compile angel
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", angelInfo.vao, angelInfo.programID);
    compiler.CreateBuffers(angelInfo, angelMesh);
    drawObjects.push_back(&angelInfo);
    angelObject.SetScale(9.2f);
    angelObject.SetPosition(-50.0f, -15.0f, -40.0f);
    angelObject.Rotate(0.0f, 15.0f, 0.0f);

    //compile second angel
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", angelInfoSecond.vao, angelInfoSecond.programID);
    compiler.CreateBuffers(angelInfoSecond, angelMesh);
    drawObjects.push_back(&angelInfoSecond);
    angelObjectSecond.SetScale(8.5f);
    angelObjectSecond.SetPosition(28.0f, -15.0f, -58.0f);
    angelObjectSecond.Rotate(0.0f, -25.0f, 0.0f);

    //compile rocks 
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", rockInfo.vao, rockInfo.programID);
    compiler.CreateBuffers(rockInfo, rockMesh);
    drawObjects.push_back(&rockInfo);
    rockObject.SetScale(7.0f);
    rockObject.SetPosition(-4.0f, -20.0f, -65.0f);

    //compile vases
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", vaseInfo.vao, vaseInfo.programID);
    compiler.CreateBuffers(vaseInfo, vaseMesh);
    drawObjects.push_back(&vaseInfo);
    vaseObject.SetScale(7.0f);
    vaseObject.SetPosition(-50, -15, 25.0f);
    vaseObject.Rotate(0.0f, -45.0f, 0.0f);

    //compile quad floor
    compiler.CompileShaders("ambientObject.vert", "AOBuffer.frag", quadInfo.vao, quadInfo.programID);
    compiler.CreateBuffers(quadInfo, quadMesh);
    drawObjects.push_back(&quadInfo);

    //set plane position, scale, and color for the scene
    quadObject.SetScale(200.0f);
    quadObject.SetPosition(0.0f, -15.0f, 5.0f);

    planeObject.SetScale(20.0f);
    planeObject.Rotate(90.0f, 0.0f, 0.0f);
}

void DrawObjectsContainer::InitializeCubemap() {
    compiler.CompileShaders("cubeMap.vert", "cubeMap.frag", cubeInfo.vao, cubeInfo.programID);
    cubeInfo.object->hasNormals = false;
    cubeInfo.object->hasTexCoords = false;
    cubeInfo.object->hasTextures = false;
    compiler.CreateBuffers(cubeInfo, cubeMesh);
    cubeObject.SetScale(2.0f); //create cubemap to be very large
}

ProgramInfo* DrawObjectsContainer::CameraTarget() {
    return &daisInfo;
}

ProgramInfo* DrawObjectsContainer::CubeTarget() {
    return &cubeInfo;
}

void DrawObjectsContainer::SetUniformEnvironment(Camera& camera, persProj& projInfo) {


    //generate view matrix from camera
    //remove translation from the env cube matrix, so it only corresponds to rotation
    glm::mat4 camViewMat = glm::mat4(glm::mat3(camera.GetMatrix()));

    //generate perpsective/ortho projection matrix
    glm::mat4 projMat = projInfo.GetProjection();

    glm::mat4 worldMat = cubeObject.GetMat();

    GLint uniformLocation;

    //send the camera view variable
    uniformLocation = glGetUniformLocation(cubeInfo.programID, "view");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &camViewMat[0][0]);

    //send the projection variable
    uniformLocation = glGetUniformLocation(cubeInfo.programID, "projection");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &projMat[0][0]);

    //send the camera view variable
    uniformLocation = glGetUniformLocation(cubeInfo.programID, "world");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &worldMat[0][0]);
}

