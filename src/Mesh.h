#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class Mesh {
public:

	#pragma pack(push, 1)

	struct FaceVertex {
		int position;
		int texture;
	};
	struct Face {
		FaceVertex vertex[3];
	};

	#pragma pack(pop)

	int numVertices;
	glm::vec3 *vertices;
	int numTextureCoordinates;
	glm::vec2 *textureCoordinates;
	int numFaces;
	Face *faces;

	const char *name;

	Mesh();
	~Mesh();

	bool SaveToObjectFile(const char *path);

	static Mesh *FromObjFile(const char *path);
	static Mesh *FromObjectFile(const char *path);

private:

};

} }