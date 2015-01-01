#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

enum {
	OWNERSHIP_NEUTRAL = 255
};

typedef unsigned char ownership8;
typedef unsigned char angle8;

class WorldObject {
public:
	union {
		struct { glm::ivec3 position; };
		struct { int x, y, z; };
	};

	ownership8 ownership;
	angle8 rotation;

	WorldObject();
	virtual ~WorldObject();

	virtual void Update();
	virtual void Draw() const;
};

} }