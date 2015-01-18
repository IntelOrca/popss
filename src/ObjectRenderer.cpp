#include "Camera.h"
#include "LandscapeRenderer.h"
#include "LightSource.h"
#include "Mesh.h"
#include "ObjectRenderer.h"
#include "OrcaShader.h"
#include "World.h"
#include "Objects/WorldObject.h"
#include "Objects/Units/Unit.h"
#include "Objects/Buildings/Building.h"
#include "Objects/Scenery/Tree.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace IntelOrca::PopSS;

bool LoadTexture(GLuint texture, const char *path);

const VertexAttribPointerInfo ObjectShaderVertexInfo[] = {
	{ "VertexPosition",			GL_FLOAT,	3,	offsetof(ObjectVertex, position)	},
	{ "VertexNormal",			GL_FLOAT,	3,	offsetof(ObjectVertex, normal)		},
	{ "VertexTextureCoords",	GL_FLOAT,	2,	offsetof(ObjectVertex, texcoords)	},
	{ NULL }
};

ObjectRenderer::ObjectRenderer()
{
	this->debugRenderType = DEBUG_LANDSCAPE_RENDER_TYPE_NONE;
	this->lastDebugRenderType = this->debugRenderType;

	this->objectShader = NULL;
	this->objectVertexBuffer = NULL;

	this->unitMesh = NULL;
	for (int i = 0; i < 3; i++)
		this->treeMesh[i] = NULL;
	this->redGuardTowerMesh[0] = NULL;
	this->redGuardTowerMesh[1] = NULL;
	this->vokMesh = NULL;
}

ObjectRenderer::~ObjectRenderer()
{
	SafeDelete(this->objectShader);
	SafeDelete(this->objectVertexBuffer);

	SafeDelete(this->unitMesh);
	SafeDelete(this->redGuardTowerMesh[0]);
	SafeDelete(this->redGuardTowerMesh[1]);
	SafeDelete(this->vokMesh);
}

void ObjectRenderer::Initialise()
{
	this->InitialiseShader();

	this->unitMesh = Mesh::FromObjectFile("data/objects/unit.object");
	this->treeMesh[0] = Mesh::FromObjectFile("data/objects/tree0.object");
	this->treeMesh[1] = Mesh::FromObjectFile("data/objects/tree1.object");
	this->treeMesh[2] = Mesh::FromObjectFile("data/objects/tree2.object");

	this->redGuardTowerMesh[0] = Mesh::FromObjectFile("data/objects/tower_red.0.object");
	this->redGuardTowerMesh[1] = Mesh::FromObjectFile("data/objects/tower_red.object");
	this->vokMesh = Mesh::FromObjectFile("data/objects/vok.object");

	glGenTextures(1, &this->arrowTexture);
	LoadTexture(this->arrowTexture, "data/textures/arrow.png");

	glGenTextures(1, &this->frameTexture);
	LoadTexture(this->frameTexture, "data/objects/tower_red.0.png");

	glGenTextures(1, &this->vokTexture);
	LoadTexture(this->vokTexture, "data/objects/vok.png");
}

void ObjectRenderer::InitialiseShader()
{
	SafeDelete(this->objectShader);

	this->objectShader = OrcaShader::FromPath(
		"object.vert",
		this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_NONE ?
			"object.frag" :
			"land_wireframe.frag"
	);
	this->objectShaderUniforms.projectionMatrix = this->objectShader->GetUniformLocation("ProjectionMatrix");
	this->objectShaderUniforms.viewMatrix = this->objectShader->GetUniformLocation("ViewMatrix");
	this->objectShaderUniforms.modelMatrix = this->objectShader->GetUniformLocation("ModelMatrix");
	this->objectShaderUniforms.sphereRatio = this->objectShader->GetUniformLocation("InputSphereRatio");
	this->objectShaderUniforms.cameraTarget = this->objectShader->GetUniformLocation("InputCameraTarget");
	this->objectShaderUniforms.texture = this->objectShader->GetUniformLocation("InputTexture");
	this->objectVertexBuffer = new SimpleVertexBuffer<ObjectVertex>(this->objectShader, ObjectShaderVertexInfo);
}

