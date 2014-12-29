#include "TerrainStyle.h"

using namespace IntelOrca::PopSS;

TerrainStyle::TerrainStyle()
{
	this->minHeight = 0;
	this->maxHeight = 1024;
	this->minSteepness = 0;
	this->maxSteepness = 1024;
	this->minDistanceFromWater = 0;
	this->maxDistanceFromWater = INT32_MAX;

	this->ambientReflectivity = 1.0f;
	this->diffuseReflectivity = 1.0f;
	this->specularReflectivity = 1.0f;
	this->shininess = 32.0f;
}