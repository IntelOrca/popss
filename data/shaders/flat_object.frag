#version 400

uniform sampler2D InputTexture;

in vec2 FragmentTextureCoords;
in vec4 FragmentColour;

layout (location = 0) out vec4 OutputColour;

void main()
{
	// Get texture pixel
	vec4 texel = texture(InputTexture, FragmentTextureCoords);

	// Set colour
	texel *= FragmentColour;

	// Output colour
	OutputColour = texel;
}