#pragma once

#include "../WorldObject.h"

namespace IntelOrca { namespace PopSS {

enum {
	UNIT_WILDMAN,
	UNIT_BRAVE,
	UNIT_WARRIOR,
	UNIT_PREIST,
	UNIT_FIRE_WARRIOR,
	UNIT_SPY,
	UNIT_SHAMAN
};

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

	void GiveMoveOrder(int x, int z);
};

} }