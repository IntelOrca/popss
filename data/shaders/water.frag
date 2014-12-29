#version 400

#include "lighting.glsl"

uniform sampler2D InputTexture0;

// Light sources
uniform int InputLightSourcesCount;
uniform LightSource InputLightSources[8];

in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 FragmentTextureCoords;
in vec4 FragmentTexture;
in vec3 FragmentLighting;
in float FragmentFog;

layout (location = 0) out vec4 OutputColour;

const vec2 size = vec2(6.0,0.0);
const ivec3 off = ivec3(-1,0,1);

void main()
{
	// bump
	vec4 wave = texture(InputTexture0, FragmentTextureCoords);
    float s11 = wave.x;
    float s01 = textureOffset(InputTexture0, FragmentTextureCoords, off.xy).x;
    float s21 = textureOffset(InputTexture0, FragmentTextureCoords, off.zy).x;
    float s10 = textureOffset(InputTexture0, FragmentTextureCoords, off.yx).x;
    float s12 = textureOffset(InputTexture0, FragmentTextureCoords, off.yz).x;
    vec3 va = normalize(vec3(size.x,s21-s01,size.y));
    vec3 vb = normalize(vec3(size.y,s12-s10,-size.x));
    vec4 bump = vec4( cross(va,vb), s11 );

	vec3 position = FragmentPosition;
	position.y += bump.w;

	// Calculate fragment lighting
	vec3 totalLighting = vec3(0.0);
	for (int i = 0; i < InputLightSourcesCount; i++) {
		/*
		totalLighting += InputLightSources[i].Ambient;

		totalLighting += DiffuseShading(
			InputLightSources[i].Diffuse,
			vec3(0.0, 0.0, 1.0),
			InputLightSources[i].Position, position, bump.xyz
		);

		totalLighting += SpecularShading(
			InputLightSources[i].Diffuse,
			vec3(1.0),
			32.0,
			InputLightSources[i].Position, position, FragmentNormal
		);
		*/

		totalLighting += PhongShading(
			// Ambient, Diffuse, specular
			InputLightSources[i].Ambient, InputLightSources[i].Diffuse, InputLightSources[i].Specular,
			// Ambient, Diffuse, specular, shininess
			vec3(1.0), vec3(0.0, 0.0, 1.0), vec3(4.0), 64.0,
			InputLightSources[i].Position, position, bump.xyz
		);
	}

	OutputColour = vec4(totalLighting, 1.0);

	/*
	vec3 texel = texture(InputTexture0, FragmentTextureCoords.xy).rgb;
	vec4 colour = vec4(texel, 1.0);

	vec4 fogColour = vec4(vec3(0.75), 1.0);
	colour = mix(colour, fogColour, FragmentFog);
	colour *= vec4(FragmentLighting, 1.0);

	OutputColour = colour;
	*/
}