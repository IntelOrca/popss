#include "Camera.h"
#include "GameView.h"
#include "LightSource.h"
#include "OrcaShader.h"
#include "Unit.h"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace IntelOrca::PopSS;

static void SetLightSources(const Camera *camera, OrcaShader *shader)
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

Unit::Unit() : WorldObject()
{
	this->group = OBJECT_GROUP_UNIT;
	this->movingToDestination = false;
}

Unit::~Unit() { }

void Unit::Update()
{
	const WorldTile *tile = gWorld->GetTile(this->x / World::TileSize, this->z / World::TileSize);
	const float speed = 2.0f;

	if (this->movingToDestination && this->position != this->destination) {
		glm::vec3 pos = this->position;
		glm::vec3 dst = this->destination;

		pos.y = 0;
		dst.y = 0;

		glm::vec3 direction = dst - pos;

		if (glm::length(direction) <= speed) {
			this->position = this->destination;
			this->subposition = this->position;
			this->velocity = glm::vec3(0);
			this->movingToDestination = false;
		} else {
			this->velocity = glm::normalize(direction) * speed;
			this->subposition += this->velocity;
			this->position = this->subposition;
		}
	} else {
		this->subposition = this->position;
	}

	this->y = gWorld->GetHeight(this->x, this->z);
}

void Unit::Draw() const { }

void Unit::GiveMoveOrder(int x, int z)
{
	this->destination = glm::vec3(x, 0, z);
	this->movingToDestination = true;
}