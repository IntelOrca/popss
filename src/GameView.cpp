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
	this->camera.Update();

	// Input
	if (gIsKey[SDLK_f] & KEY_PRESSED) {
		this->landscapeRenderer.debugRenderType++;
		if (this->landscapeRenderer.debugRenderType > DEBUG_LANDSCAPE_RENDER_TYPE_POINTS)
			this->landscapeRenderer.debugRenderType = DEBUG_LANDSCAPE_RENDER_TYPE_NONE;
		this->objectRenderer.debugRenderType = this->landscapeRenderer.debugRenderType;
	}

	if ((gIsScanKey[SDL_SCANCODE_UP] & KEY_DOWN) || (gIsKey[SDLK_w] & KEY_DOWN))
		this->camera.MoveForwards();
	if ((gIsScanKey[SDL_SCANCODE_DOWN] & KEY_DOWN) || (gIsKey[SDLK_s] & KEY_DOWN))
		this->camera.MoveBackwards();
	if ((gIsScanKey[SDL_SCANCODE_LEFT] & KEY_DOWN) || (gIsKey[SDLK_a] & KEY_DOWN))
		this->camera.RotateLeft();
	if ((gIsScanKey[SDL_SCANCODE_RIGHT] & KEY_DOWN) || (gIsKey[SDLK_d] & KEY_DOWN))
		this->camera.RotateRight();

	if (gCursor.wheel < 0)
		this->camera.ZoomOut();
	else if (gCursor.wheel > 0)
		this->camera.ZoomIn();

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