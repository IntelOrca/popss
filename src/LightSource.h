#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

enum {
	LIGHT_SOURCE_TYPE_SUN,
	LIGHT_SOURCE_TYPE_MOON,
	LIGHT_SOURCE_TYPE_FIRE
};

class LightSource {
public:
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float radius;
};

} }