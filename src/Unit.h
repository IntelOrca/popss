#pragma once

#include "WorldObject.h"

namespace IntelOrca { namespace PopSS {

class Unit : public WorldObject {
public:
	bool selected;
	bool movingToDestination;
	glm::ivec3 destination;
	glm::vec3 subposition;
	glm::vec3 velocity;

	Unit();
	override ~Unit();

	override void Update();
	override void Draw() const;

	void GiveMoveOrder(int x, int z);
};

} }