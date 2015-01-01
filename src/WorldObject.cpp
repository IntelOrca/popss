#include "WorldObject.h"

using namespace IntelOrca::PopSS;

WorldObject::WorldObject()
{
	this->position = glm::vec3(0);
	this->ownership = OWNERSHIP_NEUTRAL;
	this->rotation = 0;
}

WorldObject::~WorldObject() { }

void WorldObject::Update() { }

void WorldObject::Draw() const { }