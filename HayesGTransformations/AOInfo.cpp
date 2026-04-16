#include "AOInfo.h"

//alters intensity of AO
void AOInfo::SetAOPower(bool up) {
    if (up) {
        currentOcclusionLevel++;
        if (currentOcclusionLevel >= 4) { currentOcclusionLevel = 3; }
    }
    else {
        currentOcclusionLevel--;
        if (currentOcclusionLevel < 0) { currentOcclusionLevel = 0; }
    }
    AOPower = occlusionPowerLevels[currentOcclusionLevel];
    std::cout << "set AO power to: " << AOPower << std::endl;
}

//alters bias of AO
void AOInfo::SetAOBias(bool up) {
    if (up) {
        currentBiasLevel++;
        if (currentBiasLevel >= 4) { currentBiasLevel = 3; }
    }
    else {
        currentBiasLevel--;
        if (currentBiasLevel < 0) { currentBiasLevel = 0; }
    }
    AObias = biasLevels[currentBiasLevel];
    std::cout << "set AO bias to: " << AObias << std::endl;
}

//alters radius of AO
void AOInfo::SetAORadius(bool up) {
    if (up) {
        currentRadiusLevel++;
        if (currentRadiusLevel >= 4) { currentRadiusLevel = 3; }
    }
    else {
        currentRadiusLevel--;
        if (currentRadiusLevel < 0) { currentRadiusLevel = 0; }
    }
    AOradius = radiusLevels[currentRadiusLevel];
    std::cout << "set AO radius to: " << AOradius << std::endl;
}
