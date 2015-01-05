#version 330

uniform sampler2D uShadowTexture;
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

vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Classic Perlin noise
float cnoise(vec2 P)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod289(Pi); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;

  vec4 i = permute(permute(ix) + iy);

  vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
  vec4 gy = abs(gx) - 0.5 ;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;

  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);

  vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.x;  
  g01 *= norm.y;  
  g10 *= norm.z;  
  g11 *= norm.w;  

  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));

  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

// Classic Perlin noise, periodic variant
float pnoise(vec2 P, vec2 rep)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, rep.xyxy); // To create noise with explicit period
  Pi = mod289(Pi);        // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;

  vec4 i = permute(permute(ix) + iy);

  vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
  vec4 gy = abs(gx) - 0.5 ;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;

  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);

  vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.x;  
  g01 *= norm.y;  
  g10 *= norm.z;  
  g11 *= norm.w;  

  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));

  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

float perlin(in vec2 pos)
{
	float total = 0;
	float p = 1.0 / 1.25;
	float n = 12;

	for (float i = 0; i < n; i++) {
		float frequency = pow(2.0, i);
		float amplitude = pow(p, i);

		total = total + cnoise(pos * frequency) * amplitude;
	}

	return total;
}

void main()
{
	// float noisy = 0.5 + (perlin(FragmentPosition.xz / 64) * 0.5);

	// Interpolate between specified textures
	vec3 multiTexel = vec3(0.0);
	for (int i = 0; i < 8; i++) {
		float tamount = FragmentTexture[i];
		if (tamount == 0.0)
			continue;

		// float distFromEdge = min(
		// 	abs(1 - tamount),
		// 	abs(0 - tamount)
		// );
		// distFromEdge = max(0.1, distFromEdge / 0.1);
		// 
		// if (i % 2 == 0) tamount = mix(tamount, noisy, distFromEdge);
		// else tamount = mix(tamount, 1 - noisy, distFromEdge);
		
		multiTexel += vec3(tamount) * texture(InputTexture[i], FragmentTextureCoords.xy).rgb;
	}

	// Apply texture
	vec4 colour = vec4(multiTexel, 1.0);

	// vec3 sand = texture(InputTexture[0], FragmentTextureCoords.xy).rgb;
	// vec3 cliff = texture(InputTexture[3], FragmentTextureCoords.xy).rgb;
	// colour = vec4(mix(sand, cliff, noisy), 1.0);

	// Apply lighting
	colour *= vec4(FragmentLighting, 1.0);

	// Apply shadow
	float shadowAmount = texture(uShadowTexture, FragmentPosition.xz / (256.0 * 128.0)).r;
	// if (shadowAmount > 0.1) colour.rgb = vec3(0);
	shadowAmount *= 0.1;
	colour.rgb -= vec3(shadowAmount);

	// Apply fog
	vec3 fogcolour = vec3(0.5, 0.5, 0.7);
	float fogalpha = min(0.5, FragmentFog * 0.5);
	colour.rgb = colour.rgb * (1 - fogalpha) + fogcolour * fogalpha;

	// Apply highlight
	if (
		InputHighlightActive &&
		FragmentPosition.x >= InputHighlight00.x && FragmentPosition.x <= InputHighlight11.x &&
		FragmentPosition.z >= InputHighlight00.y && FragmentPosition.z <= InputHighlight11.y
	) {
		colour.rgb += vec3(0.5);
	}

	// Apply long distance fade out
	// if (FragmentFog >= 0.75)
	//	colour.rgb = mix(colour.rgb, vec3(0), (FragmentFog - 0.75) / 0.25);

	// Output the colour
	OutputColour = colour;
}