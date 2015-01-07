#pragma once

#include "Building.h"

namespace IntelOrca { namespace PopSS {

class VaultOfKnowledge : public Building {
public:
	VaultOfKnowledge();
	override ~VaultOfKnowledge();

	override void Update();
};

} }