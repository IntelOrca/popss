#include "OrcaShader.h"

using namespace IntelOrca::PopSS;

enum {
	PARSE_STATE_NORMAL,
	PARSE_STATE_LINE_COMMENT,
	PARSE_STATE_BLOCK_COMMENT,
	PARSE_STATE_PREPROCESSOR
};

int fpeekc(FILE *stream)
{
    int c = fgetc(stream);
    ungetc(c, stream);
    return c;
}

void OrcaShader::Use() const
{
	glUseProgram(this->program);
}

GLint OrcaShader::GetUniformLocation(const char *name) const
{
	return glGetUniformLocation(this->program, name);
}

GLint OrcaShader::GetAttributeLocation(const char *name) const
{
	return glGetAttribLocation(this->program, name);
}

void OrcaShader::SetVertexAttribPointer(int stride, const VertexAttribPointerInfo *vertexInfo)
{
	while (vertexInfo->name != NULL) {
		GLint attributeLocation = this->GetAttributeLocation(vertexInfo->name);

		glEnableVertexAttribArray(attributeLocation);

		switch (vertexInfo->type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_INT:
		case GL_UNSIGNED_INT:
			glVertexAttribIPointer(attributeLocation, vertexInfo->size, vertexInfo->type, stride, (void*)vertexInfo->offset);
			break;
		case GL_DOUBLE:
			glVertexAttribLPointer(attributeLocation, vertexInfo->size, vertexInfo->type, stride, (void*)vertexInfo->offset);
			break;
		default:
			glVertexAttribPointer(attributeLocation, vertexInfo->size, vertexInfo->type, GL_FALSE, stride, (void*)vertexInfo->offset);
			break;
		}

		vertexInfo++;
	}
}

OrcaShader *OrcaShader::FromPath(const char *vertexPath, const char *fragmentPath)
{
	OrcaShader *shader = new OrcaShader();

	shader->vertexShader = OrcaShader::LoadShader(GL_VERTEX_SHADER, vertexPath);
	shader->fragmentShader = OrcaShader::LoadShader(GL_FRAGMENT_SHADER, fragmentPath);

	if (shader->vertexShader == 0 || shader->fragmentShader == 0) {
		delete shader;
		return NULL;
	}

	shader->program = glCreateProgram();
	glAttachShader(shader->program, shader->vertexShader);
	glAttachShader(shader->program, shader->fragmentShader);
	glLinkProgram(shader->program);

	return shader;
}

GLuint OrcaShader::LoadShader(GLenum type, const char *path)
{
	GLint shader, status;

	char lessRelativePath[300] = { "data/shaders/" };
	strcat(lessRelativePath, path);

	char *sourceCode = LoadShader(lessRelativePath);
	if (sourceCode == NULL)
		return 0;

	shader = glCreateShader(type);
	if (shader == 0) {
		free(sourceCode);
		return 0;
	}

	glShaderSource(shader, 1, &sourceCode, NULL);
	glCompileShader(shader);

	free(sourceCode);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		glDeleteShader(shader);

		char buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, buffer);
		fprintf(stderr, buffer);
		return 0;
	}

	return shader;
}

char *OrcaShader::LoadShader(const char *path)
{
	std::vector<char> sourceCode, preprocessorDirective;
	int c, nextC, parseState;

	parseState = PARSE_STATE_NORMAL;

	FILE *file = fopen(path, "r");
	if (file == NULL)
		return NULL;

	// Skim UTF-8 byte order mark
	char bom[3];
	bool hasBOM = false;
	if (fread(bom, 3, 1, file) == 1)
		if (bom[0] == (char)0xEF && bom[1] == (char)0xBB && bom[2] == (char)0xBF)
			hasBOM = true;

	if (!hasBOM)
		fseek(file, 0, SEEK_SET);

	while ((c = fgetc(file)) != EOF) {
		nextC = fpeekc(file);

		switch (parseState) {
		case PARSE_STATE_NORMAL:
			if (c == '/' && nextC == '/') {
				fgetc(file);
				parseState = PARSE_STATE_LINE_COMMENT;
			} else if (c == '/' && nextC == '*') {
				fgetc(file);
				parseState = PARSE_STATE_BLOCK_COMMENT;
			} else if (c == '#') {
				parseState = PARSE_STATE_PREPROCESSOR;
				preprocessorDirective.clear();
			} else {
				sourceCode.push_back(c);
			}
			break;
		case PARSE_STATE_LINE_COMMENT:
			if (c == '\n' || c == '\r') {
				parseState = PARSE_STATE_NORMAL;
				sourceCode.push_back(c);
			}
			break;
		case PARSE_STATE_BLOCK_COMMENT:
			if (c == '*' && nextC == '/') {
				fgetc(file);
				parseState = PARSE_STATE_NORMAL;
			}
			break;
		case PARSE_STATE_PREPROCESSOR:
			if (c == '/' && nextC == '/') {
				fgetc(file);
				parseState = PARSE_STATE_LINE_COMMENT;
				preprocessorDirective.push_back(0);
				if (!ParsePreprocessor(path, &sourceCode, &preprocessorDirective[0]))
					return NULL;
			} else if (c == '/' && nextC == '*') {
				fgetc(file);
				parseState = PARSE_STATE_BLOCK_COMMENT;
				preprocessorDirective.push_back(0);
				if (!ParsePreprocessor(path, &sourceCode, &preprocessorDirective[0]))
					return NULL;
			} else if (c == '\n' || c == '\r') {
				parseState = PARSE_STATE_NORMAL;
				preprocessorDirective.push_back(0);
				if (!ParsePreprocessor(path, &sourceCode, &preprocessorDirective[0]))
					return NULL;
			} else {
				preprocessorDirective.push_back(c);
			}
			break;
		}
	}
	fclose(file);

	sourceCode.push_back(0);

	char *result = (char*)malloc(sourceCode.size());
	memcpy(result, &sourceCode[0], sourceCode.size());
	return result;
}

bool OrcaShader::ParsePreprocessor(const char *sourcePath, std::vector<char> *sourceCode, const char *directive)
{
	char filename[300];
	const char *ch, *end, *prefixPathEnd;
	
	end = directive;
	while (*end != ' ' && *end != '\t' && *end != 0)
		end++;

	int length = (int)(end - directive);
	if (length == 0)
		return false;

	// Include
	if (_strnicmp(directive, "include", length) == 0) {
		ch = end;

		// Skip whitespace
		do {
			ch++;
		} while (*ch == ' ' || *ch == '\t');

		// Expect quote
		if (*ch++ != '"')
			return false;

		// Get file name
		end = strchr(ch, '"');
		if (end == NULL)
			return false;

		length = min((int)(end - ch), (int)countof(filename));

		prefixPathEnd = strrchr(sourcePath, '/');
		int prefixPathLength = 0;
		if (prefixPathEnd != NULL) {
			prefixPathLength = (int)(prefixPathEnd - sourcePath);
			memcpy(filename, sourcePath, prefixPathLength);
			filename[prefixPathLength++] = '/';
		}

		int i;
		for (i = prefixPathLength; i < prefixPathLength + length; i++)
			filename[i] = *ch++;

		filename[i] = 0;

		// Parse filename
		char *includeShaderSource = LoadShader(filename);
		if (includeShaderSource == NULL)
			return NULL;

		sourceCode->insert(sourceCode->end(), includeShaderSource, includeShaderSource + strlen(includeShaderSource));
		free(includeShaderSource);
		return true;
	} else {
		sourceCode->push_back('#');
		sourceCode->insert(sourceCode->end(), directive, directive + strlen(directive));
		return true;
	}
}