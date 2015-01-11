#include "Mesh.h"

using namespace IntelOrca::PopSS;

Mesh::Mesh()
{
	this->numVertices = 0;
	this->vertices = NULL;
	this->numTextureCoordinates = 0;
	this->textureCoordinates = NULL;
	this->numNormals = 0;
	this->normals = NULL;
	this->numFaces = 0;
	this->faces = NULL;
	this->name = NULL;
}

Mesh::~Mesh()
{
	SafeDeleteArray(this->vertices);
	SafeDeleteArray(this->textureCoordinates);
	SafeDeleteArray(this->normals);
	SafeDeleteArray(this->faces);
	SafeDelete(this->name);
}

bool Mesh::SaveToObjectFile(const char *path)
{
	FILE *file;
	unsigned char buffer[256];

	file = fopen(path, "wb");
	if (file == NULL)
		return false;

	buffer[0] = 'P';
	buffer[1] = 'O';
	buffer[2] = 'B';
	buffer[3] = 'J';
	buffer[4] = 0;
	fwrite(buffer, 5, 1, file);

	if (this->name == NULL) {
		fputc(0, file);
	} else {
		fwrite(this->name, strlen(this->name) + 1, 1, file);
	}

	*((int*)buffer) = this->numVertices;
	fwrite(buffer, 4, 1, file);
	if (this->numVertices > 0)
		fwrite(this->vertices, this->numVertices * sizeof(glm::vec3), 1, file);

	*((int*)buffer) = this->numTextureCoordinates;
	fwrite(buffer, 4, 1, file);
	if (this->numTextureCoordinates > 0)
		fwrite(this->textureCoordinates, this->numTextureCoordinates * sizeof(glm::vec2), 1, file);

	*((int*)buffer) = this->numNormals;
	fwrite(buffer, 4, 1, file);
	if (this->numNormals > 0)
		fwrite(this->normals, this->numNormals * sizeof(glm::vec3), 1, file);

	*((int*)buffer) = this->numFaces;
	fwrite(buffer, 4, 1, file);
	if (this->numFaces > 0)
		fwrite(this->faces, this->numFaces * sizeof(Mesh::Face), 1, file);

	fclose(file);
	return true;
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

	std::vector<int> parsedIntsV, parsedIntsT, parsedIntsN;

	std::vector<float> parsedFloats;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	std::vector<Face> faces;
	for (char *line : lines) {
		parsedIntsV.clear();
		parsedIntsT.clear();
		parsedIntsN.clear();
		parsedFloats.clear();

		int type;
		int argIndex = 0;
		char *pch = strtok(line, " \t");
		while (pch != NULL) {
			if (argIndex == 0) {
				if (strcmp(pch, "v") == 0) type = 'v';
				else if (strcmp(pch, "f") == 0) type = 'f';
				else if (strcmp(pch, "vt") == 0) type = 't';
				else if (strcmp(pch, "vn") == 0) type = 'n';
				else {
					type = -1;
					break;
				}
			} else {
				if (type == 'v' || type == 't' || type == 'n') {
					parsedFloats.push_back(atof(pch));
				} else if (type == 'f') {
					char *sep = strchr(pch, '/');
					if (sep == NULL) {
						parsedIntsT.push_back(atoi(pch) - 1);
						parsedIntsV.push_back(-1);
						parsedIntsN.push_back(-1);
					} else {
						char a[8];
						memcpy(a, pch, (int)(sep - pch));
						a[sep - pch] = 0;
						parsedIntsT.push_back(atoi(a) - 1);

						char *sep2 = strchr(sep + 1, '/');
						if (sep2 == NULL) {
							strcpy(a, sep + 1);
							parsedIntsV.push_back(atoi(a) - 1);
							parsedIntsN.push_back(-1);
						} else {
							if (sep2 != sep + 1) {
								memcpy(a, sep + 1, (int)(sep2 - sep - 1));
								a[sep2 - sep - 1] = 0;
								parsedIntsV.push_back(atoi(a) - 1);	
							} else {
								parsedIntsV.push_back(-1);
							}

							strcpy(a, sep2 + 1);
							parsedIntsN.push_back(atoi(a) - 1);
						}
					}
				}
			}

			pch = strtok(NULL, " \t");
			argIndex++;
		}

		if (type == 'v') {
			vertices.push_back(glm::vec3(parsedFloats[0], parsedFloats[1], parsedFloats[2]));
		} else if (type == 't') {
			texCoords.push_back({ parsedFloats[0], parsedFloats[1] });
		} else if (type == 'n') {
			normals.push_back({ parsedFloats[0], parsedFloats[1], parsedFloats[2] });
		} else if (type == 'f') {
			faces.push_back({ { { parsedIntsT[0], parsedIntsV[0], parsedIntsN[0] }, { parsedIntsT[1], parsedIntsV[1], parsedIntsN[1] }, { parsedIntsT[2], parsedIntsV[2], parsedIntsN[2] } } });
			if (parsedIntsT.size() >= 4)
				faces.push_back({ { { parsedIntsT[0], parsedIntsV[0], parsedIntsN[0] }, { parsedIntsT[2], parsedIntsV[2], parsedIntsN[2] }, { parsedIntsT[3], parsedIntsV[3], parsedIntsN[3] } } });
		}
	}

	for (char *line : lines)
		free(line);

	Mesh *mesh = new Mesh();

	mesh->numVertices = vertices.size();
	mesh->vertices = new glm::vec3[mesh->numVertices];
	for (int i = 0; i < mesh->numVertices; i++)
		mesh->vertices[i] = vertices[i];

	mesh->numTextureCoordinates = texCoords.size();
	mesh->textureCoordinates = new glm::vec2[mesh->numTextureCoordinates];
	for (int i = 0; i < mesh->numTextureCoordinates; i++)
		mesh->textureCoordinates[i] = texCoords[i];

	mesh->numNormals = normals.size();
	mesh->normals = new glm::vec3[mesh->numNormals];
	for (int i = 0; i < mesh->numNormals; i++)
		mesh->normals[i] = normals[i];

	mesh->numFaces = faces.size();
	mesh->faces = new Face[mesh->numFaces];
	for (int i = 0; i < mesh->numFaces; i++)
		mesh->faces[i] = faces[i];

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

	if (fread(&mesh->numVertices, sizeof(int), 1, file) != 1) goto fail;
	if (mesh->numVertices > 0) {
		mesh->vertices = new glm::vec3[mesh->numVertices];
		if (fread(mesh->vertices, mesh->numVertices * sizeof(glm::vec3), 1, file) != 1) goto fail;
	}

	if (fread(&mesh->numTextureCoordinates, sizeof(int), 1, file) != 1) goto fail;
	if (mesh->numTextureCoordinates > 0) {
		mesh->textureCoordinates = new glm::vec2[mesh->numTextureCoordinates];
		if (fread(mesh->textureCoordinates, mesh->numTextureCoordinates * sizeof(glm::vec2), 1, file) != 1) goto fail;
	}

	if (fread(&mesh->numNormals, sizeof(int), 1, file) != 1) goto fail;
	if (mesh->numNormals > 0) {
		mesh->normals = new glm::vec3[mesh->numNormals];
		if (fread(mesh->normals, mesh->numNormals * sizeof(glm::vec3), 1, file) != 1) goto fail;
	}

	if (fread(&mesh->numFaces, sizeof(int), 1, file) != 1) goto fail;
	if (mesh->numFaces > 0) {
		mesh->faces = new Mesh::Face[mesh->numFaces];
		if (fread(mesh->faces, mesh->numFaces * sizeof(Mesh::Face), 1, file) != 1) goto fail;
	}

	fclose(file);
	return mesh;

fail:
	fclose(file);
	fprintf(stderr, "Error reading %s\n", path);
	
	delete mesh;
	return NULL;
}