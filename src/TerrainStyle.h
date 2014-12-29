#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class TerrainStyle {
public:
	// Constraints
	int minHeight;
	int maxHeight;
	int minSteepness;
	int maxSteepness;
	int minDistanceFromWater;
	int maxDistanceFromWater;

	// Apperance
	int textureIndex;
	float ambientReflectivity;
	float diffuseReflectivity;
	float specularReflectivity;
	float shininess;

	TerrainStyle();
};

} }