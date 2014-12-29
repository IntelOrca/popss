#version 400

uniform mat4 gl_ModelViewMatrix;
uniform mat4 gl_ModelViewProjectionMatrix;
uniform mat4 gl_ProjectionMatrix;

layout (triangles) in;
layout (triangle_strip, max_vertices = 64) out;

out vec3 FragmentColour;

vec4 getMidpoint(vec4 a, vec4 b, vec4 c)
{
	vec3 minPoint = vec3(min(a.x, min(b.x, c.x)), min(a.y, min(b.y, c.y)), min(a.z, min(b.z, c.z)));
	vec3 maxPoint = vec3(max(a.x, max(b.x, c.x)), max(a.y, max(b.y, c.y)), max(a.z, max(b.z, c.z)));
	return vec4(
		minPoint.x + (maxPoint.x - minPoint.x) / 2.0,
		minPoint.y + (maxPoint.y - minPoint.y) / 2.0,
		minPoint.z + (maxPoint.z - minPoint.z) / 2.0,
		1.0
	);
}

void emitTriangle(vec4 a, vec4 b, vec4 c)
{
	gl_Position = gl_ModelViewProjectionMatrix * a;
	EmitVertex();
	gl_Position = gl_ModelViewProjectionMatrix * b;
	EmitVertex();
	gl_Position = gl_ModelViewProjectionMatrix * c;
	EmitVertex();
}

void tessellate(vec4 a, vec4 b, vec4 c, int levels)
{
	vec4 midPoint[2];

	if (levels == 1) {
		midPoint[0] = getMidpoint(a, b, c);
		emitTriangle(a, b, midPoint[0]);
		emitTriangle(b, c, midPoint[0]);
		emitTriangle(c, a, midPoint[0]);
	} else if (levels == 2) {
		midPoint[0] = getMidpoint(a, b, c);
		midPoint[1] = getMidpoint(a, b, midPoint[0]);
		emitTriangle(a, b, midPoint[1]);
		emitTriangle(b, c, midPoint[1]);
		emitTriangle(c, a, midPoint[1]);

		midPoint[1] = getMidpoint(b, c, midPoint[0]);
		emitTriangle(c, midPoint[0], midPoint[1]);
		emitTriangle(midPoint[0], b, midPoint[1]);
		emitTriangle(b, c, midPoint[1]);

		midPoint[1] = getMidpoint(c, a, midPoint[0]);
		emitTriangle(a, midPoint[0], midPoint[1]);
		emitTriangle(midPoint[0], c, midPoint[1]);
		emitTriangle(c, a, midPoint[1]);
	}
}

void main()
{
	FragmentColour = vec3(0.0, 0.0, 1.0);



	tessellate(
		gl_in[0].gl_Position,
		gl_in[1].gl_Position,
		gl_in[2].gl_Position,
		1
	);
}