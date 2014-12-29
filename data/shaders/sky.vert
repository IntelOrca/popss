#version 400

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTextureCoords;

out vec2 FragmentTextureCoords;

void main()
{
	FragmentTextureCoords = VertexTextureCoords;
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(VertexPosition, 1.0);
}