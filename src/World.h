#pragma once

#include "LightManager.h"
#include "PopSS.h"
#include "Util/MathExtensions.hpp"

namespace IntelOrca { namespace PopSS {

class TerrainStyle;
class Unit;
class WorldObject;

struct WorldTile {
	unsigned int height;
	unsigned char terrain;
	glm::vec3 lightNormal;
	bool shore;
};

class World {
public:
	static const int TileSize;
	static const float OceanTileSize;
	static const float SkyDomeRadius;

	int size;
	int sizeSquared;
	int sizeByNonTiles;
	int numTerrainStyles;
	TerrainStyle *terrainStyles;

	std::list<WorldObject*> objects;

	bool landHighlightActive;
	glm::ivec3 landHighlightSource;
	glm::ivec3 landHighlightTarget;
	std::list<Unit*> selectedUnits;

	LightManager lightManager;
	glm::vec3 skyColour;

	World();
	~World();
	
	void Update();

	void Reprocess();
	void ProcessTile(int x, int z);
	
	bool IsShore(int landX, int landZ) const;
	void GenerateDistanceFromWaterMap();

	glm::vec3 CalculateNormal(int landX, int landZ) const;
	static glm::vec3 CalculateNormal(float height, float westHeight, float eastHeight, float northHeight, float southHeight);
	static float CalculateMidAngle(float leftHeight, float midHeight, float rightHeight);

	int GetSteepness(int landX, int landZ) const;
	int CalculateTerrain(int landX, int landZ) const;

	void LoadLandFromPOPTB(const char *path);

	WorldTile *GetTile(int x, int z) const;
	int GetHeight(int x, int z) const;

	template<typename T>
	T Wrap(T xz) const { return wraprange((T)0, xz, (T)(this->size * TileSize)); }

	template<typename T>
	T TileWrap(T xz) const { return wraprange((T)0, xz, (T)this->size); }

private:
	WorldTile *tiles;
	Grid<int> distanceFromWaterMap;
};

extern World *gWorld;

} }