#include "Camera.h"
#include "Util/MathExtensions.hpp"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace IntelOrca::PopSS;

const float Camera::MoveSpeed = 1.0f * World::TileSize;
const float Camera::RotationSpeed = 360.0f / (60.0f * 3.0f);

Camera::Camera()
{
	this->fov = 60.0f;
	this->target = glm::vec3(0, 512 + 128, 0);
	// this->target = glm::vec3(128 * World::TileSize, 512 + 128, 128 * World::TileSize);
	this->zoom = 1024.0 + 256;
	SetRotation(0.0f);
	this->viewHasChanged = true;
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
	return glm::lookAt(
		this->eye,
		this->target,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
}

glm::ivec3 Camera::GetWorldPositionFromViewport(int x, int y) const
{
	glm::vec3 rayNDS = glm::vec3(
		(gCursor.x * 2.0f) / 1920 - 1.0f,
		1.0f - (gCursor.y * 2.0f) / 1080,
		1.0f
	);

	glm::vec4 rayClip = glm::vec4(rayNDS.x, rayNDS.y, -1.0, 1.0);

	glm::vec4 rayEye = glm::inverse(this->Get3dProjectionMatrix()) * rayClip;
	rayEye.z = -1.0f;
	rayEye.w = 0.0f;

	// glm::mat4 viewMatrix = glm::lookAt(
	// 	glm::vec3(0.0f, this->zoom, this->zoom),
	// 	glm::vec3(0.0f, this->target.y, 0.0f),
	// 	glm::vec3(0.0f, 1.0f, 0.0f)
	// );
	// viewMatrix = glm::scale(viewMatrix, glm::vec3(-1.0f, 1.0f, 1.0f));

	glm::vec3 rayWOR = glm::vec3(glm::inverse(this->Get3dViewMatrix()) * rayEye);
	rayWOR = glm::normalize(rayWOR);

	glm::vec3 planeNormal = glm::vec3(0, 1, 0);
	float planeOffset = 0;

	float t = -(glm::dot(this->eye, planeNormal) + planeOffset) / glm::dot(rayWOR, planeNormal);
	if (t < 0)
		return glm::ivec3(INT32_MIN);
	
	return this->eye + t * rayWOR;
}

void Camera::RotateLeft()
{
	SetRotation(wraprange(0.0f, this->rotation + RotationSpeed, 360.0f));
}

void Camera::RotateRight()
{
	SetRotation(wraprange(0.0f, this->rotation - RotationSpeed, 360.0f));
}

void Camera::MoveForwards()
{
	this->target.x -= sin(toradians(this->rotation)) * MoveSpeed;
	this->target.z -= cos(toradians(this->rotation)) * MoveSpeed;

	this->target.x = this->world->Wrap(this->target.x);
	this->target.z = this->world->Wrap(this->target.z);

	UpdateEye();
}

void Camera::MoveBackwards()
{
	this->target.x += sin(toradians(this->rotation)) * MoveSpeed;
	this->target.z += cos(toradians(this->rotation)) * MoveSpeed;

	this->target.x = this->world->Wrap(this->target.x);
	this->target.z = this->world->Wrap(this->target.z);

	UpdateEye();
}

void Camera::UpdateEye()
{
	float zoom = this->zoom;
	float rotationRadians = toradians(this->rotation);

	this->eye = glm::vec3(
		sin(rotationRadians) * zoom + this->target.x,
		zoom,
		cos(rotationRadians) * zoom + this->target.z
	);;

	this->viewHasChanged = true;
}

void Camera::SetRotation(float value)
{
	this->rotation = value;
	UpdateEye();
}

void Camera::SetZoom(float value)
{
	this->zoom = value;
	UpdateEye();
}