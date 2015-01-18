#pragma once

#include "../../Pathfinding.h"
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

	bool requiresPathFind;
	Path pathToDestination;
	int pathToDestinationCurrentIndex;

	Unit();
	override ~Unit();

	override void Update();

	void RunTo(int targetX, int targetZ);
	void Stop();
	void GiveMoveOrder(int x, int z);
};

} }