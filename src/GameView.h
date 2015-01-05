#pragma once

#include "Camera.h"
#include "SkyRenderer.h"
#include "LandscapeRenderer.h"
#include "ObjectRenderer.h"
#include "PopSS.h"
#include "World.h"

namespace IntelOrca { namespace PopSS {

class GameView {
public:
	Camera camera;
	World world;

	GameView();
	~GameView();

	void Update();
	void Draw();

private:
	int updateCounter;

	SkyRenderer skyRenderer;
	LandscapeRenderer landscapeRenderer;
	ObjectRenderer objectRenderer;

	bool editLandMode;
	int editLandX, editLandZ;
	int lastCursorX, lastCursorY;
};

extern GameView *gGameView;

} }