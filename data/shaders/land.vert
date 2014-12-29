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

out vec2 FragmentTextureCoords;
out float FragmentTexture[8];
out vec3 FragmentLighting;
out float FragmentFog;

void main()
{
	// Distort the vertex position so that we get a spherical effect
	vec3 newVertexPosition = SphereDistort(VertexPosition, InputSphereRatio);

	// Fragment texture
	FragmentTextureCoords = VertexTextureCoords;
	for (int i = 0; i < 8; i++) {
		if (i == int(VertexTexture))
			FragmentTexture[i] = 1.0;
		else
			FragmentTexture[i] = 0.0;
	}

	// Calculate fragment lighting
	vec3 totalLighting = vec3(0.0);
	for (int i = 0; i < InputLightSourcesCount; i++) {
		totalLighting += PhongShading(
			// Ambient, Diffuse, Specular
			InputLightSources[i].Ambient, InputLightSources[i].Diffuse, InputLightSources[i].Specular,
			// Ambient, Diffuse, specular, shininess
			vec3(VertexMaterial.x), vec3(VertexMaterial.y), vec3(VertexMaterial.z), VertexMaterial.w,
			InputLightSources[i].Position, newVertexPosition, VertexNormal
		);
	}
	FragmentLighting = totalLighting;

	// Calculate fragment fog
	FragmentFog = GetFogFactor(VertexPosition, 256 * 4, 256 * 32);

	// Position
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(newVertexPosition, 1.0);
}