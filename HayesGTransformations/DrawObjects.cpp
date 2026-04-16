#include "DrawObjects.h"

//declare objects and programs
#pragma region objectInformation
//instance of world object transform class for the screenspace render plane
Object planeObject;

Object quadObject("models/PlaneMesh.obj");
ProgramInfo quadInfo(&quadObject);
//mesh used to load plane
cy::TriMesh quadMesh;

Object wallObject("models/PlaneMesh.obj");
ProgramInfo wallInfo(&wallObject);

//program  info for the model displaying the light
Object cubeObject("models/cube.obj");
ProgramInfo lightModelInfo(&cubeObject);
//mesh used to load light model
cy::TriMesh lightMesh;

//program info the loading the pillar
Object pillarObject("models/pillar.obj");
ProgramInfo pillarInfo(&pillarObject);
cy::TriMesh pillarMesh;

//program info for load the dais
Object daisObject("models/dais.obj");
ProgramInfo daisInfo(&daisObject);
cy::TriMesh daisMesh;

Object ceilingObject("models/ceiling.obj");
ProgramInfo ceilingInfo(&ceilingObject);
cy::TriMesh ceilingMesh;

Object angelObject("models/angel.obj");
ProgramInfo angelInfo(&angelObject);
cy::TriMesh angelMesh;

Object angelObjectSecond("models/angel.obj");
ProgramInfo angelInfoSecond(&angelObjectSecond);

Object rockObject("models/rocks.obj");
ProgramInfo rockInfo(&rockObject);
cy::TriMesh rockMesh;

Object vaseObject("models/vases.obj");
ProgramInfo vaseInfo(&vaseObject);
cy::TriMesh vaseMesh;

#pragma endregion objectInformation

void DrawObjectsContainer::InitializeObjects() {

    //compile pillar
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", pillarInfo.vao, pillarInfo.programID);
    compiler.CreateBuffers(pillarInfo, pillarMesh);
    drawObjects.push_back(&pillarInfo);
    compiler.BindTexturesMTL(pillarInfo, "textures/marble.png", pillarInfo.texIDDiffuse, "albedoTexture");
    pillarObject.SetScale(5.0f);
    pillarObject.SetPosition(0, -15.0f, 0.0f);

    //compile dais 
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", daisInfo.vao, daisInfo.programID);
    compiler.CreateBuffers(daisInfo, daisMesh);
    compiler.BindTexturesMTL(daisInfo, "textures/dais.png", daisInfo.texIDDiffuse, "albedoTexture");
    drawObjects.push_back(&daisInfo);
    daisObject.SetScale(5.0f);
    daisObject.SetPosition(0.0f, -15.0f, 0.0f);
    daisObject.Rotate(0.0f, 90.0f, 0.0f);

    //compile ceiling
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", ceilingInfo.vao, ceilingInfo.programID);
    compiler.CreateBuffers(ceilingInfo, ceilingMesh);
    drawObjects.push_back(&ceilingInfo);
    compiler.BindTexturesMTL(ceilingInfo, "textures/brickWall.png", ceilingInfo.texIDDiffuse, "albedoTexture");
    ceilingObject.SetScale(5.0f);
    ceilingObject.SetPosition(0, -15, 0);

    //compile angel
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", angelInfo.vao, angelInfo.programID);
    compiler.CreateBuffers(angelInfo, angelMesh);
    drawObjects.push_back(&angelInfo);
    compiler.BindTexturesMTL(angelInfo, "textures/angel.png", angelInfo.texIDDiffuse, "albedoTexture");
    angelObject.SetScale(9.2f);
    angelObject.SetPosition(-50.0f, 5.0, -40.0f);
    angelObject.Rotate(0.0f, 15.0f, 0.0f);

    //compile second angel
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", angelInfoSecond.vao, angelInfoSecond.programID);
    compiler.CreateBuffers(angelInfoSecond, angelMesh);
    drawObjects.push_back(&angelInfoSecond);
    compiler.BindTexturesMTL(angelInfoSecond, "textures/angel.png", angelInfoSecond.texIDDiffuse, "albedoTexture");
    angelObjectSecond.SetScale(8.5f);
    angelObjectSecond.SetPosition(28.0f, 5.0, -58.0f);
    angelObjectSecond.Rotate(0.0f, -25.0f, 0.0f);

    //compile rocks 
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", rockInfo.vao, rockInfo.programID);
    compiler.CreateBuffers(rockInfo, rockMesh);
    drawObjects.push_back(&rockInfo);
    compiler.BindTexturesMTL(rockInfo, "textures/plate.png", rockInfo.texIDDiffuse, "albedoTexture");
    rockObject.SetScale(6.5f);
    rockObject.SetPosition(0, -25.0f, -80.0f);

    //compile vases
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", vaseInfo.vao, vaseInfo.programID);
    compiler.CreateBuffers(vaseInfo, vaseMesh);
    drawObjects.push_back(&vaseInfo);
    compiler.BindTexturesMTL(vaseInfo, "textures/vases.png", vaseInfo.texIDDiffuse, "albedoTexture");
    vaseObject.SetScale(7.0f);
    vaseObject.SetPosition(-50, -15, 25.0f);
    vaseObject.Rotate(0.0f, -45.0f, 0.0f);

    //compile quad floor
    compiler.CompileShaders("shaders/ambientObject.vert", "shaders/AOBuffer.frag", quadInfo.vao, quadInfo.programID);
    compiler.CreateBuffers(quadInfo, quadMesh);
    drawObjects.push_back(&quadInfo);

    //set plane position, scale, and color for the scene
    quadObject.SetScale(200.0f);
    quadObject.SetPosition(0.0f, -15.0f, 5.0f);

    planeObject.SetScale(20.0f);
    planeObject.Rotate(90.0f, 0.0f, 0.0f);
}

ProgramInfo* DrawObjectsContainer::CameraTarget() {
    return &daisInfo;
}
