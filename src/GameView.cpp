#include "GameView.h"

using namespace IntelOrca::PopSS;

GameView::GameView()
{
	this->camera.world = &this->world;
	this->landscapeRenderer.world = &this->world;

	this->world.LoadLandFromPOPTB("data/maps/levl2011.dat");
}

GameView::~GameView()
{

}

void GameView::Update()
{
	// Input
	if (gIsScanKeyDown[SDL_SCANCODE_UP])
		this->camera.MoveForwards();
	if (gIsScanKeyDown[SDL_SCANCODE_DOWN])
		this->camera.MoveBackwards();
	if (gIsScanKeyDown[SDL_SCANCODE_LEFT])
		this->camera.RotateLeft();
	if (gIsScanKeyDown[SDL_SCANCODE_RIGHT])
		this->camera.RotateRight();
}

void GameView::Draw()
{
	this->landscapeRenderer.Render(&this->camera);
}