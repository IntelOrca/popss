﻿// Lighting include shader

vec3 SphereDistort(in vec3 position, in vec3 cameraTarget, in float ratio)
{
	vec3 relative = position - cameraTarget;
	return vec3(
		position.x,
		position.y - (ratio * (relative.x * relative.x + relative.z * relative.z)),
		position.z
	);
}

float GetFogFactor(in vec3 position, in vec3 cameraTarget, in float minDistance, in float maxDistance)
{
	vec3 relative = position - cameraTarget;
	float distance = sqrt(relative.x * relative.x + relative.z * relative.z);
	if (distance > minDistance)
		return clamp((distance - minDistance) / maxDistance, 0.0, 1.0);
	else
		return 0.0;
}