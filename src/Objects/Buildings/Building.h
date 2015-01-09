#pragma once

#include "../WorldObject.h"

namespace IntelOrca { namespace PopSS {

enum {
	BUILDING_HUT,
	BUILDING_GUARD_TOWER,
	BUILDING_WARRIOR,
	BUILDING_PREACHER,
	BUILDING_FIRE_WARRIOR,
	BUILDING_SPY,
	BUILDING_STABLE,
	BUILDING_BOAT,
	BUILDING_BALLOON,
	BUILDING_PRISON,
	BUILDING_VAULT_OF_KNOWLEDGE
};

class Building : public WorldObject {
public:
	Building();
	override ~Building();

	override void Update();
};

} }