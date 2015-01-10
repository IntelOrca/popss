#version 330

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

in vec3 aPosition;
in vec3 aNormal;

smooth out vec3 fPosition;
smooth out vec3 fNormal;

void main()
{
	fPosition = (uModelMatrix * vec4(aPosition, 1.0)).xyz;
	fNormal = normalize((uModelMatrix * vec4(aNormal, 1.0)).xyz);

	gl_Position = uProjectionMatrix * uViewMatrix * vec4(fPosition, 1.0);
}