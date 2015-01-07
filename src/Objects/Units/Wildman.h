#pragma once

#include "Unit.h"

namespace IntelOrca { namespace PopSS {

class Wildman : public Unit {
public:
	Wildman();
	override ~Wildman();

	override void Update();
};

} }