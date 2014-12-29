#version 400

#include "landscape.glsl"
#include "lighting.glsl"

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

uniform float InputSphereRatio;

// Light sources
uniform int InputLightSourcesCount;
uniform LightSource InputLightSources[8];

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTextureCoords;
layout (location = 3) in float VertexTexture;
layout (location = 4) in vec4 VertexMaterial;

out vec3 FragmentPosition;
out vec3 FragmentNormal;
out vec2 FragmentTextureCoords;
out vec4 FragmentTexture;
out vec3 FragmentLighting;
out float FragmentFog;

void main()
{
	if (InputSphereRatio == 0) {
		gl_Position = vec4(0.0);
		return;
	}

	// Distort the vertex position so that we get a spherical effect
	vec3 newVertexPosition = SphereDistort(VertexPosition, InputSphereRatio);
	FragmentPosition = VertexPosition;
	FragmentNormal = VertexNormal;

	// Fragment texture
	FragmentTextureCoords = VertexTextureCoords;
	FragmentTexture = vec4(0.0);

	// Calculate fragment lighting
	vec3 totalLighting = vec3(0.0);
	for (int i = 0; i < InputLightSourcesCount; i++) {
		totalLighting += PhongShading(
			// Ambient, Diffuse, specular
			InputLightSources[i].Ambient, InputLightSources[i].Diffuse, InputLightSources[i].Specular,
			// Ambient, Diffuse, specular, shininess
			vec3(1.0), vec3(0.0, 0.0, 1.0), vec3(4.0), 16.0,
			InputLightSources[i].Position, newVertexPosition, VertexNormal
		);
	}
	FragmentLighting = totalLighting;

	// Calculate fragment fog
	FragmentFog = GetFogFactor(VertexPosition, 256 * 8, 256 * 16);

	// Position
    gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(newVertexPosition, 1.0);
}