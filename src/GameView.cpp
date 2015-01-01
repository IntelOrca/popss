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
	if (gIsScanKeyDown[SDL_SCANCODE_UP])
		this->camera.MoveForwards();
	if (gIsScanKeyDown[SDL_SCANCODE_DOWN])
		this->camera.MoveBackwards();
	if (gIsScanKeyDown[SDL_SCANCODE_LEFT])
		this->camera.RotateLeft();
	if (gIsScanKeyDown[SDL_SCANCODE_RIGHT])
		this->camera.RotateRight();

	if (gCursor.wheel != 0)
		this->camera.SetZoom(this->camera.zoom + (gCursor.wheel * 32));

	if (gCursor.button & SDL_BUTTON_LMASK) {
		glm::ivec3 worldPosition = this->camera.GetWorldPositionFromViewport(gCursor.x, gCursor.y);
		worldPosition.x = this->world.Wrap(worldPosition.x);
		worldPosition.z = this->world.Wrap(worldPosition.z);
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