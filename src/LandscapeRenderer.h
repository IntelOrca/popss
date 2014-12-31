#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

typedef void (*RenderQuadFunc)(int, int, float, float);

struct LandVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoords;
	float texture;
	glm::vec4 material;
};

class Camera;
class OrcaShader;
class VertexBuffer;
class World;
class WorldTile;
class LandscapeRenderer {
public:
	World *world;

	LandscapeRenderer();
	~LandscapeRenderer();

	void Initialise();
	void Render(const Camera *camera);

private:
	static const float SphereRatio;
	static const float TextureMapSize;

	int landViewSize;
	int oceanViewSize;

	glm::mat4 projectionMatrix;
	glm::mat4 modelViewMatrix;

	OrcaShader *landShader;
	VertexBuffer *landVertexBuffer;

	GLuint terrainTextures[8];

	void LandscapeRenderer::RenderArea(const Camera *camera, int viewSize, bool water);

	void RenderLand(const Camera *camera);
	void RenderLandQuad(int landX, int landZ, float x, float z);
	void RenderLandTriangle000111(float vx, float vz, int landX, int landZ);
	void RenderLandTriangle001110(float vx, float vz, int landX, int landZ);
	void RenderLandTriangle(float vx, float vz, int landX, int landZ, const int *offsetsX, const int *offsetsZ, const glm::vec2 *uv);

	void AddVertex(glm::vec3 position, glm::vec2 uv, const WorldTile *tile);
	void AddVertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv, int texture, const glm::vec4 &material);
	glm::vec2 GetTextureUV(int landX, int landZ);
	glm::vec4 GetColourFromLand(int height);
};

} }