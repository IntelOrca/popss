#pragma once

#include "PopSS.h"
#include "SimpleVertexBuffer.hpp"

namespace IntelOrca { namespace PopSS {

struct HandVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

class Camera;
class OrcaShader;
class LoadingScreen {
public:
	LoadingScreen();
	~LoadingScreen();

	void Draw();
	void Update();
	
private:
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;

	OrcaShader *handShader;
	SimpleVertexBuffer<HandVertex> *handVertexBuffer;

	float angle;

	void Initialise();
};

} }