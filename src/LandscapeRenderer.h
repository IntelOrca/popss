#pragma once

#include "PopSS.h"
#include "SimpleVertexBuffer.hpp"

namespace IntelOrca { namespace PopSS {

struct LandVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoords;
	unsigned char texture;
	glm::vec4 material;
};

struct WaterVertex {
	glm::vec3 position;
};

enum DEBUG_LANDSCAPE_RENDER_TYPE {
	DEBUG_LANDSCAPE_RENDER_TYPE_NONE,
	DEBUG_LANDSCAPE_RENDER_TYPE_WIREFRAME,
	DEBUG_LANDSCAPE_RENDER_TYPE_POINTS
};

struct LandWaterShaderUniform {
	GLint projectionMatrix;
	GLint viewMatrix;
	GLint modelMatrix;

	GLint sphereRatio;
	GLint cameraTarget;
	GLint highlightActive;
	GLint highlight00;
	GLint highlight11;
};

class Camera;
class LightSource;
class OrcaShader;
class World;
class WorldTile;
class LandscapeRenderer {
public:
	static const float SphereRatio;
	static const float TextureMapSize;

	World *world;
	unsigned char lastDebugRenderType, debugRenderType;

	LandscapeRenderer();
	~LandscapeRenderer();

	void Initialise();
	void Render(const Camera *camera);

	void SetDirtyTile(int x, int z);
	void SetDirtyTile(int x0, int z0, int x1, int z1);

private:
	// Shared
	glm::mat4 projectionMatrix;
	glm::mat4 modelViewMatrix;

	GLuint shadowTexture;

	void GenerateShadowTexture();

	bool *dirtyLandBlocks;
	bool *dirtyWaterBlocks;

	void UpdateDirtyBlocks();

	// Land
	int landViewSize;

	int landBlocksPerRow;
	int numLandBlocks;
	int totalLandVertexBufferSize;
	LandVertex *landVertices;
	GLuint glLandVBO;
	GLuint glLandVAO;
	GLuint glLandIndexVBO;
	std::vector<uint32> *landVertexIndices;

	OrcaShader *landShader;
	LandWaterShaderUniform landShaderUniform;
	GLuint terrainTextures[8];

	void InitialiseLandShader();

	void InitialiseLandBlocks();
	void UpdateLandAllSubBlocks();
	void UpdateLandSubBlock(int blockX, int blockZ);
	void UpdateLandSubBlockTileIndices(std::vector<uint32> *blockIndices, int landX, int landZ, int baseIndex);
	void GetLandVertex(int landX, int landZ, LandVertex *topLeft, LandVertex *centre);

	void RenderLand(const Camera *camera);
	void DrawVisibleLandBlocks(const Camera *camera);
	void DrawLandSubBlock(int blockX, int blockZ);

	int GetLandBlockBaseVertexIndex(int blockX, int blockZ) const;
	int GetLandBlockBaseVertexIndexIndex(int blockX, int blockZ) const;
	int GetLandBlockVertexIndex(int x, int z) const;

	// Water
	int waterBlocksPerRow;
	int numWaterBlocks;
	int totalWaterVertexBufferSize;
	WaterVertex *waterVertices;
	GLuint glWaterVBO;
	GLuint glWaterVAO;
	GLuint glWaterIndexVBO;
	std::vector<uint32> *waterVertexIndices;

	OrcaShader *waterShader;
	LandWaterShaderUniform waterShaderUniform;

	int oceanViewSize;
	float time;

	void InitialiseWaterShader();

	void InitialiseWaterBlocks();
	void UpdateWaterAllSubBlocks();
	void UpdateWaterSubBlock(int blockX, int blockZ);
	void UpdateWaterSubBlockTileIndices(std::vector<uint32> *blockIndices, int landX, int landZ, int baseIndex);
	void GetWaterVertex(int landX, int landZ, WaterVertex *topLeft);

	void RenderWater(const Camera *camera);
	void DrawVisibleWaterBlocks(const Camera *camera);
	void DrawWaterSubBlock(int blockX, int blockZ);

	int GetWaterBlockBaseVertexIndex(int blockX, int blockZ) const;
	int GetWaterBlockBaseVertexIndexIndex(int blockX, int blockZ) const;
	int GetWaterBlockVertexIndex(int x, int z) const;
};

} }