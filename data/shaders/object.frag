#version 330

uniform sampler2D InputTexture;

in vec2 FragmentTextureCoords;
in vec3 FragmentLighting;
in float FragmentFog;

out vec4 OutputColour;

void main()
{
	vec4 colour = texture(InputTexture, FragmentTextureCoords.xy);
	// if (colour.a == 0) discard;

	// Apply fog
	vec3 fogcolour = vec3(0.5, 0.5, 0.7);
	float fogalpha = min(0.5, FragmentFog * 0.5);
	colour.rgb = colour.rgb * (1 - fogalpha) + fogcolour * fogalpha;

	// Apply lighting
	colour.rgb *= FragmentLighting;

	// Output colour
	OutputColour = colour;
}