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

#pragma endregion objectInformation

void DrawObjectsContainer::InitializeObjects() {

}
