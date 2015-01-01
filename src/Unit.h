#pragma once

#include "WorldObject.h"

namespace IntelOrca { namespace PopSS {

class Unit : public WorldObject {
public:
	Unit();
	override ~Unit();

	override void Update();
	override void Draw() const;
};

} }