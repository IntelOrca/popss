#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class World;
class Camera {
public:
	static const float MoveSpeed;
	static const float RotationSpeed;

	bool viewHasChanged;

	World *world;

	float fov;
	float rotation;
	float zoom;
	float targetZoom;
	float zoomSpeed;
	glm::vec3 target;

	glm::vec3 eye;

	Camera();
	~Camera();

	glm::mat4 Get3dProjectionMatrix() const;
	glm::mat4 Get3dViewMatrix() const;

	bool GetWorldPositionFromViewport(int x, int y, glm::ivec3 *outPosition) const;

	void Update();
	void UpdateEye();

	void RotateLeft();
	void RotateRight();
	void MoveForwards();
	void MoveBackwards();

	void SetRotation(float value);
	void SetZoom(float value);

	void UpdateZoom();
	void ZoomIn();
	void ZoomOut();

private:
	glm::vec3 GetViewportRayDirection(int x, int y) const;
	glm::vec3 SphereDistort(const glm::vec3 &position, float ratio) const;
};

} }