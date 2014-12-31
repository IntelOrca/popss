#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class LightSource {
public:
	glm::vec3 position;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float radius;
};

} }