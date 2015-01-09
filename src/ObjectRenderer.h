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
class WorldObject;
class ObjectRenderer {
public:
	World *world;
	unsigned char debugRenderType;

	ObjectRenderer();
	~ObjectRenderer();

	void Initialise();
	void Render(const Camera *camera);

private:
	unsigned char lastDebugRenderType;

	std::vector<WorldObject*> visibleObjects;

	Mesh *unitMesh;
	Mesh *treeMesh[3];

	Mesh *redGuardTowerMesh[2];
	Mesh *vokMesh;

	GLuint arrowTexture;
	GLuint frameTexture;
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

	void InitialiseShader();

	void RenderObjectGroups(const Camera *camera);
	void RenderObjectGroup(const Camera *camera, WorldObject **objects, int count);

	void UpdateVisibleObjects(const Camera *camera);
	bool IsObjectVisible(const Camera *camera, WorldObject *obj);

	glm::vec3 GetObjectTranslationRelativeToCamera(const Camera *camera, const WorldObject *obj);

	void PrepareMesh(const Mesh *mesh);
	void RenderVertices();

	void RenderUnitSelectionArrows(const Camera *camera);
};

} }