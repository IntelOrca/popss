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
	glm::vec3 normal;
	glm::vec2 texcoords;
};

enum DEBUG_LANDSCAPE_RENDER_TYPE {
	DEBUG_LANDSCAPE_RENDER_TYPE_NONE,
	DEBUG_LANDSCAPE_RENDER_TYPE_WIREFRAME,
	DEBUG_LANDSCAPE_RENDER_TYPE_POINTS
};

struct LandWaterShaderUniform {
	GLint projectionMatrix;
	GLint modelViewMatrix;
	GLint sphereRatio;
	GLint cameraTarget;
	GLint highlightActive;
	GLint highlight00;
	GLint highlight11;
};

class Camera;
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

private:
	// Shared
	glm::mat4 projectionMatrix;
	glm::mat4 modelViewMatrix;

	void SetLightSources(const Camera *camera, OrcaShader *shader);

	// Land
	int landViewSize;

	OrcaShader *landShader;
	SimpleVertexBuffer<LandVertex> *landVertexBuffer;
	LandWaterShaderUniform landShaderUniform;
	GLuint terrainTextures[8];
	GLuint shadowTexture;

	void InitialiseLandShader();
	void GenerateShadowTexture();

	void RenderLand(const Camera *camera);
	void UpdateLandPrimitives(const Camera *camera);
	void AddLandQuad(int landX, int landZ, float x, float z);
	void AddLandTriangle(float x, float z, int landX, int landZ, const float *offsetsX, const float *offsetsZ);
	void AddLandVertex(const glm::vec3 &position, const glm::vec2 &uv, const WorldTile *tile);
	void AddLandVertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &uv, int texture, const glm::vec4 &material);
	glm::vec2 GetTextureUV(int landX, int landZ);
	glm::vec4 GetColourFromLand(int height);

	// Water
	int oceanViewSize;
	float time;

	OrcaShader *waterShader;
	SimpleVertexBuffer<WaterVertex> *waterVertexBuffer;
	LandWaterShaderUniform waterShaderUniform;
	GLuint waterTexture;

	void InitialiseWaterShader();
	void RenderWater(const Camera *camera);
	void UpdateWaterPrimitives(const Camera *camera);
};

} }