void ObjectRenderer::Render(const Camera *camera)
{
	if (this->debugRenderType != this->lastDebugRenderType)
		InitialiseShader();

	glm::mat4 pmatrix = camera->Get3dProjectionMatrix();
	glm::mat4 vmatrix = camera->Get3dViewMatrix();

	// Use shader
	this->objectShader->Use();

	// Set common uniforms
	glUniformMatrix4fv(this->objectShaderUniforms.projectionMatrix, 1, GL_FALSE, glm::value_ptr(pmatrix));
	glUniformMatrix4fv(this->objectShaderUniforms.viewMatrix, 1, GL_FALSE, glm::value_ptr(vmatrix));
	glUniform1f(this->objectShaderUniforms.sphereRatio, LandscapeRenderer::SphereRatio);
	glUniform3fv(this->objectShaderUniforms.cameraTarget, 1, glm::value_ptr(camera->target));
	glUniform1i(this->objectShaderUniforms.texture, 0);
	this->world->lightManager.SetLightSources(camera, this->objectShader);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	UpdateVisibleObjects(camera);

	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE) {
		glUniform4f(this->objectShader->GetUniformLocation("uColour"), 0, 0, 0, 1);
		RenderObjectGroups(camera);
	}
	
	switch (this->debugRenderType) {
	case DEBUG_LANDSCAPE_RENDER_TYPE_NONE:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case DEBUG_LANDSCAPE_RENDER_TYPE_WIREFRAME:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case DEBUG_LANDSCAPE_RENDER_TYPE_POINTS:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	}

	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE)
		glUniform4f(this->objectShader->GetUniformLocation("uColour"), 0.75f, 0.75f, 0.75f, 1);

	RenderObjectGroups(camera);

	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	RenderUnitSelectionArrows(camera);

	this->lastDebugRenderType = this->debugRenderType;
}

void ObjectRenderer::RenderObjectGroups(const Camera *camera)
{
	int numVisibleObjects = this->visibleObjects.size();
	if (numVisibleObjects == 0)
		return;

	WorldObject **first = &this->visibleObjects[0];
	int count = 1;

	for (int i = 1; i < numVisibleObjects; i++) {
		WorldObject **obj = &this->visibleObjects[i];
		if ((*obj)->type == (*first)->type && (*obj)->group == (*first)->group) {
			count++;
		} else {
			this->RenderObjectGroup(camera, first, count);
			first = obj;
			count = 1;
		}
	}

	// Last group
	this->RenderObjectGroup(camera, first, count);
}

void ObjectRenderer::RenderObjectGroup(const Camera *camera, WorldObject **objects, int count)
{
	const WorldObject *obj = objects[0];

	glEnable(GL_CULL_FACE);

	switch (obj->group) {
	case OBJECT_GROUP_UNIT:
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->vokTexture);
		PrepareMesh(this->unitMesh);
		break;
	case OBJECT_GROUP_BUILDING:
		switch (obj->type) {
		case BUILDING_GUARD_TOWER:
			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->frameTexture);
			PrepareMesh(this->redGuardTowerMesh[0]);
			break;
		case BUILDING_VAULT_OF_KNOWLEDGE:
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->vokTexture);
			PrepareMesh(this->vokMesh);
			break;
		}
		break;
	case OBJECT_GROUP_SCENERY:
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->vokTexture);
		if (obj->type >= SCENERY_TREE0 && obj->type <= SCENERY_TREE2)
			PrepareMesh(this->treeMesh[obj->type - SCENERY_TREE0]);
		break;
	}

	for (int i = 0; i < count; i++, obj = objects[i]) {
		glm::mat4 mmatrix;
		mmatrix = glm::translate(mmatrix, GetObjectTranslationRelativeToCamera(camera, obj));
		mmatrix = glm::rotate(mmatrix, (obj->rotation / 128.0f) * (float)M_PI, glm::vec3(0, 1, 0));

		switch (obj->group) {
		case OBJECT_GROUP_UNIT:
			mmatrix = glm::scale(mmatrix, glm::vec3(0.5 * World::TileSize));
			break;
		case OBJECT_GROUP_BUILDING:
			// if (obj->type == BUILDING_VAULT_OF_KNOWLEDGE) {
				mmatrix = glm::scale(mmatrix, glm::vec3(2 * World::TileSize));
			// }
			break;
		case OBJECT_GROUP_SCENERY:
			if (obj->type >= SCENERY_TREE0 && obj->type <= SCENERY_TREE2) {
				mmatrix = glm::scale(mmatrix, glm::vec3(2 * World::TileSize));
			}
			break;
		}

		glUniformMatrix4fv(this->objectShaderUniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(mmatrix));
		RenderVertices();
	}
}

void ObjectRenderer::UpdateVisibleObjects(const Camera *camera)
{
	this->visibleObjects.clear();
	for (WorldObject *obj : this->world->objects)
		if (this->IsObjectVisible(camera, obj))
			this->visibleObjects.push_back(obj);

	std::sort(this->visibleObjects.begin(), this->visibleObjects.end(), [](WorldObject *a, WorldObject *b) -> bool {
		if (a->group != b->group) return a->group < b->group;
		return a->type < b->type;
	});
}

