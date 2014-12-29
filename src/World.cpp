#include "TerrainStyle.h"
#include "World.h"

using namespace IntelOrca::PopSS;

const float World::TileSize = 128.0f;
const float World::OceanTileSize = TileSize / 1.0f;
const float World::SkyDomeRadius = 96.0f * TileSize;

World::World()
{
	this->tiles = NULL;
	this->distanceFromWaterMap = NULL;
}

World::~World()
{
	if (this->tiles != NULL)
		delete[] this->tiles;

	if (this->distanceFromWaterMap == NULL)
		delete[] this->distanceFromWaterMap;
}

void World::Reprocess()
{
	if (this->distanceFromWaterMap == NULL)
		delete[] this->distanceFromWaterMap;

	this->distanceFromWaterMap = GenerateDistanceFromWaterMap();
	for (int x = 0; x < this->size; x++)
		for (int z = 0; z < this->size; z++)
			ProcessTile(x, z);

	// LightSourceManager.Natural = LandscapeStyle.NaturalLight;
	// LightSourceManager.Sun = LandscapeStyle.SunLight;
	// LightSourceManager.Sun.Position *= SkyDomeRadius;
}

void World::ProcessTile(int x, int z)
{
	WorldTile *tile = &this->tiles[x + (z * this->size)];

	tile->lightNormal = CalculateNormal(x, z);
	tile->terrain = CalculateTerrain(x, z);
	tile->shore = IsShore(x, z);
}

int *World::GenerateDistanceFromWaterMap() const
{
	int *distanceMap = new int[this->size * this->size];
	int numTilesSet = 0;
	int totalNumTiles = this->size * this->size;

	struct xzdist { int x, z, distance; };

	std::queue<xzdist> tileQueue;
	for (int z = 0; z < this->size; z++) {
		for (int x = 0; x < this->size; x++) {
			if (this->tiles[x + (z * this->size)].height == 0) {
				numTilesSet++;
				distanceMap[x, z] = 0;
				tileQueue.push({ x, z, 0 });
			} else {
				// distanceMap[x, z] = -1;

				distanceMap[x, z] = 0;
			}
		}
	}
	return distanceMap;

	while (tileQueue.size() > 0 && numTilesSet < totalNumTiles) {
		xzdist tile = tileQueue.front();
		tileQueue.pop();

		for (int z = -1; z <= 1; z++) {
			for (int x = -1; x <= 1; x++) {
				int xx = MapWrap(tile.x + x);
				int zz = MapWrap(tile.z + z);

				if (distanceMap[xx, zz] != -1)
					continue;
				numTilesSet++;
				distanceMap[xx + (zz * this->size)] = tile.distance + 1;
				tileQueue.push({ xx, zz, tile.distance + 1 });
			}
		}
	}

	return distanceMap;
}

bool World::IsShore(int landX, int landZ) const
{
	int minHeight = 0, maxHeight = 0;

	for (int z = -1; z <= 1; z++) {
		for (int x = -1; x <= 1; x++) {
			int height = this->GetTile(landX + x, landZ + z)->height;
			minHeight = min(minHeight, height);
			maxHeight = max(maxHeight, height);
		}
	}

	return minHeight <= 0 && maxHeight > 0;
}

glm::vec3 World::CalculateNormal(int landX, int landZ) const
{
	return this->CalculateNormal(
		this->GetTile(landX, landZ)->height,
		this->GetTile(landX - 1, landZ)->height,
		this->GetTile(landX + 1, landZ)->height,
		this->GetTile(landX, landZ - 1)->height,
		this->GetTile(landX, landZ + 1)->height
	);
}

glm::vec3 World::CalculateNormal(float height, float westHeight, float eastHeight, float northHeight, float southHeight)
{
	double xMidAngle = CalculateMidAngle(westHeight, height, eastHeight);
	double zMidAngle = CalculateMidAngle(northHeight, height, southHeight);
	return glm::vec3(
		cos(xMidAngle) * sin(zMidAngle),
		sin(xMidAngle),
		cos(xMidAngle) * cos(zMidAngle)
	);
}

