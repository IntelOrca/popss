#include "Camera.h"
#include "LandscapeRenderer.h"
#include "Util/MathExtensions.hpp"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace IntelOrca::PopSS;

const float Camera::MoveSpeed = 1.0f * World::TileSize;
const float Camera::RotationSpeed = 360.0f / (60.0f * 3.0f);

Camera::Camera()
{
	this->fov = 60.0f;
	// this->target = glm::vec3(0, 512 + 128, 0);
	this->target = glm::vec3(128 * World::TileSize, 512 + 128, 128 * World::TileSize);
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

glm::vec3 Camera::GetViewportRayDirection(int x, int y) const
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

	return glm::normalize(glm::vec3(glm::inverse(this->Get3dViewMatrix()) * rayEye));
}

glm::vec3 Camera::SphereDistort(const glm::vec3 &position, float ratio) const
{
	glm::vec3 relative = position - this->target;
	return glm::vec3(
		position.x,
		position.y - (ratio * (relative.x * relative.x + relative.z * relative.z)),
		position.z
	);
}

bool Camera::GetWorldPositionFromViewport(int x, int y, glm::ivec3 *outPosition) const
{
	float t, bestT;
	glm::vec3 eye = this->eye;
	glm::vec3 eyeDirection = this->GetViewportRayDirection(x, y);

	bool foundLandIntersection = false;
	float dist, sdist;
	glm::vec3 v[4];
	glm::vec3 sintersection;

	int landOriginX = (int)(this->target.x / World::TileSize);
	int landOriginZ = (int)(this->target.z / World::TileSize);
	for (int z = -32; z <= 32; z++) {
		for (int x = -32; x <= 32; x++) {
			int landX = landOriginX + x;
			int landZ = landOriginZ + z;
			int lx = landX * World::TileSize;
			int lz = landZ * World::TileSize;
			
			v[0] = glm::vec3(lx, this->world->GetTile(landX, landZ)->height, lz);
			v[1] = glm::vec3(lx, this->world->GetTile(landX, landZ + 1)->height, lz + World::TileSize);
			v[2] = glm::vec3(lx + World::TileSize, this->world->GetTile(landX + 1, landZ + 1)->height, lz + World::TileSize);
			v[3] = glm::vec3(lx + World::TileSize, this->world->GetTile(landX + 1, landZ)->height, lz);

			for (int i = 0; i < 4; i++)
				v[i] = this->SphereDistort(v[i], LandscapeRenderer::SphereRatio);
			
			if (TriangleIntersect(eye, eyeDirection, v[0], v[1], v[2], &t) || TriangleIntersect(eye, eyeDirection, v[2], v[3], v[0], &t)) {
				float dist = abs(((v[0].x + v[1].x + v[2].x + v[3].x) / 4.0f - eye.x) + ((v[0].z + v[1].z + v[2].z + v[3].z) / 4.0f - eye.z));
				if (!foundLandIntersection || dist < sdist) {
					foundLandIntersection = true;
					sdist = dist;
					bestT = t;
				}
			}
		}
	}

	if (foundLandIntersection) {
		*outPosition = eye + eyeDirection * t;
		return true;
	}

	if (!PlaneIntersect(eye, eyeDirection, glm::vec3(0), glm::vec3(0, 1, 0), &t))
		return false;
	
	*outPosition = eye + eyeDirection * t;
	return true;
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