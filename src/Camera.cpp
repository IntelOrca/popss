#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Util/MathExtensions.hpp"
#include "World.h"

using namespace IntelOrca::PopSS;

const float Camera::MoveSpeed = 1.0f / 1.0f;
const float Camera::RotationSpeed = 360.0f / (60.0f * 3.0f);

Camera::Camera()
{
	this->fov = 60.0f;
	this->target = glm::vec3(128, 512 + 128, 128);
	this->zoom = 1024.0 + 256;
	this->rotation = 0.0f;
}

Camera::~Camera() { }

glm::mat4 Camera::Get3dProjectionMatrix() const
{
	return glm::perspective(
		this->fov * (float)(M_PI / 180),
		1920.0f / 1080.0f,
		1.0f,
		256.0f * 256.0f
	);
}

glm::mat4 Camera::Get3dViewMatrix() const
{
	glm::mat4 result = glm::lookAt(
		glm::vec3(0.0f, this->zoom, this->zoom),
		glm::vec3(0.0f, this->target.y, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	result = glm::scale(result, glm::vec3(-1.0f, 1.0f, 1.0f));
	result = glm::rotate(result, toradians(this->rotation), glm::vec3(0.0f, 1.0f, 0.0f));

	return result;
}

void Camera::RotateLeft()
{
	this->rotation = wraprange(0.0f, this->rotation + RotationSpeed, 360.0f);
}

void Camera::RotateRight()
{
	this->rotation = wraprange(0.0f, this->rotation - RotationSpeed, 360.0f);
}

void Camera::MoveForwards()
{
	this->target.x += sin(toradians(this->rotation)) * MoveSpeed;
	this->target.z -= cos(toradians(this->rotation)) * MoveSpeed;

	this->target.x = this->world->MapWrap(this->target.x);
	this->target.z = this->world->MapWrap(this->target.z);
}

void Camera::MoveBackwards()
{
	this->target.x -= sin(toradians(this->rotation)) * MoveSpeed;
	this->target.z += cos(toradians(this->rotation)) * MoveSpeed;

	this->target.x = this->world->MapWrap(this->target.x);
	this->target.z = this->world->MapWrap(this->target.z);
}