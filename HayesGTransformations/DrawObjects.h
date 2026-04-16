#pragma once
//custom include files
#include "Object.h"
#include "ProgramInfo.h"
#include "ShaderCompiler.h"
#include "Camera.h"
#include "ProjectionInfo.h"
//cem yuksul libraries
#include "cyCore.h"
#include "cyTriMesh.h"
#include "ShaderCompiler.h"
#include "cyGL.h"

//holds list of all standard objects tot be drawn to the screen
class DrawObjectsContainer {
public:
	std::vector<ProgramInfo*> drawObjects;
	void InitializeObjects();
	void InitializeCubemap();
	void SetUniformEnvironment(Camera&, persProj&);
	ProgramInfo* CameraTarget();
	ProgramInfo* CubeTarget();
	ShaderCompiler compiler;

};