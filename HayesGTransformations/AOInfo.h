#pragma once
#include "ProgramInfo.h"
#include <sstream>

//stores information related to AO
struct AOInfo {
public:
	//whether to render ambient occlusion
	bool renderAO = true;
	bool blurAO = true;
	bool useMultiAO = true;
	bool calculateLighting = false;

	//change the exaggeration of AO
	int AOPower = 1;
	int currentOcclusionLevel = 0;
	int occlusionPowerLevels[4] = { 1, 2, 4, 8 };

	//change the radius for AO
	float AOradius = 2.0f;
	int currentRadiusLevel = 1;
	float radiusLevels[4] = { 0.5f, 2.0f, 3.5f, 5.0f };

	//change the radius for AO
	float AObias = 0.5f;
	int currentBiasLevel = 1;
	float biasLevels[4] = { 0.25, 0.5f, 0.75f, 1.0f };
	//program info getting SSAO to the screen
	ProgramInfo screenPlaneInfo;

	//program info for blurring info
	ProgramInfo blurPlaneInfo;

	//program info for getting final render to the scren
	ProgramInfo renderPlaneInfo;

	//program info for rendering multi scale AO
	ProgramInfo resolutionPlaneInfo;

	int influenceRadius = 4; //how global the AO is, how many resolutions are sampled

	void SetAOPower(bool);
	void SetAOBias(bool);
	void SetAORadius(bool);
};