#include "../World.h"
#include "WorldObject.h"

using namespace IntelOrca::PopSS;

WorldObject::WorldObject()
{
	this->type = 0;
	this->group = 0;
	this->position = glm::vec3(0);
	this->ownership = OWNERSHIP_NEUTRAL;
	this->rotation = 0;
}

WorldObject::~WorldObject() { }

void WorldObject::Update() { }

void WorldObject::SetYToLandHeight()
{
	this->y = gWorld->GetHeight(this->x, this->z);
}