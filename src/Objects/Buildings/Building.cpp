#include "Building.h"
#include "../../World.h"

using namespace IntelOrca::PopSS;

Building::Building() : WorldObject()
{
	this->group = OBJECT_GROUP_BUILDING;
}

Building::~Building() { }

void Building::Update()
{
	this->y = gWorld->GetTile(this->x / World::TileSize, this->z / World::TileSize)->height;
}