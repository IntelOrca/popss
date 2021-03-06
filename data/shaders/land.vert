#version 330

#include "landscape.glsl"
#include "lighting.glsl"

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

uniform float InputSphereRatio;
uniform vec3 InputCameraTarget;

// Light sources
uniform int InputLightSourcesCount;
uniform LightSource InputLightSources[8];

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTextureCoords;
in int VertexTexture;
in vec4 VertexMaterial;

out vec3 FragmentPosition;
out vec2 FragmentTextureCoords;
out float FragmentTexture[8];
out vec3 FragmentLighting;
out float FragmentFog;

void main()
{
	vec3 modelVertexPosition = (ModelMatrix * vec4(VertexPosition, 1.0)).xyz;
	vec3 distortedVertexPosition = SphereDistort(modelVertexPosition, InputCameraTarget, InputSphereRatio);

	FragmentPosition = modelVertexPosition;

	// Fragment texture
	FragmentTextureCoords = VertexTextureCoords;
	for (int i = 0; i < 8; i++) {
		if (i == VertexTexture)
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
			InputLightSources[i].Position, distortedVertexPosition, VertexNormal
		);
	}
	FragmentLighting = totalLighting;

	// Calculate fragment fog
	FragmentFog = GetFogFactor(modelVertexPosition, InputCameraTarget, 256 * 4, 256 * 32);

	// Position
	gl_Position = ProjectionMatrix * ViewMatrix * vec4(distortedVertexPosition, 1.0);
}