#pragma once

#include "../PopSS.h"

template<typename T>
void splitdecimalcomponents(T x, int *whole, T *fraction)
{
	if (whole != NULL) *whole = (int)x;
	if (fraction != NULL) *fraction = x - (int)x;
}

template<typename T>
T wraprange(T low, T x, T high)
{
	while (x < low)
		x += (high - low);
	while (x >= high)
		x -= (high - low);
	return x;
}

template<typename T>
T wrapangledeg(T angle)
{
	return wraprange((T)-180, angle, (T)180);
}

template<typename T>
T smallestangledeltadeg(T a, T b)
{
	a = wrapangledeg(a);
	b = wrapangledeg(b);

	double result = b - a;
	result += (result > 180) ? -360 : (result < -180) ? 360 : 0;
	return result;
}

template<typename T>
T todegrees(T x)
{
	return x * (T)(180 / M_PI);
}

template<typename T>
T toradians(T x)
{
	return x * (T)(M_PI / 180);
}

bool PlaneIntersect(
	const glm::vec3 &orig, const glm::vec3 &dir,
	const glm::vec3 &p0, const glm::vec3 &pNormal,
	float *outT
);

/**
 * Möller–Trumbore intersection algorithm
 * @cite http://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 */
bool TriangleIntersect(
	const glm::vec3 &orig, const glm::vec3 &dir,
	const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
	float *out
);