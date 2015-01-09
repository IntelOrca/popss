#include "Objects/Units/Unit.h"
#include "Objects/WorldObject.h"
#include "TerrainStyle.h"
#include "World.h"

using namespace IntelOrca::PopSS;

World *IntelOrca::PopSS::gWorld;

const int World::TileSize = 128;
const float World::OceanTileSize = TileSize / 1.0f;
const float World::SkyDomeRadius = 96.0f * TileSize;

World::World()
{
	this->tiles = NULL;

	this->numTerrainStyles = 6;
	this->terrainStyles = new TerrainStyle[this->numTerrainStyles];

	// Cliff
	this->terrainStyles[0].textureIndex = 3;
	this->terrainStyles[0].minSteepness = 256;
	
	// Sand
	this->terrainStyles[1].textureIndex = 0;
	this->terrainStyles[1].maxHeight = 256;
	this->terrainStyles[1].maxDistanceFromWater = 3;

	// Low grass
	this->terrainStyles[2].textureIndex = 1;
	this->terrainStyles[2].maxHeight = 256;
	this->terrainStyles[2].diffuseReflectivity = 0.75f;

	// Dirt
	this->terrainStyles[3].textureIndex = 4;
	this->terrainStyles[3].maxHeight = 512;

	// High grass
	this->terrainStyles[4].textureIndex = 1;
	this->terrainStyles[4].maxHeight = 768;

	// Snow
	this->terrainStyles[5].textureIndex = 2;

	// Morning
	this->lightManager.natural = {
		glm::vec3(0.0f, 2048.0f, 0.0f),
		glm::vec3(0.2f),
		glm::vec3(0.6f),
		glm::vec3(0.0f),
		0
	};
	this->lightManager.sun = {
		glm::vec3(-1.0f, 0.1f, 1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.8f),
		glm::vec3(0.25f),
		0
	};
	this->skyColour = glm::vec3(0.5, 0.5, 1.0f);

	return;

	// Sunset
	this->lightManager.natural = {
		glm::vec3(0.0f, 2048.0f, 0.0f),
		glm::vec3(0.05f),
		glm::vec3(0.4f),
		glm::vec3(0.0f),
		0
	};
	this->lightManager.sun = {
		glm::vec3(-1.0f, 0.1f, 1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.5f, 0.5f, 0.0f),
		glm::vec3(0.25f, 0.25f, 0.0f),
		0
	};
	this->skyColour = glm::vec3(0.5f, 0.5f, 1.0f);

	// Dark
	this->lightManager.natural = {
		glm::vec3(0.0f, 2048.0f, 0.0f),
		glm::vec3(0.05f),
		glm::vec3(0.25f),
		glm::vec3(0.0f),
		0
	};
	this->lightManager.sun = {
		glm::vec3(-1.0f, 0.25f, 1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.25f),
		glm::vec3(0.25f),
		0
	};
	this->skyColour = glm::vec3(0.1f, 0.1f, 0.4f);
}

World::~World()
{
	if (this->tiles != NULL)
		delete[] this->tiles;
}

void World::Update()
{
	for (WorldObject *obj : this->objects)
		obj->Update();
}

