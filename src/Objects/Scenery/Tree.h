#pragma once

#include "../WorldObject.h"

namespace IntelOrca { namespace PopSS {

enum {
	SCENERY_TREE0,
	SCENERY_TREE1,
	SCENERY_TREE2
};

class Tree : public WorldObject {
public:
	int wood;
	int maxWood;
	int ticksRemainingForNextWood;

	Tree();
	override ~Tree();

	override void Update();
};

} }