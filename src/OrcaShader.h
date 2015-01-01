#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

struct VertexAttribPointerInfo {
	const char *name;
	GLenum type;
	unsigned char size;
	unsigned short offset;
};

class OrcaShader {
public:
	GLuint program;

	void Use() const;
	GLint GetUniformLocation(const char *name) const;
	GLint GetAttributeLocation(const char *name) const;

	void SetVertexAttribPointer(int stride, const VertexAttribPointerInfo *vertexInfo);

	static OrcaShader *FromPath(const char *vertexPath, const char *fragmentPath);
	static GLuint LoadShader(GLenum type, const char *path);

private:
	GLuint vertexShader;
	GLuint fragmentShader;

	static char *LoadShader(const char *path);
	static bool ParsePreprocessor(const char *sourcePath, std::vector<char> *sourceCode, const char *directive);
};

} }