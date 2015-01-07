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

out vec3 FragmentPosition;
out vec3 FragmentLighting;
out float FragmentFog;

void main()
{
	vec3 modelVertexPosition = (ModelMatrix * vec4(VertexPosition, 1.0)).xyz;
	vec3 distortedVertexPosition = SphereDistort(modelVertexPosition, InputCameraTarget, InputSphereRatio);

	FragmentPosition = modelVertexPosition;

	// Calculate fragment lighting
	vec3 totalLighting = vec3(0.0);
	for (int i = 0; i < InputLightSourcesCount; i++) {
		totalLighting += PhongShading(
			// Ambient, Diffuse, specular
			InputLightSources[i].Ambient, InputLightSources[i].Diffuse, InputLightSources[i].Specular,
			// Ambient, Diffuse, specular, shininess
			vec3(1.0), vec3(0.0, 0.0, 1.0), vec3(4.0), 16.0,
			InputLightSources[i].Position, modelVertexPosition, vec3(0, 1, 0)
		);
	}
	FragmentLighting = totalLighting;

	// Calculate fragment fog
	FragmentFog = GetFogFactor(modelVertexPosition, InputCameraTarget, 256 * 8, 256 * 16);

	// Position
	gl_Position = ProjectionMatrix * ViewMatrix * vec4(distortedVertexPosition, 1.0);
}