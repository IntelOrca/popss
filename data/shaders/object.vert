#version 400

#include "landscape.glsl"
#include "lighting.glsl"

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

uniform float InputSphereRatio;

// Light sources
uniform int InputLightSourcesCount;
uniform LightSource InputLightSources[8];

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 FragmentLighting;
out float FragmentFog;

void main()
{
	vec3 modelVertexPosition = (ModelMatrix * vec4(VertexPosition, 1.0)).xyz;
	vec3 distortedVertexPosition = SphereDistort(modelVertexPosition, InputSphereRatio);

	// Calculate fragment lighting
	vec3 totalLighting = vec3(0.0);
	for (int i = 0; i < InputLightSourcesCount; i++) {
		totalLighting += PhongShading(
			// Ambient, Diffuse, Specular
			InputLightSources[i].Ambient, InputLightSources[i].Diffuse, InputLightSources[i].Specular,
			// Ambient, Diffuse, specular, shininess
			vec3(1.0), vec3(1.0), vec3(1.0), 16.0,
			InputLightSources[i].Position, distortedVertexPosition, VertexNormal
		);
	}
	FragmentLighting = totalLighting;

	// Calculate fragment fog
	FragmentFog = GetFogFactor(modelVertexPosition, 256 * 4, 256 * 32);

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(distortedVertexPosition, 1.0);
}