#pragma once

#include "../../WorldObject.h"

namespace IntelOrca { namespace PopSS {

enum {
	BUILDING_VAULT_OF_KNOWLEDGE
};

class Building : public WorldObject {
public:
	Building();
	override ~Building();

	override void Update();
	override void Draw() const;
};

} }