#include "Shaman.h"

using namespace IntelOrca::PopSS;

Shaman::Shaman() : Unit()
{
	this->type = UNIT_SHAMAN;
}

Shaman::~Shaman() { }

void Shaman::Update()
{
	Unit::Update();
}