#include "GameView.h"
#include "Unit.h"
#include "WorldObject.h"

using namespace IntelOrca::PopSS;

GameView *IntelOrca::PopSS::gGameView;

GameView::GameView()
{
	this->camera.world = &this->world;
	this->skyRenderer.world = &this->world;
	this->landscapeRenderer.world = &this->world;
	this->objectRenderer.world = &this->world;

	this->world.LoadLandFromPOPTB("data/maps/levl2011.dat");

	gWorld = &this->world;

	this->editLandMode = true;
	this->editLandX = -1;
	this->editLandZ = -1;
}

GameView::~GameView()
{

}

void GameView::Update()
{
	if (updateCounter == 0) {
		this->skyRenderer.Initialise();
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

	if (this->editLandMode) {
		if ((gCursorPress.button & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) || gCursor.x != this->lastCursorX || gCursor.y != this->lastCursorY) {
			glm::ivec3 worldPosition;
			if (this->camera.GetWorldPositionFromViewport(gCursor.x, gCursor.y, &worldPosition)) {
				this->editLandX = worldPosition.x / World::TileSize;
				this->editLandZ = worldPosition.z / World::TileSize;
			} else {
				this->editLandX = -1;
				this->editLandZ = -1;
			}
		}

		int landIncreaseDecrease = 0;
		if (gCursor.button & SDL_BUTTON_LMASK)
			landIncreaseDecrease = 1;
		else if (gCursor.button & SDL_BUTTON_RMASK)
			landIncreaseDecrease = -1;

		if (landIncreaseDecrease != 0 && this->editLandX != -1 && this->editLandZ != -1) {
			int *originalHeight = NULL;
			
			bool average = gIsScanKey[SDL_SCANCODE_LCTRL] & KEY_DOWN;
			if (average) {
				originalHeight = new int[this->world.size * this->world.size];
				for (int z = 0; z < this->world.size; z++)
					for (int x = 0; x < this->world.size; x++)
						originalHeight[x + z * this->world.size] = this->world.GetTile(x, z)->height;
			}

			int radius = 3;
			for (int z = -radius; z <= radius; z++) {
				for (int x = -radius; x <= radius; x++) {
					float distance = sqrt(x * x + z * z);
					if (distance > radius)
						continue;

					WorldTile *tile = this->world.GetTile(this->editLandX + x, this->editLandZ + z);

					if (average) {
						int targetHeight = 0;
						for (int zz = -1; zz <= 1; zz++)
							for (int xx = -1; xx <= 1; xx++)
								targetHeight += originalHeight[(this->editLandX + x + xx) + (this->editLandZ + z + zz) * this->world.size];
						targetHeight /= 9;

						int heightDiff = targetHeight - (int)tile->height;

						tile->height = clamp((int)tile->height + min(2, abs(heightDiff)) * glm::sign(heightDiff), 0, 1024);
					} else {
						int heightDiff = ((radius - distance) + 1) * 2;
						tile->height = clamp((int)tile->height + heightDiff * landIncreaseDecrease, 0, 1024);
					}
				}
			}

			if (average)
				delete[] originalHeight;

			for (int z = -radius * 2; z <= radius * 2; z++) {
				for (int x = -radius * 2; x <= radius * 2; x++) {
					this->world.ProcessTile(this->world.TileWrap(this->editLandX + x), this->world.TileWrap(this->editLandZ + z));
				}
			}

			this->camera.viewHasChanged = true;
		}
	} else {
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
	}

	updateCounter++;

	this->lastCursorX = gCursor.x;
	this->lastCursorY = gCursor.y;
}

void GameView::Draw()
{
	this->skyRenderer.Render(&this->camera);
	this->landscapeRenderer.Render(&this->camera);
	this->objectRenderer.Render(&this->camera);

	this->camera.viewHasChanged = false;
}