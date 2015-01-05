#pragma once

#include "PopSS.h"
#include "SimpleVertexBuffer.hpp"

namespace IntelOrca { namespace PopSS {

struct SkyVertex {
	glm::vec4 position;
};

class Camera;
class OrcaShader;
class World;
class SkyRenderer {
public:
	World *world;

	SkyRenderer();
	~SkyRenderer();

	void Initialise();
	void Render(const Camera *camera);

private:
	OrcaShader *skyShader;
	SimpleVertexBuffer<SkyVertex> *skyVertexBuffer;
};

} }