#version 330

uniform sampler2D InputTexture;

in vec2 FragmentTextureCoords;
in vec3 FragmentLighting;
in float FragmentFog;

out vec4 OutputColour;

void main()
{
	vec4 colour = texture(InputTexture, FragmentTextureCoords.xy);
	if (colour.a == 0)
		discard;

	// Apply fog
	vec4 fogColour = vec4(0.8, 0.8, 0.8, 1.0);
	colour = mix(colour, fogColour, FragmentFog);

	// Apply lighting
	colour *= vec4(FragmentLighting, 1.0);

	if (FragmentFog >= 0.75)
		colour.a = mix(colour.a, 0.0, (FragmentFog - 0.75) / 0.25);

	// Output colour
	OutputColour = colour;
}