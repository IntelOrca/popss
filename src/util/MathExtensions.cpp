#include "MathExtensions.hpp"

bool PlaneIntersect(
	const glm::vec3 &orig, const glm::vec3 &dir,
	const glm::vec3 &p0, const glm::vec3 &pNormal,
	float *outT
) {
	float t = glm::dot(p0 - orig, pNormal) / glm::dot(dir, pNormal);
	if (t < 0)
		return false;
	
	*outT = t;
	return true;
}

bool TriangleIntersect(
	const glm::vec3 &orig, const glm::vec3 &dir,
	const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
	float *outT
) {
	const float epsilon = 0.000001f;

	glm::vec3 edge1, edge2, pvec, qvec, tvec;
	float det, invdet, u, v, t;

	// Find vectors for two edges sharing V1
	edge1 = v1 - v0;
	edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate u parameter
	pvec = glm::cross(dir, edge2);
	
	// If determinant is near zero, ray lies in plane of triangle
	det = glm::dot(edge1, pvec);
	
	// NOT CULLING
	if (det > -epsilon && det < epsilon) return false;
	invdet = 1.f / det;

	// Calculate distance from V1 to ray origin
	tvec = orig - v0;

	// Calculate u parameter and test bound
	u = glm::dot(tvec, pvec) * invdet;

	// The intersection lies outside of the triangle
	if (u < 0.0f || u > 1.0f)
		return false;

	// Prepare to test v parameter
	qvec = glm::cross(tvec, edge1);

	// Calculate V parameter and test bound
	v = glm::dot(dir, qvec) * invdet;
	
	// The intersection lies outside of the triangle
	if (v < 0.0f || u + v  > 1.0f)
		return false;

	t = glm::dot(edge2, qvec) * invdet;
	if (t > epsilon) {
		// Ray intersection
		*outT = t;
		return true;
	}

	// No hit
	return false;
}