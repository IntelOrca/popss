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

bool GetIntersection(const glm::vec3 &l0, const glm::vec3 &ldirection, const glm::vec3 &planePoint, const glm::vec3 &planeNormal, glm::vec3 *intersection)
{
	float t = glm::dot(planePoint - l0, planeNormal) / glm::dot(ldirection, planeNormal);
	if (t < 0)
		return false;
	
	*intersection = l0 + (ldirection * t);
	return true;
}

#define EPSILON 0.000001
bool TriangleIntersect(const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, float *out)
{
	glm::vec3 e1, e2;  //Edge1, Edge2
	glm::vec3 P, Q, T;
	float det, inv_det, u, v;
	float t;

	//Find vectors for two edges sharing V1
	e1 = v1 - v0;
	e2 = v2 - v0;
	//Begin calculating determinant - also used to calculate u parameter
	P = glm::cross(dir, e2);
	//if determinant is near zero, ray lies in plane of triangle
	det = glm::dot(e1, P);
	//NOT CULLING
	if (det > -EPSILON && det < EPSILON) return false;
	inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	T = orig - v0;

	//Calculate u parameter and test bound
	u = glm::dot(T, P) * inv_det;
	//The intersection lies outside of the triangle
	if (u < 0.f || u > 1.f) return false;

	//Prepare to test v parameter
	Q = glm::cross(T, e1);

	//Calculate V parameter and test bound
	v = glm::dot(dir, Q) * inv_det;
	//The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f) return false;

	t = glm::dot(e2, Q) * inv_det;

	if (t > EPSILON) { //ray intersection
		*out = t;
		return true;
	}

	// No hit, no win
	return false;
}

int QuadIntersect(const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 *v, float *out)
{
	if (TriangleIntersect(orig, dir, v[0], v[1], v[2], out) || TriangleIntersect(orig, dir, v[2], v[3], v[0], out)) {
		float dist = ((v[0].x + v[1].x + v[2].x + v[3].x) / 4.0f - orig.x) + ((v[0].z + v[1].z + v[2].z + v[3].z) / 4.0f - orig.z);
		if(dist < 0)
			dist *= -1;

		return dist;
	}

	return INT32_MIN;
}

glm::ivec3 Camera::GetWorldPositionFromViewport(int x, int y) const
{
	// glm::vec3 wc = glm::unProject(
	// 	glm::vec3(x, 1080 - y - 1.0f, 1.0f),
	// 	this->Get3dViewMatrix(),
	// 	this->Get3dProjectionMatrix(),
	// 	glm::vec4(0, 0, 1920, 1080)
	// );
	// printf("x: %d (%d), y: %d (%d), z: %d (%d)\n", (int)wc.x, (int)(wc.x / World::TileSize), (int)wc.y, (int)(wc.y / World::TileSize), (int)wc.z, (int)(wc.z / World::TileSize));

	glm::vec3 rayNDS = glm::vec3(
		(gCursor.x * 2.0f) / 1920 - 1.0f,
		1.0f - (gCursor.y * 2.0f) / 1080,
		1.0f
	);

	glm::vec4 rayClip = glm::vec4(rayNDS.x, rayNDS.y, -1.0, 1.0);

	glm::vec4 rayEye = glm::inverse(this->Get3dProjectionMatrix()) * rayClip;
	rayEye.z = -1.0f;
	rayEye.w = 0.0f;

	glm::vec3 rayWOR = glm::vec3(glm::inverse(this->Get3dViewMatrix()) * rayEye);
	rayWOR = glm::normalize(rayWOR);

	bool f = false;
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
			
			float t;
			dist = QuadIntersect(this->eye, rayWOR, v, &t);
			if ((!f || dist < sdist) && dist != INT32_MIN) {
				f = true;
				sdist = dist;
				sintersection = this->eye + rayWOR * t;
				// if (gCursorPress.button == 0)
				//	sintersection = glm::vec3(lx + World::TileSize, 0, lz + World::TileSize);
			}


			// int landHeight = this->world->GetTile(landX, landZ)->height;
			// glm::vec3 landPoint = glm::vec3(lx, landHeight, lz);
			// glm::vec3 landNormal = this->world->CalculateNormal(x, z);
			// 
			// glm::vec3 intersection;
			// if (!GetIntersection(this->eye, rayWOR, landPoint, landNormal, &intersection))
			// 	continue;
			// 
			// float distance = glm::length(landPoint - intersection);
			// 
			// float dx = this->target.x - intersection.x;
			// float dz = this->target.z - intersection.z;
			// dist = sqrt(dx * dx + dz * dz);
			// if (distance < World::TileSize && (!f || dist < sdist)) {
			// 	f = true;
			// 	sdist = dist;
			// 	sintersection = intersection;
			// }
		}
	}

	if (f) {
		return sintersection;
	}


	glm::vec3 planePoint = glm::vec3(0, 0, 0);
	glm::vec3 planeNormal = glm::vec3(0, 1, 0);
	glm::vec3 intersection;
	if (!GetIntersection(this->eye, rayWOR, planePoint, planeNormal, &intersection))
		return glm::ivec3(INT32_MIN);
	
	return intersection;
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