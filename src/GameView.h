#pragma once

#include "Camera.h"
#include "LandscapeRenderer.h"
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
	LandscapeRenderer landscapeRenderer;
};

} }