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
	glm::vec3 target;

	glm::vec3 eye;

	Camera();
	~Camera();

	glm::mat4 Get3dProjectionMatrix() const;
	glm::mat4 Get3dViewMatrix() const;

	glm::ivec3 GetWorldPositionFromViewport(int x, int y) const;

	void RotateLeft();
	void RotateRight();
	void MoveForwards();
	void MoveBackwards();

	void SetRotation(float value);
	void SetZoom(float value);

private:
	void UpdateEye();
};

} }