#version 330

#include "landscape.glsl"
#include "lighting.glsl"

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

uniform float InputSphereRatio;
uniform vec3 InputCameraTarget;

// Light sources
uniform int InputLightSourcesCount;
uniform LightSource InputLightSources[8];

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTextureCoords;

out vec3 FragmentPosition;
out vec3 FragmentNormal;
out vec2 FragmentTextureCoords;
out vec3 FragmentLighting;
out float FragmentFog;

void main()
{
	// Distort the vertex position so that we get a spherical effect
	vec3 newVertexPosition = SphereDistort(VertexPosition, InputCameraTarget, InputSphereRatio);
	FragmentPosition = VertexPosition;
	FragmentNormal = VertexNormal;

	// Fragment texture
	FragmentTextureCoords = VertexTextureCoords;

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
	FragmentFog = GetFogFactor(VertexPosition, InputCameraTarget, 256 * 8, 256 * 16);

	// Position
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(newVertexPosition, 1.0);
}