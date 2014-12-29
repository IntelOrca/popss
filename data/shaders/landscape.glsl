// Lighting include shader

vec3 SphereDistort(vec3 position, float ratio)
{
	return vec3(
		position.x,
		position.y - (ratio * (position.x * position.x + position.z * position.z)),
		position.z
	);
}

float GetFogFactor(vec3 position, float minDistance, float maxDistance)
{
	float distance = sqrt(position.x * position.x + position.z * position.z);
	if (distance > minDistance)
		return clamp((distance - minDistance) / maxDistance, 0.0, 1.0);
	else
		return 0.0;
}