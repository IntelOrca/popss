#include "Camera.h"
#include "LandscapeRenderer.h"
#include "LightSource.h"
#include "Mesh.h"
#include "ObjectRenderer.h"
#include "OrcaShader.h"
#include "World.h"
#include "WorldObject.h"
#include "Objects/Buildings/Building.h"

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
	
}

ObjectRenderer::~ObjectRenderer()
{
	delete this->vokMesh;
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

	glGenVertexArrays(1, &this->objectVAO);
	glGenBuffers(1, &this->objectVBO);

	glBindVertexArray(this->objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->objectVBO);
	this->objectShader->SetVertexAttribPointer(sizeof(ObjectVertex), ObjectShaderVertexInfo);

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

	glBindVertexArray(this->objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->objectVBO);

	// Render vault of knowledges
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->vokTexture);
	PrepareMesh(this->vokMesh);
	for (WorldObject *worldObject : this->world->objects) {
		if (worldObject->type == BUILDING_VAULT_OF_KNOWLEDGE && worldObject->group == OBJECT_GROUP_BUILDING) {	
			glm::mat4 mmatrix;
			mmatrix = glm::translate(mmatrix, glm::vec3(worldObject->position));
			mmatrix = glm::rotate(mmatrix, (worldObject->rotation / 128.0f) * (float)M_PI, glm::vec3(0, 1, 0));
			mmatrix = glm::scale(mmatrix, glm::vec3(3 * World::TileSize));

			glUniformMatrix4fv(this->objectShaderUniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(mmatrix));
			RenderVertices();
		}
	}
}

void ObjectRenderer::PrepareMesh(const Mesh *mesh)
{
	this->objectVertices.clear();
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
			this->objectVertices.push_back(vertex);
		}
	}
}

void ObjectRenderer::RenderVertices()
{
	glBufferData(GL_ARRAY_BUFFER, this->objectVertices.size() * sizeof(ObjectVertex), this->objectVertices.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, this->objectVertices.size());
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