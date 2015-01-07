#pragma once

#include "Unit.h"

namespace IntelOrca { namespace PopSS {

class Shaman : public Unit {
public:
	Shaman();
	override ~Shaman();

	override void Update();
};

} }