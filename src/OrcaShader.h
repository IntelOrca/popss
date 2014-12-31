#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class OrcaShader {
public:
	GLuint program;

	void Use() const;

	static OrcaShader *FromPath(const char *vertexPath, const char *fragmentPath);
	static GLuint LoadShader(GLenum type, const char *path);

private:
	GLuint vertexShader;
	GLuint fragmentShader;

	static char *LoadShader(const char *path);
	static bool ParsePreprocessor(const char *sourcePath, std::vector<char> *sourceCode, const char *directive);
};

} }