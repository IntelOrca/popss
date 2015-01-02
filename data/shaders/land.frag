#version 330

uniform sampler2D InputTexture[8];

uniform bool InputHighlightActive;
uniform vec2 InputHighlight00;
uniform vec2 InputHighlight11;

in vec3 FragmentPosition;
in vec2 FragmentTextureCoords;
in float FragmentTexture[8];
in vec3 FragmentLighting;
in float FragmentFog;

out vec4 OutputColour;

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
	float luma = colour.r * 0.2126 + colour.g + 0.7152 + colour.b * 0.0722;
	vec3 fogColour = vec3(luma);
	colour.rgb = mix(colour.rgb, fogColour, FragmentFog);

	// Apply lighting
	colour *= vec4(FragmentLighting, 1.0);

	// Apply highlight
	if (
		InputHighlightActive &&
		FragmentPosition.x >= InputHighlight00.x && FragmentPosition.x <= InputHighlight11.x &&
		FragmentPosition.z >= InputHighlight00.y && FragmentPosition.z <= InputHighlight11.y
	) {
		colour.rgb += vec3(0.5);
	}

	// Apply long distance fade out
	if (FragmentFog >= 0.75)
		colour.rgb = mix(colour.rgb, vec3(0), (FragmentFog - 0.75) / 0.25);

	// Output the colour
	OutputColour = colour;
}