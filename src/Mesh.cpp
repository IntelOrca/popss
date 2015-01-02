#include "Mesh.h"

using namespace IntelOrca::PopSS;

Mesh::~Mesh()
{
	delete[] this->vertices;
	delete[] this->faces;
}

Mesh *Mesh::FromObjFile(const char *path)
{
	FILE *file = fopen(path, "r");
	if (file == NULL) {
		fprintf(stderr, "Unable to open %s\n", path);
		return NULL;
	}

	int c;

	std::vector<char*> lines;
	std::vector<char> lineBuffer;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\r' || c == '\n') {
			if (lineBuffer.size() > 0) {
				lineBuffer.push_back(0);
				char *newLine = (char*)malloc(lineBuffer.size());
				memcpy(newLine, lineBuffer.data(), lineBuffer.size());
				lines.push_back(newLine);
				lineBuffer.clear();
			}
		} else {
			lineBuffer.push_back(c);
		}
	}
	fclose(file);

	std::vector<int> parsedInts;
	std::vector<float> parsedFloats;
	std::vector<glm::vec3> vertices;
	std::vector<Face> faces;
	for (char *line : lines) {
		parsedInts.clear();
		parsedFloats.clear();

		int type;
		int argIndex = 0;
		char *pch = strtok(line, " \t");
		while (pch != NULL) {
			if (argIndex == 0) {
				if (strcmp(pch, "v") == 0) type = 'v';
				else if (strcmp(pch, "f") == 0) type = 'f';
				else {
					type = -1;
					break;
				}
			} else {
				if (type == 'v') {
					parsedFloats.push_back(atof(pch));
				} else if (type == 'f') {
					parsedInts.push_back(atoi(pch) - 1);
				}
			}

			pch = strtok(NULL, " \t");
			argIndex++;
		}

		if (type == 'v') {
			vertices.push_back(glm::vec3(parsedFloats[0], parsedFloats[1], parsedFloats[2]));
		} else if (type == 'f') {
			faces.push_back({ { parsedInts[0], parsedInts[1], parsedInts[2] } } );
			if (parsedInts.size() >= 4)
				faces.push_back({ { parsedInts[0], parsedInts[2], parsedInts[3] } } );
		}
	}

	for (char *line : lines)
		free(line);

	Mesh *mesh = new Mesh();

	mesh->numVertices = vertices.size();
	mesh->vertices = new glm::vec3[mesh->numVertices];
	for (int i = 0; i < mesh->numVertices; i++)
		mesh->vertices[i] = vertices[i];

	mesh->numFaces = faces.size();
	mesh->faces = new Face[mesh->numFaces];
	for (int i = 0; i < mesh->numFaces; i++)
		mesh->faces[i] = faces[i];

	return mesh;
}