float World::CalculateMidAngle(float leftHeight, float midHeight, float rightHeight)
{
	glm::vec3 leftVector = glm::vec3(-TileSize, leftHeight - midHeight, 0);
	glm::vec3 rightVector = glm::vec3(TileSize, rightHeight - midHeight, 0);

	float leftAngle = atan2(leftVector.y, leftVector.x);
	float rightAngle = atan2(rightVector.y, rightVector.x);
	return leftAngle - abs(rightAngle - leftAngle) / 2.0f;
}

int World::GetSteepness(int landX, int landZ) const
{
	unsigned int minHeight = INT32_MAX, maxHeight = INT32_MIN;
	for (int z = -1; z <= 1; z++) {
		for (int x = -1; x <= 1; x++) {
			minHeight = min(minHeight, this->GetTile(landX + x, landZ + z)->height);
			maxHeight = max(maxHeight, this->GetTile(landX + x, landZ + z)->height);
		}
	}
	return maxHeight - minHeight;
}

int World::CalculateTerrain(int landX, int landZ) const
{
	int height = this->GetTile(landX, landZ)->height;
	int steepness = GetSteepness(landX, landZ);
	int distanceFromWater = this->distanceFromWaterMap[landX + (landZ * this->size)];

	for (int i = 0; i < this->numTerrainStyles; i++) {
		TerrainStyle *ts = &this->terrainStyles[i];

		if (height < ts->minHeight || height > ts->maxHeight)
			continue;
		if (steepness < ts->minSteepness || steepness > ts->maxSteepness)
			continue;
		if (distanceFromWater < ts->minDistanceFromWater || distanceFromWater > ts->maxDistanceFromWater)
			continue;

		return i;
	}

	return 255;
}

void World::LoadLandFromPOPTB(const char *path)
{
	unsigned short *heightMap = new unsigned short[128 * 128];

	FILE *file = fopen(path, "rb");
	if (file == NULL)
		exit(-1);

	fread(heightMap, 128 * 128 * 2, 1, file);
	fclose(file);

	this->size = 256;
	this->tiles = new WorldTile[256 * 256];
	for (int z = 0; z < 256; z++) {
		for (int x = 0; x < 256; x++) {
			if (x % 2 == 0 && z % 2 == 0) {
				this->GetTile(x, z)->height = heightMap[(x / 2) + ((z / 2) * 128)];
			} else if (x % 2 != 0 && z % 2 == 0) {
				int hmx = (x / 2);
				int hmz = (z / 2);
				int height = heightMap[(hmx % 128) + ((hmz % 128) * 128)] + heightMap[((hmx + 1) % 128) + ((hmz % 128) * 128)];
				this->GetTile(x, z)->height = height / 2;
			} else if (x % 2 == 0 && z % 2 != 0) {
				int hmx = (x / 2);
				int hmz = (z / 2);
				int height = heightMap[(hmx % 128) + ((hmz % 128) * 128)] + heightMap[(hmx % 128) + (((hmz + 1) % 128) * 128)];
				this->GetTile(x, z)->height = height / 2;
			} else {
				this->GetTile(x, z)->height = 0;
			}
		}
	}

	delete[] heightMap;

	for (int z = 0; z < 256; z++) {
		for (int x = 0; x < 256; x++) {
			if (x % 2 != 0 && z % 2 != 0) {
				int height = 0;
				for (int zz = -1; zz <= 1; zz++)
					for (int xx = -1; xx <= 1; xx++)
						height += this->GetTile(x + xx, z + zz)->height;
				this->GetTile(x, z)->height = height / 8;
			}
		}
	}

	this->Reprocess();
}

WorldTile *World::GetTile(int x, int z) const
{
	return &this->tiles[this->MapWrap(x) + (this->MapWrap(z) * this->size)];
}