void World::Reprocess()
{
	GenerateDistanceFromWaterMap();
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

void World::GenerateDistanceFromWaterMap()
{
	struct xzdist { int x, z, distance; };

	int mapSize = this->size;

	Grid<int> distanceMap = Grid<int>(mapSize);
	int numTilesSet = 0;
	int totalNumTiles = mapSize * mapSize;

	// An array is used as std::queue was far too slow
	xzdist *tileQueue = new xzdist[mapSize * mapSize];
	int tileQueueFrontIndex = 0;
	int tileQueueBackIndex = 0;

	// Initialise queue and distance map with water tiles
	for (int z = 0; z < mapSize; z++) {
		for (int x = 0; x < mapSize; x++) {
			if (this->tiles[x + (z * mapSize)].height == 0) {
				numTilesSet++;
				distanceMap.Set(x, z, 0);
				tileQueue[tileQueueBackIndex++] = { x, z, 0 };
			} else {
				distanceMap.Set(x, z, -1);
			}
		}
	}

	// Breath first search until all tiles are set
	while (tileQueueFrontIndex != tileQueueBackIndex && numTilesSet < totalNumTiles) {
		xzdist tile = tileQueue[tileQueueFrontIndex++];

		for (int z = -1; z <= 1; z++) {
			for (int x = -1; x <= 1; x++) {
				int xx = TileWrap(tile.x + x);
				int zz = TileWrap(tile.z + z);

				if (distanceMap.Get(xx, zz) != -1)
					continue;
				numTilesSet++;
				distanceMap.Set(xx, zz, tile.distance + 1);
				tileQueue[tileQueueBackIndex++] = { xx, zz, tile.distance + 1 };
				// tileQueue.push({ xx, zz, tile.distance + 1 });
			}
		}
	}
	
	delete[] tileQueue;
	this->distanceFromWaterMap = distanceMap;
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
	unsigned int minHeight = UINT32_MAX, maxHeight = 0;
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
	int distanceFromWater = this->distanceFromWaterMap.Get(landX, landZ);

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

#include "Objects/Units/Wildman.h"
#include "Objects/Units/Shaman.h"
#include "Objects/Buildings/VaultOfKnowledge.h"
#include "Objects/Scenery/Tree.h"

void World::LoadLandFromPOPTB(const char *path)
{
	Grid<uint16> heightMap = Grid<uint16>(128);

	FILE *file = fopen(path, "rb");
	if (file == NULL)
		exit(-1);

	fread(heightMap.GetData(), heightMap.GetDataSize(), 1, file);
	fseek(file, 0x14043, SEEK_SET);
	for (int i = 0; i < 2000; i++) {
		unsigned char objdata[55];
		fread(objdata, sizeof(objdata), 1, file);

		WorldObject *obj = NULL;

		if (objdata[0] == 1 && objdata[1] == 1) {
			Wildman *wildman = new Wildman();
			wildman->ownership = OWNERSHIP_NEUTRAL;
			obj = wildman;
		}

		if (objdata[0] == 1 && objdata[1] == 1) {
			Wildman *wildman = new Wildman();
			wildman->ownership = OWNERSHIP_NEUTRAL;
			obj = wildman;
		}

		if (objdata[0] >= 1 && objdata[0] <= 3 && objdata[1] == 5) {
			Tree *tree = new Tree();
			tree->type = objdata[0] - 1;
			tree->ownership = OWNERSHIP_NEUTRAL;
			obj = tree;
		}

		if (objdata[0] == 7 && objdata[1] == 1) {
			obj = new Shaman();
		}

		if (objdata[0] == 18 && objdata[1] == 2) {
			VaultOfKnowledge *vok = new VaultOfKnowledge();
			vok->rotation = objdata[8] * 32;
			obj = vok;
		}

		if (obj != NULL) {
			obj->ownership = objdata[2];
			obj->x = objdata[4] * World::TileSize;
			obj->z = (255 - objdata[6]) * World::TileSize;
			this->objects.push_back(obj);
		}
	}
	fclose(file);

	for (int z = 0; z < 64; z++) {
		for (int x = 0; x < 128; x++) {
			int tmp = heightMap.Get(x, z);
			heightMap.Set(x, z, heightMap.Get(x, 127 - z));
			heightMap.Set(x, 127 - z, tmp);
		}
	}

	this->size = 256;
	this->sizeSquared = this->size * this->size;
	this->sizeByNonTiles = this->size * World::TileSize;

	this->tiles = new WorldTile[this->size * this->size];
	for (int j = 0; j < this->size; j++) {
		for (int i = 0; i < this->size; i++) {
			double u = i / (double)this->size;
			double v = j / (double)this->size;
			u = u * 128 - 0.5;
			v = v * 128 - 0.5;
			int x = floor(u);
			int y = floor(v);
			double u_ratio = u - x;
			double v_ratio = v - y;
			double u_opposite = 1 - u_ratio;
			double v_opposite = 1 - v_ratio;

			int x0 = (x + 128) % 128;
			int y0 = (y + 128) % 128;
			int x1 = ((x + 1) + 128) % 128;
			int y1 = ((y + 1) + 128) % 128;
			double result = (heightMap.Get(x0, y0) * u_opposite + heightMap.Get(x1, y0) * u_ratio) * v_opposite + 
							(heightMap.Get(x0, y1) * u_opposite + heightMap.Get(x1, y1) * u_ratio) * v_ratio;
			
			this->GetTile(i, j)->height = (int)result;

			// if (x % 2 == 0 && z % 2 == 0) {
			// 	this->GetTile(x, z)->height = heightMap[(x / 2) + ((z / 2) * 128)];
			// } else if (x % 2 != 0 && z % 2 == 0) {
			// 	int hmx = (x / 2);
			// 	int hmz = (z / 2);
			// 	int height = heightMap[(hmx % 128) + ((hmz % 128) * 128)] + heightMap[((hmx + 1) % 128) + ((hmz % 128) * 128)];
			// 	this->GetTile(x, z)->height = height / 2;
			// } else if (x % 2 == 0 && z % 2 != 0) {
			// 	int hmx = (x / 2);
			// 	int hmz = (z / 2);
			// 	int height = heightMap[(hmx % 128) + ((hmz % 128) * 128)] + heightMap[(hmx % 128) + (((hmz + 1) % 128) * 128)];
			// 	this->GetTile(x, z)->height = height / 2;
			// } else {
			// 	this->GetTile(x, z)->height = 0;
			// }
		}
	}

	// for (int z = 0; z < 256; z++) {
	// 	for (int x = 0; x < 256; x++) {
	// 		if (x % 2 != 0 && z % 2 != 0) {
	// 			int height = 0;
	// 			for (int zz = -1; zz <= 1; zz++)
	// 				for (int xx = -1; xx <= 1; xx++)
	// 					height += this->GetTile(x + xx, z + zz)->height;
	// 			this->GetTile(x, z)->height = height / 8;
	// 		}
	// 	}
	// }

	this->Reprocess();
}

WorldTile *World::GetTile(int x, int z) const
{
	return &this->tiles[this->TileWrap(x) + (this->TileWrap(z) * this->size)];
}

int World::GetHeight(int x, int z) const
{
	int modx = x % World::TileSize;
	int modz = z % World::TileSize;

	if (modx == 0 && modz == 0)
		return GetTile(x / World::TileSize, z / World::TileSize)->height;

	int landx0 = x / World::TileSize;
	int landz0 = z / World::TileSize;
	int landx1 = landx0 + 1;
	int landz1 = landz0 + 1;

	int height00 = GetTile(landx0, landz0)->height;
	int height01 = GetTile(landx0, landz1)->height;
	int height10 = GetTile(landx1, landz0)->height;
	int height11 = GetTile(landx1, landz1)->height;

	if (modx + modz < World::TileSize) {
		// Top left triangle
		int heightX = ((height10 - height00) * modx) / World::TileSize;
		int heightZ = ((height01 - height00) * modz) / World::TileSize;
		return height00 + heightX + heightZ;
	} else {
		// Bottom right triangle
		int heightX = ((height01 - height11) * (World::TileSize - modx)) / World::TileSize;
		int heightZ = ((height10 - height11) * (World::TileSize - modz)) / World::TileSize;
		return height11 + heightX + heightZ;
	}
}