#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <time.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <cassert>

#include <list>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>

#ifdef WIN32
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include <GL/glew.h>
#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define abstract
#define interface struct
#define override virtual
#define abstract_method = 0

#define countof(x) (sizeof((x)) / sizeof((x)[0]))

#define M_2PI (M_PI * 2.0)

template<typename T>
T min(T a, T b) { return std::min(a, b); }

template<typename T>
T max(T a, T b) { return std::max(a, b); }

template<typename T>
T clamp(T x, T low, T high)
{
	return min(max(x, low), high);
}

template<typename T>
struct trect {
	T x, y, w, h;

	trect() { }
	trect(T x, T y, T w, T h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}

	T left() const { return this->x; }
	T top() const { return this->y; }
	T right() const { return this->x + this->w; }
	T bottom() const { return this->y + this->h; }

	void setLeft(T value) {
		this->w = this->x + this->w - value;
		this->x = value;
	}
	void setTop(T value) {
		this->h = this->y + this->h - value;
		this->y = value;
	}
	void setRight(T value) { this->w = value - this->x; }
	void setBottom(T value) { this->h = value - this->y; }
};

typedef trect<float> rect;
typedef trect<int> irect;

extern SDL_Window *glWindow;
extern SDL_GLContext gglContext;

enum {
	KEY_PRESSED = (1 << 0),
	KEY_DOWN = (1 << 1),
	KEY_RELEASED = (1 << 2)
};

extern unsigned char gIsScanKey[];
extern unsigned char gIsKey[];

struct cursor {
	int x, y, button, wheel;
};

extern cursor gCursor;
extern cursor gCursorPress;
extern cursor gCursorRelease;