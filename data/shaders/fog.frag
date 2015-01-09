#version 330

uniform vec3 uSkyColour;

out vec4 OutputColour;

void main()
{
	OutputColour = vec4(vec3(uSkyColour), 1.0);
}