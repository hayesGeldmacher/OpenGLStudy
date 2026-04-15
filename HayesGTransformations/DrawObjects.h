#pragma once
//custom include files
#include "Object.h"
#include "ProgramInfo.h"
//cem yuksul libraries
#include "cyCore.h"
#include "cyTriMesh.h"
#include "cyGL.h"

struct ObjectHolder {

	Object object;
	ProgramInfo objectInfo;
	//mesh used to load plane
	cy::TriMesh objectMesh;
};

class DrawObjectsContainer {
public:
	std::vector<ObjectHolder> objectHolder;

	void InitializeObjects();
};