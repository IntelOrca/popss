#version 400

in vec3 FragmentLighting;
in float FragmentFog;

layout (location = 0) out vec4 OutputColour;

void main()
{
	vec4 colour = vec4(1.0);

	// Apply fog
	vec4 fogColour = vec4(0.5, 0.5, 0.5, 1.0);
	colour = mix(colour, fogColour, FragmentFog);

	// Apply lighting
	colour *= vec4(FragmentLighting, 1.0);

	if (FragmentFog >= 0.75)
		colour.a = mix(colour.a, 0.0, (FragmentFog - 0.75) / 0.25);

	// Output colour
	OutputColour = colour;
}