#version 330

#include "lighting.glsl"

uniform float iGlobalTime;
uniform vec3 InputCameraPosition;
uniform vec3 InputCameraTarget;
uniform sampler2D InputTexture0;

// Light sources
uniform int InputLightSourcesCount;
uniform LightSource InputLightSources[8];

in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 FragmentTextureCoords;
in vec3 FragmentLighting;
in float FragmentFog;

out vec4 OutputColour;

float diffuse(vec3 n,vec3 l,float p) {
	return pow(dot(n,l) * 0.4 + 0.6,p);
}
float specular(vec3 n,vec3 l,vec3 e,float s) {    
	float nrm = (s + 8.0) / (3.1415 * 8.0);
	return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

float hash(in vec2 p) {
	float h = dot(p, vec2(127.1,311.7));
	return fract(sin(h) * 43758.5453123);
}

float noise(in vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	return -1.0 + 2.0 * mix(mix(hash(i + vec2(0.0,0.0)),
						hash(i + vec2(1.0,0.0)), u.x),
						mix(hash(i + vec2(0.0,1.0)),
						hash(i + vec2(1.0,1.0)), u.x), u.y);
}

float sea_octave(in vec2 uv, in float choppy) {
	uv += noise(uv);
	vec2 wv = 1.0 - abs(sin(uv));
	vec2 swv = abs(cos(uv));
	wv = mix(wv, swv, wv);
	return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
}

const int NUM_STEPS = 8;
const float PI	 	= 3.1415;
const float EPSILON	= 1e-3;
float EPSILON_NRM	= 0.1 / 1920;

const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.8;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3 SEA_BASE = vec3(0.1, 0.19, 0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8, 0.9, 0.6);

float SEA_TIME = iGlobalTime * SEA_SPEED;
mat2 octave_m = mat2(1.6, 1.2, -1.2, 1.6);

float map(in vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;

	float d, h = 0.0;
	for(int i = 0; i < ITER_GEOMETRY; i++) {
		d = sea_octave((uv + SEA_TIME) * freq, choppy);
		d += sea_octave((uv - SEA_TIME) * freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

float map_detailed(vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;
	
	float d, h = 0.0;
	for(int i = 0; i < ITER_FRAGMENT; i++) {
		d = sea_octave((uv + SEA_TIME) * freq, choppy);
		d += sea_octave((uv - SEA_TIME) * freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

vec3 getSkyColor(in vec3 e) {
	e.y = max(e.y, 0.0);
	vec3 ret;
	ret.x = pow(1.0 - e.y, 2.0);
	ret.y = 1.0 - e.y;
	ret.z = 0.6 + (1.0 - e.y) * 0.4;
	return ret;
}

vec3 getSeaColor(in vec3 p, in vec3 n, in vec3 l, in vec3 eye, in vec3 dist) {
	float fresnel = 1.0 - max(dot(n, -eye), 0.0);
	fresnel = pow(fresnel,3.0) * 0.65;

	vec3 reflected = getSkyColor(reflect(eye,n));
	vec3 refracted = SEA_BASE + diffuse(n,l,80.0) * SEA_WATER_COLOR * 0.12;

	vec3 color = mix(refracted,reflected,fresnel);

	float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
	color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;

	color += vec3(specular(n, l, eye, 60.0));

	return color;
}

vec3 getNormal(vec3 p, float eps) {
    vec3 n;
    n.y = map_detailed(p);    
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - n.y;
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

void main()
{
	vec4 colour = vec4(vec3(0), 1);
	vec3 p = FragmentPosition / 64;
	p.y = 0.6;

	vec3 dist = (FragmentPosition - InputCameraPosition) / 64;
	vec3 n = getNormal(p, dot(dist, dist) * EPSILON_NRM);
	vec3 light = normalize(vec3(0.0, 1.0, 0.8));
	colour.rgb = getSeaColor(p, n, light, normalize(dist), dist);

	OutputColour = colour;
	// texture(InputTexture0, FragmentTextureCoords);
}