#include "Mesh.h"

using namespace IntelOrca::PopSS;

Mesh::Mesh()
{
	this->numVertices = 0;
	this->vertices = NULL;
	this->numTextureCoordinates = 0;
	this->textureCoordinates = NULL;
	this->numFaces = 0;
	this->faces = NULL;
}

Mesh::~Mesh()
{
	if (this->vertices != NULL) delete[] this->vertices;
	if (this->textureCoordinates != NULL) delete[] this->textureCoordinates;
	if (this->faces != NULL) delete[] this->faces;
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
			faces.push_back({ { { parsedInts[0], -1 }, { parsedInts[1], -1 }, { parsedInts[2], -1 } } });
			if (parsedInts.size() >= 4)
				faces.push_back({ { { parsedInts[0], -1 }, { parsedInts[2], -1 }, { parsedInts[3], -1 } } });
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

	unsigned char buffer[256];
	fopen("C:\\Users\\Ted\\Documents\\GitHub\\popss\\data\\objects\\vok.object", "wb");

	buffer[0] = 'P';
	buffer[1] = 'O';
	buffer[2] = 'B';
	buffer[3] = 'J';
	buffer[4] = 0;
	fwrite(buffer, 5, 1, file);

	strcpy((char*)buffer, "vault of knowledge");
	fwrite(buffer, strlen((char*)buffer) + 1, 1, file);

	*((int*)buffer) = mesh->numVertices;
	fwrite(buffer, 4, 1, file);
	if (mesh->numVertices > 0)
		fwrite(mesh->vertices, mesh->numVertices * sizeof(glm::vec3), 1, file);

	*((int*)buffer) = mesh->numTextureCoordinates;
	fwrite(buffer, 4, 1, file);
	if (mesh->numTextureCoordinates > 0)
		fwrite(mesh->textureCoordinates, mesh->numTextureCoordinates * sizeof(float), 1, file);

	*((int*)buffer) = mesh->numFaces;
	fwrite(buffer, 4, 1, file);
	if (mesh->numFaces > 0)
		fwrite(mesh->faces, mesh->numFaces * sizeof(Mesh::Face), 1, file);

	fclose(file);

	return mesh;
}

Mesh *Mesh::FromObjectFile(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		fprintf(stderr, "Unable to open %s\n", path);
		return NULL;
	}

	unsigned char magicNumber[4];
	fread(magicNumber, sizeof(magicNumber), 1, file);
	if (magicNumber[0] != 'P' || magicNumber[1] != 'O' || magicNumber[2] != 'B' || magicNumber[3] != 'J')
		return NULL;

	unsigned char version;
	fread(&version, sizeof(version), 1, file);
	if (version > 0)
		return NULL;

	std::vector<char> name;
	char c;
	do {
		c = getc(file);
		name.push_back(c);
	} while (c != 0);

	Mesh *mesh = new Mesh();

	fread(&mesh->numVertices, sizeof(int), 1, file);
	if (mesh->numVertices > 0) {
		mesh->vertices = new glm::vec3[mesh->numVertices];
		fread(mesh->vertices, mesh->numVertices * sizeof(glm::vec3), 1, file);
	}

	fread(&mesh->numTextureCoordinates, sizeof(int), 1, file);
	if (mesh->numTextureCoordinates > 0) {
		mesh->textureCoordinates = new glm::vec2[mesh->numTextureCoordinates];
		fread(mesh->textureCoordinates, mesh->numTextureCoordinates * sizeof(glm::vec2), 1, file);
	}

	fread(&mesh->numFaces, sizeof(int), 1, file);
	if (mesh->numFaces > 0) {
		mesh->faces = new Mesh::Face[mesh->numFaces];
		fread(mesh->faces, mesh->numFaces * sizeof(Mesh::Face), 1, file);
	}

	fclose(file);

	return mesh;
}