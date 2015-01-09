#pragma once

#include "Building.h"

namespace IntelOrca { namespace PopSS {

class GuardTower : public Building {
public:
	GuardTower();
	override ~GuardTower();

	override void Update();
};

} }