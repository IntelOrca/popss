#include "../../GameView.h"
#include "../../OrcaShader.h"
#include "../../World.h"
#include "Unit.h"

using namespace IntelOrca::PopSS;

Unit::Unit() : WorldObject()
{
	this->group = OBJECT_GROUP_UNIT;
	this->movingToDestination = false;
	this->selected = false;
	this->requiresPathFind = false;
}

Unit::~Unit() { }

void Unit::Update()
{
	const WorldTile *tile = gWorld->GetTile(this->x / World::TileSize, this->z / World::TileSize);

	if (this->movingToDestination && (this->position.x != this->destination.x || this->position.z != this->destination.z)) {
		if (this->pathToDestination.length == 0) {
			this->Stop();
		} else {
			if (this->pathToDestinationCurrentIndex >= this->pathToDestination.length - 1) {
				RunTo(this->destination.x, this->destination.z);
			} else {
				if (this->pathToDestinationCurrentIndex == 0)
					this->pathToDestinationCurrentIndex++;

				const PathPosition *pathtilepos = &this->pathToDestination.positions[this->pathToDestinationCurrentIndex];
				int pathposX = pathtilepos->x * World::TileSize + (World::TileSize / 2);
				int pathposZ = pathtilepos->z * World::TileSize + (World::TileSize / 2);

				glm::ivec2 delta = gWorld->GetClosestDelta(this->position.x, this->position.z, pathposX, pathposZ);
				int magnitude = sqrt(delta.x * delta.x + delta.y * delta.y);
				if (magnitude < World::TileSize / 4)
					this->pathToDestinationCurrentIndex++;

				RunTo(pathposX, pathposZ);
			}
		}
	} else {
		this->subposition = this->position;
	}

	this->SetYToLandHeight();
}

void Unit::RunTo(int targetX, int targetZ)
{
	float minSpeed = 0.1f;
	float maxSpeed = 5.0f;
	float acceleration = 0.4f;
	float deceleration = 0.25f;

	glm::ivec2 delta = gWorld->GetClosestDelta(this->position.x, this->position.z, targetX, targetZ);
	glm::vec3 direction = glm::vec3(delta.x, 0, delta.y);

	float directionMagnitude = glm::length(direction);
	direction = glm::normalize(direction);

	if (directionMagnitude <= maxSpeed) {
		this->position.x = this->destination.x;
		this->position.z = this->destination.z;
		this->Stop();
	} else {
		float resistance = 1 - (gWorld->GetTile(this->position.x, this->position.z)->steepness / 1024.0f);
		resistance = resistance * resistance;
		maxSpeed = maxSpeed * resistance;
		acceleration = acceleration * resistance;

		float currentSpeed = glm::length(this->velocity);
		if (currentSpeed > maxSpeed) currentSpeed = max(maxSpeed, currentSpeed - deceleration);
		if (currentSpeed < maxSpeed) currentSpeed = min(maxSpeed, currentSpeed + acceleration);

		this->velocity = direction * currentSpeed;
		this->subposition += this->velocity;
		this->position = this->subposition;
	}
}

void Unit::Stop()
{
	this->subposition = this->position;
	this->velocity = glm::vec3(0);
	this->movingToDestination = false;
}

void Unit::GiveMoveOrder(int x, int z)
{
	this->destination = glm::vec3(x, 0, z);
	this->movingToDestination = true;
	this->requiresPathFind = true;
	this->pathToDestinationCurrentIndex = 0;
}