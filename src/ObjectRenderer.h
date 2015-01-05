#pragma once

#include "PopSS.h"
#include "SimpleVertexBuffer.hpp"

namespace IntelOrca { namespace PopSS {

struct ObjectVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoords;
};

class Camera;
class Mesh;
class OrcaShader;
class World;
class ObjectRenderer {
public:
	World *world;
	unsigned char debugRenderType;

	ObjectRenderer();
	~ObjectRenderer();

	void Initialise();
	void Render(const Camera *camera);

private:
	Mesh *unitMesh;

	Mesh *vokMesh;
	GLuint vokTexture;

	OrcaShader *objectShader = NULL;
	SimpleVertexBuffer<ObjectVertex> *objectVertexBuffer;

	struct {
		GLint projectionMatrix;
		GLint viewMatrix;
		GLint modelMatrix;
		GLint sphereRatio;
		GLint cameraTarget;
		GLint texture;
	} objectShaderUniforms;

	void PrepareMesh(const Mesh *mesh);
	void RenderVertices();

	static void ObjectRenderer::SetLightSources(const Camera *camera, OrcaShader *shader);
};

} }