bool ObjectRenderer::IsObjectVisible(const Camera *camera, WorldObject *obj)
{
	glm::ivec2 delta = this->world->GetClosestDelta(obj->x, obj->z, camera->target.x, camera->target.z);
	int distance = sqrt(delta.x * delta.x + delta.y * delta.y);
	return distance < 128 * World::TileSize;
}

glm::vec3 ObjectRenderer::GetObjectTranslationRelativeToCamera(const Camera *camera, const WorldObject *obj)
{
	glm::ivec3 cameraPosition = glm::ivec3(camera->target);

	int translateX = 0;
	int translateZ = 0;

	int distanceXa = abs(obj->x - cameraPosition.x);
	int distanceXb = world->sizeByNonTiles - distanceXa;
	int distanceZa = abs(obj->z - cameraPosition.z);
	int distanceZb = world->sizeByNonTiles - distanceZa;

	if (distanceXb < distanceXa) {
		if (obj->x > cameraPosition.x) translateX -= this->world->sizeByNonTiles;
		else translateX += this->world->sizeByNonTiles;
	}
	if (distanceZb < distanceZa) {
		if (obj->z > cameraPosition.z) translateZ -= this->world->sizeByNonTiles;
		else translateZ += this->world->sizeByNonTiles;
	}

	return glm::vec3(obj->position + glm::ivec3(translateX, 0, translateZ));
}

void ObjectRenderer::PrepareMesh(const Mesh *mesh)
{
	this->objectVertexBuffer->Clear();
	for (int i = 0; i < mesh->numFaces; i++) {
		const Mesh::Face *face = &mesh->faces[i];
		glm::vec3 vertices[3];
		glm::vec2 texcoords[3];
		glm::vec3 normal[3];
		
		for (int j = 0; j < 3; j++) {
			vertices[j] = mesh->vertices[face->vertex[j].position];
			texcoords[j] = face->vertex[j].texture == -1 ? glm::vec2(0) : mesh->textureCoordinates[face->vertex[j].texture];
			normal[j] = face->vertex[j].normal == -1 ? glm::vec3(0) : mesh->normals[face->vertex[j].normal];
		}

		for (int j = 0; j < 3; j++) {
			ObjectVertex vertex;
			vertex.position = vertices[j];
			vertex.normal = normal[j];
			vertex.texcoords = texcoords[j];

			vertex.texcoords.t = 1 - vertex.texcoords.t;

			this->objectVertexBuffer->Add(vertex);
		}
	}
	this->objectVertexBuffer->Update();
}

void ObjectRenderer::RenderVertices()
{
	this->objectVertexBuffer->Draw(GL_TRIANGLES);
}

void ObjectRenderer::RenderUnitSelectionArrows(const Camera *camera)
{
	ObjectVertex vertices[6] = {
		{ { -0.5, +0.5, 0.0 }, { 0, 1, 0 }, { 0, 0 } },
		{ { -0.5, -0.5, 0.0 }, { 0, 1, 0 }, { 0, 1 } },
		{ { +0.5, +0.5, 0.0 }, { 0, 1, 0 }, { 1, 0 } },
		{ { +0.5, +0.5, 0.0 }, { 0, 1, 0 }, { 1, 0 } },
		{ { -0.5, -0.5, 0.0 }, { 0, 1, 0 }, { 0, 1 } },
		{ { +0.5, -0.5, 0.0 }, { 0, 1, 0 }, { 1, 1 } }
	};

	glm::mat4 viewMatrix = camera->Get3dViewMatrix();
	glm::vec3 left = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
	glm::vec3 up = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->arrowTexture);
	for (WorldObject *obj : this->visibleObjects) {
		if (obj->group != OBJECT_GROUP_UNIT)
			continue;

		Unit *unit = static_cast<Unit*>(obj);
		if (!unit->selected)
			continue;

		glm::mat4 mmatrix;
		mmatrix = glm::translate(mmatrix, GetObjectTranslationRelativeToCamera(camera, obj) + glm::vec3(0, 256, 0));
		mmatrix = glm::scale(mmatrix, glm::vec3(32));

		vertices[0].position = -left + up;
		vertices[1].position = -left - up;
		vertices[2].position =  left + up;
		vertices[5].position =  left - up;
		vertices[3].position = vertices[2].position;
		vertices[4].position = vertices[1].position;

		this->objectVertexBuffer->Clear();
		this->objectVertexBuffer->AddRange(vertices, countof(vertices));
		this->objectVertexBuffer->Update();

		glUniformMatrix4fv(this->objectShaderUniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(mmatrix));
		RenderVertices();
	}
}