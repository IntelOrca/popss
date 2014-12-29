#version 400

#include "landscape.glsl"

uniform float InputSphereRatio;

uniform mat4 gl_ModelViewMatrix;
uniform mat4 gl_ModelViewProjectionMatrix;

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTextureCoords;
layout (location = 2) in vec4 VertexColour;

out vec2 FragmentTextureCoords;
out vec4 FragmentColour;

void main()
{
	vec3 newVertexPosition = SphereDistort(VertexPosition, InputSphereRatio);

	FragmentTextureCoords = VertexTextureCoords;
	FragmentColour = VertexColour;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(newVertexPosition, 1.0);
}