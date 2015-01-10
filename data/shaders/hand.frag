#version 330

smooth in vec3 fPosition;
smooth in vec3 fNormal;

out vec4 OutputColour;

vec3 PhongShading(
	vec3 la, vec3 ld, vec3 ls,
	vec3 ka, vec3 kd, vec3 ks, float shininess,
	vec3 lightPosition, vec3 position, vec3 normal)
{
	vec3 lightSource = normalize(lightPosition - position);
	vec3 cameraDestination = normalize(vec3(0, 1024, -1024) - position);
	vec3 reflection = reflect(-lightSource, normal);
	float sDotN = max(dot(lightSource, normal), 0.0);

	vec3 ambient = la * ka;
	vec3 diffuse = ld * kd * sDotN;
	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = ls * ks * pow(max(dot(reflection, cameraDestination), 0.0), shininess);
	return ambient + diffuse + spec;
}

void main()
{
	vec3 lightPos = vec3(0, 8, -8);
	vec4 colour = vec4(0, 0, 0, 1);

	colour.rgb = PhongShading(
		vec3(0.1), vec3(0.8), vec3(0.0),
		vec3(1.0), vec3(1.0), vec3(1.0), 0,
		lightPos, fPosition, fNormal
	);

	OutputColour = colour;
}