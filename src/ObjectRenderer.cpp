#include "Camera.h"
#include "LandscapeRenderer.h"
#include "LightSource.h"
#include "Mesh.h"
#include "ObjectRenderer.h"
#include "OrcaShader.h"
#include "World.h"
#include "Objects/WorldObject.h"
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
	this->objectShader = NULL;
	this->objectVertexBuffer = NULL;

	this->unitMesh = NULL;
	this->vokMesh = NULL;
}

ObjectRenderer::~ObjectRenderer()
{
	if (this->objectShader != NULL) delete this->objectShader;
	if (this->objectVertexBuffer != NULL) delete this->objectVertexBuffer;

	if (this->unitMesh != NULL) delete this->unitMesh;
	if (this->vokMesh != NULL) delete this->vokMesh;
}

void ObjectRenderer::Initialise()
{
	this->objectShader = OrcaShader::FromPath("object.vert", "object.frag");
	this->objectShaderUniforms.projectionMatrix = this->objectShader->GetUniformLocation("ProjectionMatrix");
	this->objectShaderUniforms.viewMatrix = this->objectShader->GetUniformLocation("ViewMatrix");
	this->objectShaderUniforms.modelMatrix = this->objectShader->GetUniformLocation("ModelMatrix");
	this->objectShaderUniforms.sphereRatio = this->objectShader->GetUniformLocation("InputSphereRatio");
	this->objectShaderUniforms.cameraTarget = this->objectShader->GetUniformLocation("InputCameraTarget");
	this->objectShaderUniforms.texture = this->objectShader->GetUniformLocation("InputTexture");
	this->objectVertexBuffer = new SimpleVertexBuffer<ObjectVertex>(this->objectShader, ObjectShaderVertexInfo);

	this->unitMesh = Mesh::FromObjectFile("data/objects/unit.object");
	this->treeMesh[0] = Mesh::FromObjectFile("data/objects/tree1.object");
	this->treeMesh[1] = Mesh::FromObjectFile("data/objects/tree2.object");
	this->treeMesh[2] = Mesh::FromObjectFile("data/objects/tree3.object");

	this->vokMesh = Mesh::FromObjectFile("data/objects/vok.object");

	glGenTextures(1, &this->vokTexture);
	LoadTexture(this->vokTexture, "data/objects/vok.png");
}

void ObjectRenderer::Render(const Camera *camera)
{
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
	SetLightSources(camera, this->objectShader);

	UpdateVisibleObjects(camera);
	RenderObjectGroups(camera);

	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->vokTexture);

	switch (obj->group) {
	case OBJECT_GROUP_UNIT:
		PrepareMesh(this->unitMesh);
		break;
	case OBJECT_GROUP_BUILDING:
		if (obj->type == BUILDING_VAULT_OF_KNOWLEDGE)
			PrepareMesh(this->vokMesh);
		break;
	case OBJECT_GROUP_SCENERY:
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
			if (obj->type == BUILDING_VAULT_OF_KNOWLEDGE) {
				mmatrix = glm::scale(mmatrix, glm::vec3(3 * World::TileSize));
			}
			break;
		case OBJECT_GROUP_SCENERY:
			if (obj->type >= SCENERY_TREE0 && obj->type <= SCENERY_TREE2) {
				mmatrix = glm::scale(mmatrix, glm::vec3(3 * World::TileSize));
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
	glm::ivec3 cameraPosition = glm::ivec3(camera->target);

	int distanceXa = abs(obj->x - cameraPosition.x);
	int distanceXb = world->sizeByNonTiles - distanceXa;
	int distanceZa = abs(obj->z - cameraPosition.z);
	int distanceZb = world->sizeByNonTiles - distanceZa;
	int distanceX = min(distanceXa, distanceXb);
	int distanceZ = min(distanceZa, distanceZb);
	int distance = sqrt(distanceX * distanceX + distanceZ * distanceZ);

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
		
		for (int j = 0; j < 3; j++) {
			vertices[j] = mesh->vertices[face->vertex[j].position];
			texcoords[j] = face->vertex[j].texture == -1 ? glm::vec2(0) : mesh->textureCoordinates[face->vertex[j].texture];
		}

		glm::vec3 normal = glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
		for (int j = 0; j < 3; j++) {
			ObjectVertex vertex;
			vertex.position = vertices[j];
			vertex.normal = normal;
			vertex.texcoords = texcoords[j];
			this->objectVertexBuffer->Add(vertex);
		}
	}
	this->objectVertexBuffer->Update();
}

void ObjectRenderer::RenderVertices()
{
	this->objectVertexBuffer->Draw(GL_TRIANGLES);
}

void ObjectRenderer::SetLightSources(const Camera *camera, OrcaShader *shader)
{
	LightSource alight, *light = &alight;

	light->position = glm::vec3(0.0f, 2048.0f, 0.0f) + glm::vec3(camera->target.x, 0, camera->target.z);
	light->ambient = glm::vec4(0.05f);
	light->diffuse = glm::vec4(0.4f);
	light->specular = glm::vec4(0.0f);

	glUniform1i(glGetUniformLocation(shader->program, "InputLightSourcesCount"), 2);
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Specular"), 1, glm::value_ptr(light->specular));

	light->position = glm::vec3(-1.0f, 0.1f, 1.0f) * World::SkyDomeRadius + glm::vec3(camera->target.x, 0, camera->target.z);
	light->ambient = glm::vec4(0.0f);
	light->diffuse = glm::vec4(0.5f, 0.5f, 0.0f, 0.0f);
	light->specular = glm::vec4(0.25f, 0.25f, 0.0f, 0.0f);

	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Specular"), 1, glm::value_ptr(light->specular));
}