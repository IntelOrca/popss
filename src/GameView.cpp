#include "GameView.h"

using namespace IntelOrca::PopSS;

GameView *IntelOrca::PopSS::gGameView;

GameView::GameView()
{
	this->camera.world = &this->world;
	this->landscapeRenderer.world = &this->world;

	this->world.LoadLandFromPOPTB("data/maps/levl2011.dat");

	gWorld = &this->world;
}

GameView::~GameView()
{

}

void GameView::Update()
{
	if (updateCounter == 0) {
		this->landscapeRenderer.Initialise();
	}

	this->world.Update();

	// Input
	if (gIsScanKeyDown[SDL_SCANCODE_UP] || gIsKeyDown[SDLK_w])
		this->camera.MoveForwards();
	if (gIsScanKeyDown[SDL_SCANCODE_DOWN] || gIsKeyDown[SDLK_s])
		this->camera.MoveBackwards();
	if (gIsScanKeyDown[SDL_SCANCODE_LEFT] || gIsKeyDown[SDLK_a])
		this->camera.RotateLeft();
	if (gIsScanKeyDown[SDL_SCANCODE_RIGHT] || gIsKeyDown[SDLK_d])
		this->camera.RotateRight();

	if (gCursor.wheel != 0)
		this->camera.SetZoom(this->camera.zoom + (gCursor.wheel * 32));

	if (gCursor.button & SDL_BUTTON_LMASK) {
		glm::ivec3 worldPosition = this->camera.GetWorldPositionFromViewport(gCursor.x, gCursor.y);
		if (worldPosition != glm::ivec3(INT32_MIN)) {
			if (gCursorPress.button & SDL_BUTTON_LMASK) {
				this->world.landHighlightSource.x = worldPosition.x;
				this->world.landHighlightSource.z = worldPosition.z;
				this->world.landHighlightTarget.x = worldPosition.x;
				this->world.landHighlightTarget.z = worldPosition.z;
			} else {
				this->world.landHighlightTarget.x = worldPosition.x;
				this->world.landHighlightTarget.z = worldPosition.z;
			}
		}
		this->world.landHighlightActive = true;
	} else {
		this->world.landHighlightActive = false;
	}

	updateCounter++;
}

void GameView::Draw()
{
	this->landscapeRenderer.Render(&this->camera);

	this->camera.viewHasChanged = false;
}