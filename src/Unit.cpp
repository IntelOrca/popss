#include "Camera.h"
#include "GameView.h"
#include "LightSource.h"
#include "OrcaShader.h"
#include "Unit.h"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace IntelOrca::PopSS;

struct UnitVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

const VertexAttribPointerInfo UnitShaderVertexInfo[] = {
	{ "VertexPosition",		GL_FLOAT,	3,	offsetof(UnitVertex, position)	},
	{ "VertexNormal",		GL_FLOAT,	3,	offsetof(UnitVertex, normal)	},
	{ NULL }
};

OrcaShader *unitShader = NULL;
GLuint unitVAO;
GLuint unitVBO;
std::vector<UnitVertex> unitVertices;

GLint projectionMatrix;
GLint viewMatrix;
GLint modelMatrix;
GLint sphereRatio;

const UnitVertex UnitVertexData[] = {
	{ { -0.5, 0, -0.5 }, { 0, -0.5, 0 } },
	{ { -0.5, 0,  0.5 }, { 0, -0.5, 0 } },
	{ {  0.5, 0,  0.5 }, { 0, -0.5, 0 } },

	{ { -0.5, 3, -0.5 }, { 0,  0.5, 0 } },
	{ { -0.5, 3,  0.5 }, { 0,  0.5, 0 } },
	{ {  0.5, 3,  0.5 }, { 0,  0.5, 0 } },

	{ { -0.5, 3, -0.5 }, { -1, 0, 0 } },
	{ { -0.5, 0, -0.5 }, { -1, 0, 0 } },
	{ { -0.5, 0,  0.5 }, { -1, 0, 0 } },
	{ { -0.5, 3, -0.5 }, { -1, 0, 0 } },
	{ { -0.5, 0,  0.5 }, { -1, 0, 0 } },
	{ { -0.5, 3,  0.5 }, { -1, 0, 0 } },

	{ {  0.5, 3, 0.5 }, { 0, 0, 1 } },
	{ {  0.5, 0, 0.5 }, { 0, 0, 1 } },
	{ { -0.5, 0, 0.5 }, { 0, 0, 1 } },
	{ {  0.5, 3, 0.5 }, { 0, 0, 1 } },
	{ { -0.5, 0, 0.5 }, { 0, 0, 1 } },
	{ { -0.5, 3, 0.5 }, { 0, 0, 1 } },

	{ { -0.5, 3, -0.5 }, { 1, 0, -1 } },
	{ { -0.5, 0, -0.5 }, { 1, 0, -1 } },
	{ {  0.5, 0,  0.5 }, { 1, 0, -1 } },
	{ { -0.5, 3, -0.5 }, { 1, 0, -1 } },
	{ {  0.5, 0,  0.5 }, { 1, 0, -1 } },
	{ {  0.5, 3,  0.5 }, { 1, 0, -1 } },
};

static void SetLightSources(OrcaShader *shader)
{
	LightSource alight, *light = &alight;

	light->position = glm::vec3(0.0f, 2048.0f, 0.0f);
	light->ambient = glm::vec4(0.05f);
	light->diffuse = glm::vec4(0.4f);
	light->specular = glm::vec4(0.0f);

	glUniform1i(glGetUniformLocation(shader->program, "InputLightSourcesCount"), 2);
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Specular"), 1, glm::value_ptr(light->specular));

	light->position = glm::vec3(-1.0f, 0.1f, 1.0f) * World::SkyDomeRadius;
	light->ambient = glm::vec4(0.0f);
	light->diffuse = glm::vec4(0.5f, 0.5f, 0.0f, 0.0f);
	light->specular = glm::vec4(0.25f, 0.25f, 0.0f, 0.0f);

	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Specular"), 1, glm::value_ptr(light->specular));
}

Unit::Unit() : WorldObject()
{

}

Unit::~Unit() { }

void Unit::Update()
{
	const WorldTile *tile = gWorld->GetTile(this->x, this->z);
	
	this->y = tile->height;
}

void Unit::Draw() const
{
	const Camera *camera = &gGameView->camera;

	// Check if the object is close enough to the camera
	float relativeX = (float)(this->x - camera->target.x);
	float relativeZ = (float)(this->z - camera->target.z);
	if (sqrt((relativeX * relativeX) + (relativeZ + relativeZ)) > 76)
		return;

	if (unitShader == NULL) {
		unitShader = OrcaShader::FromPath("object.vert", "object.frag");
		projectionMatrix = unitShader->GetUniformLocation("ProjectionMatrix");
		viewMatrix = unitShader->GetUniformLocation("ViewMatrix");
		modelMatrix = unitShader->GetUniformLocation("ModelMatrix");
		sphereRatio = unitShader->GetUniformLocation("InputSphereRatio");

		glGenVertexArrays(1, &unitVAO);
		glGenBuffers(1, &unitVBO);

		glBindVertexArray(unitVAO);
		glBindBuffer(GL_ARRAY_BUFFER, unitVBO);
		unitShader->SetVertexAttribPointer(sizeof(UnitVertex), UnitShaderVertexInfo);
	} else {
		glBindVertexArray(unitVAO);
		glBindBuffer(GL_ARRAY_BUFFER, unitVBO);
	}

	glm::mat4 pmatrix = camera->Get3dProjectionMatrix();
	glm::mat4 vmatrix = camera->Get3dViewMatrix();

	// Produce a model matrix
	glm::mat4 mmatrix;
	mmatrix = glm::translate(mmatrix, glm::vec3(relativeX * World::TileSize, this->y, relativeZ * World::TileSize));
	mmatrix = glm::scale(mmatrix, glm::vec3(World::TileSize / 2.0f));

	unitShader->Use();

	glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, glm::value_ptr(pmatrix));
	glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, glm::value_ptr(vmatrix));
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, glm::value_ptr(mmatrix));
	glUniform1f(sphereRatio, LandscapeRenderer::SphereRatio);
	SetLightSources(unitShader);

	unitVertices.clear();
	unitVertices.insert(unitVertices.end(), UnitVertexData, UnitVertexData + countof(UnitVertexData));
	glBufferData(GL_ARRAY_BUFFER, unitVertices.size() * sizeof(UnitVertex), unitVertices.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, unitVertices.size());
}