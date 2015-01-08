#include "../../GameView.h"
#include "../../OrcaShader.h"
#include "../../World.h"
#include "Unit.h"

using namespace IntelOrca::PopSS;

Unit::Unit() : WorldObject()
{
	this->group = OBJECT_GROUP_UNIT;
	this->movingToDestination = false;
}

Unit::~Unit() { }

void Unit::Update()
{
	const WorldTile *tile = gWorld->GetTile(this->x / World::TileSize, this->z / World::TileSize);
	const float speed = 2.0f;

	if (this->movingToDestination && this->position != this->destination) {
		glm::vec3 pos = this->position;
		glm::vec3 dst = this->destination;

		pos.y = 0;
		dst.y = 0;

		glm::vec3 direction = dst - pos;

		if (glm::length(direction) <= speed) {
			this->position = this->destination;
			this->subposition = this->position;
			this->velocity = glm::vec3(0);
			this->movingToDestination = false;
		} else {
			this->velocity = glm::normalize(direction) * speed;
			this->subposition += this->velocity;
			this->position = this->subposition;
		}
	} else {
		this->subposition = this->position;
	}

	this->SetYToLandHeight();
}

void Unit::GiveMoveOrder(int x, int z)
{
	this->destination = glm::vec3(x, 0, z);
	this->movingToDestination = true;
}