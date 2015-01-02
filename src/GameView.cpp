#include "GameView.h"
#include "Unit.h"
#include "WorldObject.h"

using namespace IntelOrca::PopSS;

GameView *IntelOrca::PopSS::gGameView;

GameView::GameView()
{
	this->camera.world = &this->world;
	this->landscapeRenderer.world = &this->world;
	this->objectRenderer.world = &this->world;

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
		this->objectRenderer.Initialise();
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

	if (gCursorPress.button & SDL_BUTTON_RMASK) {
		for (Unit *unit : this->world.selectedUnits)
			unit->selected = false;

		this->world.selectedUnits.clear();
	}

	if (gCursor.button & SDL_BUTTON_LMASK) {
		glm::ivec3 worldPosition;
		if (this->camera.GetWorldPositionFromViewport(gCursor.x, gCursor.y, &worldPosition)) {
			if (this->world.selectedUnits.size() == 0) {
				if (gCursorPress.button & SDL_BUTTON_LMASK) {
					this->world.landHighlightSource.x = worldPosition.x;
					this->world.landHighlightSource.z = worldPosition.z;
					this->world.landHighlightTarget.x = worldPosition.x;
					this->world.landHighlightTarget.z = worldPosition.z;
				} else {
					this->world.landHighlightTarget.x = worldPosition.x;
					this->world.landHighlightTarget.z = worldPosition.z;
				}
				this->world.landHighlightActive = true;
			} else {
				for (Unit *unit : this->world.selectedUnits)
					unit->GiveMoveOrder(worldPosition.x, worldPosition.z);
			}
		}
	} else {
		if (this->world.landHighlightActive) {
			glm::ivec3 source = this->world.landHighlightSource;
			glm::ivec3 target = this->world.landHighlightTarget;

			for (WorldObject *obj : this->world.objects) {
				if (obj->x >= source.x && obj->z >= source.z && obj->x <= target.x && obj->z <= target.z) {
					if (obj->group == OBJECT_GROUP_UNIT) {
						Unit *unit = (Unit*)obj;
						unit->selected = true;
						this->world.selectedUnits.push_back(unit);
					}
				}
			}
		}
		this->world.landHighlightActive = false;
	}

	updateCounter++;
}

void GameView::Draw()
{
	this->landscapeRenderer.Render(&this->camera);
	this->objectRenderer.Render(&this->camera);

	this->camera.viewHasChanged = false;
}