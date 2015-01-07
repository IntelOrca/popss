#include "Wildman.h"

using namespace IntelOrca::PopSS;

Wildman::Wildman() : Unit()
{
	this->type = UNIT_WILDMAN;
}

Wildman::~Wildman() { }

void Wildman::Update()
{
	Unit::Update();
}