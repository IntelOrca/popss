#include "Tree.h"

using namespace IntelOrca::PopSS;

Tree::Tree() : WorldObject()
{
	this->type = SCENERY_TREE0;
	this->group = OBJECT_GROUP_SCENERY;
	this->wood = 4;
	this->maxWood = 4;
	this->ticksRemainingForNextWood = 0;
}

Tree::~Tree() { }

void Tree::Update()
{
	this->SetYToLandHeight();
}