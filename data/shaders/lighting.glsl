// Lighting include shader

struct LightSource {
	vec3 Position;
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

vec3 DiffuseShading(vec3 ld, vec3 kd, vec3 lightPosition, vec3 position, vec3 normal)
{
	vec3 lightSource = normalize(lightPosition - position);
	return ld * kd * max(dot(lightSource, normal), 0.1);
}

vec3 SpecularShading(
	vec3 ls, vec3 ks, float shininess, 
	vec3 lightPosition, vec3 position, vec3 normal)
{
	vec3 lightSource = normalize(lightPosition - position);
	vec3 cameraDestination = normalize(vec3(0, 1024, -1024) - position);
	vec3 reflection = reflect(-lightSource, normal);
	float sDotN = max(dot(lightSource, normal), 0.0);

	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = ls * ks * pow(max(dot(reflection, cameraDestination), 0.0), shininess);
	return spec;
}

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