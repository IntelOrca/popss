#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

typedef void (*RenderQuadFunc)(int, int, float, float);

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
	unsigned char debugRenderType;

	LandscapeRenderer();
	~LandscapeRenderer();

	void Initialise();
	void Render(const Camera *camera);

private:
	int landViewSize;
	int oceanViewSize;

	float time;

	glm::mat4 projectionMatrix;
	glm::mat4 modelViewMatrix;

	OrcaShader *landShader;
	OrcaShader *waterShader;

	GLuint landVAO;
	GLuint landVBO;
	std::vector<LandVertex> landVertices;

	GLuint waterVAO;
	GLuint waterVBO;
	std::vector<WaterVertex> waterVertices;

	LandWaterShaderUniform landShaderUniform;
	LandWaterShaderUniform waterShaderUniform;

	GLuint terrainTextures[8];
	GLuint waterTexture;

	void RenderArea(const Camera *camera, int viewSize, bool water);

	void RenderLand(const Camera *camera);
	void RenderLandQuad(int landX, int landZ, float x, float z);
	void RenderLandTriangle000111(float vx, float vz, int landX, int landZ);
	void RenderLandTriangle001110(float vx, float vz, int landX, int landZ);
	void RenderLandTriangle(float vx, float vz, int landX, int landZ, const int *offsetsX, const int *offsetsZ, const glm::vec2 *uv);

	void RenderOcean(const Camera *camera);
	void RenderOceanQuad(int landX, int landZ, float vx, float vz);
	void RenderOceanTriangle000111(float vx, float vz, int oceanX, int oceanZ);
	void RenderOceanTriangle001110(float vx, float vz, int oceanX, int oceanZ);
	void RenderOceanTriangle(float vx, float vz, int oceanX, int oceanZ, const int *offsetsX, const int *offsetsZ, const glm::vec2 *uv);
	float GetWaveHeight(int oceanX, int oceanZ);

	void RenderObjects(const Camera *camera);

	void AddVertex(const glm::vec3 &position, const glm::vec2 &uv, const WorldTile *tile);
	void AddVertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &uv, int texture, const glm::vec4 &material);
	glm::vec2 GetTextureUV(int landX, int landZ);
	glm::vec4 GetColourFromLand(int height);

	void SetLightSources(const Camera *camera, OrcaShader *shader);
};

} }