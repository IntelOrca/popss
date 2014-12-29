#version 400

uniform sampler2D InputTexture[8];

in vec2 FragmentTextureCoords;
in float FragmentTexture[8];
in vec3 FragmentLighting;
in float FragmentFog;

layout (location = 0) out vec4 OutputColour;

void main()
{
	// Interpolate between specified textures
	vec3 multiTexel = vec3(0.0);
	for (int i = 0; i < 8; i++) {
		if (FragmentTexture[i] == 0.0)
			continue;

		multiTexel += vec3(FragmentTexture[i]) * texture(InputTexture[i], FragmentTextureCoords.xy).rgb;
	}

	// Apply texture
    vec4 colour = vec4(multiTexel, 1.0);

	// Apply fog
	vec4 fogColour = vec4(0.5, 0.5, 0.5, 1.0);
	colour = mix(colour, fogColour, FragmentFog);

	// Apply lighting
	colour *= vec4(FragmentLighting, 1.0);

	// Apply long distance fade out
	if (FragmentFog >= 0.75)
		colour.a = mix(colour.a, 0.0, (FragmentFog - 0.75) / 0.25);

	// Output the colour
	OutputColour = colour;
}