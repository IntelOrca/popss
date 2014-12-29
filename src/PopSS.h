#pragma once

#define _CRT_SECURE_NO_WARNINGS
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
	#include <gl/gl.h>
	#include <gl/glext.h>
#elif __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
#else
	#include <GL/gl.h>
#endif

#include <SDL.h>

#include <glm/glm.hpp>

#include "glUtil.h"

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
};

typedef trect<float> rect;
typedef trect<int> recti;

extern SDL_Window *glWindow;
extern SDL_GLContext gglContext;

extern bool gIsScanKeyDown[];
extern bool gIsKeyDown[];