#pragma once

#include "../PopSS.h"

namespace IntelOrca { namespace PopSS {

enum {
	OBJECT_GROUP_UNIT,
	OBJECT_GROUP_BUILDING,
	OBJECT_GROUP_SCENERY,
	OBJECT_GROUP_EFFECT
};

enum {
	OWNERSHIP_NEUTRAL = 255
};

typedef unsigned char objecttype8;
typedef unsigned char objectgroup8;
typedef unsigned char ownership8;
typedef unsigned char angle8;

class WorldObject {
public:
	objecttype8 type;
	objectgroup8 group;

	union {
		struct { glm::ivec3 position; };
		struct { int x, y, z; };
	};

	ownership8 ownership;
	angle8 rotation;

	WorldObject();
	virtual ~WorldObject();

	virtual void Update();
};

} }