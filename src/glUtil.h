#pragma once

#include "PopSS.h"

#ifdef WIN32
	extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
#endif

void initOpenGLCalls();
void projectFlat(int width, int height);
void projectIsometric();

void glDrawEllipse(float radiusX, float radiusY);
void glFillEllipse(float radiusX, float radiusY);