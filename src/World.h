#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class TerrainStyle;

struct WorldTile {
	unsigned int height;
	unsigned char terrain;
	glm::vec3 lightNormal;
	bool shore;
};

class World {
public:
	static const float TileSize;
	static const float OceanTileSize;
	static const float SkyDomeRadius;

	int size;
	int numTerrainStyles;
	TerrainStyle *terrainStyles;

	World();
	~World();
	
	void Reprocess();
	void ProcessTile(int x, int z);
	
	bool IsShore(int landX, int landZ) const;
	int *World::GenerateDistanceFromWaterMap() const;

	glm::vec3 CalculateNormal(int landX, int landZ) const;
	static glm::vec3 CalculateNormal(float height, float westHeight, float eastHeight, float northHeight, float southHeight);
	static float CalculateMidAngle(float leftHeight, float midHeight, float rightHeight);

	int GetSteepness(int landX, int landZ) const;
	int CalculateTerrain(int landX, int landZ) const;

	void LoadLandFromPOPTB(const char *path);

	WorldTile *GetTile(int x, int z) const;

	template<typename T>
	T MapWrap(T xz) const {
		while (xz < 0)
			xz += this->size;
		while (xz >= this->size)
			xz -= this->size;
		return xz;
	}

private:
	WorldTile *tiles;
	int *distanceFromWaterMap;
};

} }