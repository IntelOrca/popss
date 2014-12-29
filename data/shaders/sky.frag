#version 400

uniform sampler2D InputCloudTexture;

uniform vec2 InputSkySize;
uniform vec2 InputSunPosition;
uniform vec4 InputSunColour;
uniform vec3 InputSkyColour;

in vec2 FragmentTextureCoords;

layout (location = 0) out vec4 OutputColour;

void main()
{
	vec4 texel = texture(InputCloudTexture, FragmentTextureCoords);
	OutputColour = vec4(mix(InputSkyColour, vec3(1.0, 1.0, 1.0), texel.x), 1.0);

	/*
	float xDiffA = abs((InputSunPosition.x - InputSkySize.x) - FragmentPosition.x);
	float xDiffB = abs((InputSunPosition.x                 ) - FragmentPosition.x);
	float xDiffC = abs((InputSunPosition.x + InputSkySize.x) - FragmentPosition.x);
	float x = min(min(xDiffA, xDiffB), xDiffC) * 2;
	float y = InputSunPosition.y - FragmentPosition.y;
	float distanceFromSun = sqrt(x * x + y * y);

	float intensity = InputSunColour.w;
	distanceFromSun = pow(distanceFromSun, intensity);
	float maxDistanceFromSun = pow(InputSkySize.x, intensity);

	float sunStrength = 1.0 - clamp(distanceFromSun / maxDistanceFromSun, 0.0, 1.0);
	OutputColour = vec4(mix(InputSkyColour, InputSunColour.rgb, sunStrength), 1.0);
	*/
}