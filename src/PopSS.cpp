#include "Audio.h"
#include "PopSS.h"
#include "GameView.h"
#include "glUtil.h"

SDL_Window *glWindow;
SDL_GLContext gglContext;
bool gIsScanKeyDown[256] = { 0 };
bool gIsKeyDown[256] = { 0 };

static bool _quit = false;

static bool _updateStepMode = false;
static bool _updateStepModeCanStep = false;
static int _updateStep = 1;

static IntelOrca::PopSS::GameView *_gameView;

bool init_sdl()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
	glWindow = SDL_CreateWindow("PopSS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (glWindow == NULL) {
		fprintf(stderr, "No window 4 u");
		return false;
	}

	gglContext = SDL_GL_CreateContext(glWindow);

	printf("%s\n", glGetString(GL_VENDOR));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glShadeModel(GL_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(100 / 255.0, 149 / 255.0, 237 / 255.0, 1);
	glViewport(0, 0, 1920, 1080);
	return true;
}

void exit_sdl()
{
	SDL_GL_DeleteContext(gglContext);
	SDL_DestroyWindow(glWindow);
	SDL_Quit();
}

void resize(int width, int height)
{
	glViewport(0, 0, width, height);

	GLdouble ratio = (double)height / width;
	glLoadIdentity();
	glFrustum(-1, 1, -ratio, ratio, 1, 500);
}

void handle_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_BACKSLASH:
					_updateStepMode = true;
					_updateStepModeCanStep = true;
					break;
				case SDLK_ESCAPE:
					_updateStepMode = false;
					break;
				case SDLK_TAB:
					_updateStep = 8;
					break;
				}
				if (event.key.keysym.scancode < countof(gIsScanKeyDown))
					gIsScanKeyDown[event.key.keysym.scancode] = true;
				if (event.key.keysym.sym < countof(gIsKeyDown))
					gIsKeyDown[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
				case SDLK_TAB:
					_updateStep = 1;
					break;
				}
				if (event.key.keysym.scancode < countof(gIsScanKeyDown))
					gIsScanKeyDown[event.key.keysym.scancode] = false;
				if (event.key.keysym.sym < countof(gIsKeyDown))
					gIsKeyDown[event.key.keysym.sym] = false;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
					resize(event.window.data1, event.window.data2);
				break;
			case SDL_QUIT:
				_quit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEMOTION:
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			case SDL_MOUSEWHEEL:
				break;
		}
	}
}

void update()
{
	_gameView->Update();
}

void draw()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	_gameView->Draw();

	SDL_GL_SwapWindow(glWindow);
}

int main(int argc, char** argv)
{
	long lastTicks = 0, ticks;

	srand(time(NULL));
	init_sdl();

	_gameView = new IntelOrca::PopSS::GameView();

	while (!_quit) {
		handle_events();

		ticks = SDL_GetTicks();
		if (ticks + (1000 / 60) > lastTicks) {
			lastTicks = ticks;

			if (!_updateStepMode || (_updateStepMode && _updateStepModeCanStep)) {
				for (int i = 0; i < _updateStep; i++)
					update();

				_updateStepModeCanStep = false;
			}

			draw();
		}
	}

	exit_sdl();

	return 0;
}