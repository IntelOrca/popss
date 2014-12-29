#include "glUtil.h"

#ifdef WIN32
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
#endif

void initOpenGLCalls() {

	#ifdef WIN32
		glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
	#endif

}

void projectFlat(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -10000, 10000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void projectIsometric() {

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-10.0f, 10.0f, -10.0f, 10.0f, -50.0f, 50.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(35.264, 1.0, 0.0, 0.0);
	
	glRotatef(-45.0, 0.0, 1.0, 0.0);

}

void glDrawEllipse(float radiusX, float radiusY)
{
	if (radiusX <= 0 || radiusY <= 0)
		return;

	glBegin(GL_LINES);
	int numSectors = (int)max(radiusX, radiusY) / 2;
	float sectorAngle = M_2PI / numSectors;
	for (float angle = 0; angle < M_2PI; angle += sectorAngle) {
		float x0 = radiusX * cos(angle);
		float y0 = radiusY * sin(angle);
		float x1 = radiusX * cos(angle + sectorAngle);
		float y1 = radiusY * sin(angle + sectorAngle);

		glVertex2f(x0, y0);
		glVertex2f(x1, y1);
	}
	glEnd();
}

void glFillEllipse(float radiusX, float radiusY)
{
	if (radiusX <= 0 || radiusY <= 0)
		return;

	glBegin(GL_POLYGON);
	int numSectors = (int)max(radiusX, radiusY);
	float sectorAngle = M_2PI / numSectors;
	for (float angle = 0; angle < M_2PI; angle += sectorAngle) {
		float x0 = radiusX * cos(angle);
		float y0 = radiusY * sin(angle);
		float x1 = radiusX * cos(angle + sectorAngle);
		float y1 = radiusY * sin(angle + sectorAngle);

		glVertex2f(x0, y0);
		glVertex2f(x1, y1);
	}
	glEnd();
}