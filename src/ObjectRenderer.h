#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

struct ObjectVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

class Camera;
class Mesh;
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
	Mesh *vokMesh;

	OrcaShader *objectShader = NULL;
	GLuint objectVAO;
	GLuint objectVBO;
	std::vector<ObjectVertex> objectVertices;

	struct {
		GLint projectionMatrix;
		GLint viewMatrix;
		GLint modelMatrix;
		GLint sphereRatio;
		GLint cameraTarget;
	} objectShaderUniforms;

	void PrepareMesh(const Mesh *mesh);
	void RenderVertices();
	static void ObjectRenderer::SetLightSources(const Camera *camera, OrcaShader *shader);
};

} }