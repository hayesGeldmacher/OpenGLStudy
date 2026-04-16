#pragma once
//custom include files
#include "Object.h"
#include "ProgramInfo.h"
#include "ShaderCompiler.h"
//cem yuksul libraries
#include "cyCore.h"
#include "cyTriMesh.h"
#include "ShaderCompiler.h"
#include "cyGL.h"

class DrawObjectsContainer {
public:
	std::vector<ProgramInfo*> drawObjects;
	void InitializeObjects();
	ProgramInfo* CameraTarget();
	ShaderCompiler compiler;

};