#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class World;
class Camera {
public:
	static const float MoveSpeed;
	static const float RotationSpeed;

	World *world;

	float fov;
	float rotation;
	float zoom;
	glm::vec3 target;

	Camera();
	~Camera();

	glm::mat4 Get3dProjectionMatrix() const;
	glm::mat4 Get3dViewMatrix() const;

	void RotateLeft();
	void RotateRight();
	void MoveForwards();
	void MoveBackwards();
};

} }