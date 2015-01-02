#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class Mesh {
public:
	struct Face {
		int vertex[3];
	};

	int numVertices;
	glm::vec3 *vertices;
	int numFaces;
	Face *faces;

	~Mesh();

	static Mesh *FromObjFile(const char *path);

private:

};

} }