#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;

in  vec3  GeomPosition[];
in  vec2  GeomTextureCoords[];
in  float GeomTexture[8][];
in  vec3  GeomLighting[];
in  float GeomFog[];

out vec3  FragmentPosition;
out vec2  FragmentTextureCoords;
out float FragmentTexture[8];
out vec3  FragmentLighting;
out float FragmentFog;

void main() {
	FragmentPosition      = GeomPosition[0];
	FragmentTextureCoords = GeomTextureCoords[0];
	FragmentTexture[8]    = GeomTexture[8][0];
	FragmentLighting      = GeomLighting[0];
	FragmentFog           = GeomFog[0];

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	FragmentPosition      = GeomPosition[1];
	FragmentTextureCoords = GeomTextureCoords[1];
	FragmentTexture[8]    = GeomTexture[8][1];
	FragmentLighting      = GeomLighting[1];
	FragmentFog           = GeomFog[1];

	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	FragmentPosition      = GeomPosition[2];
	FragmentTextureCoords = GeomTextureCoords[2];
	FragmentTexture[8]    = GeomTexture[8][2];
	FragmentLighting      = GeomLighting[2];
	FragmentFog           = GeomFog[2];

	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}