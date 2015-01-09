#include "Camera.h"
#include "LightManager.h"
#include "OrcaShader.h"

using namespace IntelOrca::PopSS;

void LightManager::RegisterLightSource(const LightSource *lightSource)
{
	this->lightSources.push_back(lightSource);
}

void LightManager::UnregisterLightSource(const LightSource *lightSource)
{
	this->lightSources.remove(lightSource);
}

void LightManager::SetLightSources(const Camera *camera, OrcaShader *shader)
{
	this->SetLightSource(camera, shader, 0, &this->natural);
	this->SetLightSource(camera, shader, 1, &this->sun);

	glUniform1i(glGetUniformLocation(shader->program, "InputLightSourcesCount"), 2);
}

void LightManager::SetLightSource(const Camera *camera, OrcaShader *shader, int index, const LightSource *light)
{
	char positionBuffer[] = "InputLightSources[0].Position";
	char ambientBuffer[] = "InputLightSources[0].Ambient";
	char diffuseBuffer[] = "InputLightSources[0].Diffuse";
	char specularBuffer[] = "InputLightSources[0].Specular";

	const int indexCharPosition = 18;

	positionBuffer[indexCharPosition] = '0' + index;
	ambientBuffer[indexCharPosition] = '0' + index;
	diffuseBuffer[indexCharPosition] = '0' + index;
	specularBuffer[indexCharPosition] = '0' + index;

	glm::vec3 pos = light->position + glm::vec3(camera->target.x, 0, camera->target.z);

	glUniform3fv(glGetUniformLocation(shader->program, positionBuffer), 1, glm::value_ptr(pos));
	glUniform3fv(glGetUniformLocation(shader->program, ambientBuffer), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(shader->program, diffuseBuffer), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(shader->program, specularBuffer), 1, glm::value_ptr(light->specular));
}