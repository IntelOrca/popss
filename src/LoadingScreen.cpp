#include "LoadingScreen.h"
#include "Mesh.h"
#include "OrcaShader.h"
#include "Util/MathExtensions.hpp"

using namespace IntelOrca::PopSS;

const VertexAttribPointerInfo HandShaderVertexInfo[] = {
	{ "aPosition",	GL_FLOAT,	3,	offsetof(HandVertex, position)	},
	{ "aNormal",	GL_FLOAT,	3,	offsetof(HandVertex, normal)	},
	{ NULL }
};

LoadingScreen::LoadingScreen()
{
	this->handShader = NULL;
	this->handVertexBuffer = NULL;
	this->angle = 0.0f;
}

LoadingScreen::~LoadingScreen()
{
	SafeDelete(this->handShader);
	SafeDelete(this->handVertexBuffer);
}

void LoadingScreen::Initialise()
{
	this->handShader = OrcaShader::FromPath("hand.vert", "hand.frag");
	this->handVertexBuffer = new SimpleVertexBuffer<HandVertex>(this->handShader, HandShaderVertexInfo);

	Mesh* mesh = Mesh::FromObjectFile("data/objects/hand.object");
	for (int i = 0; i < mesh->numFaces; i++) {
		const Mesh::Face *face = &mesh->faces[i];
		glm::vec3 vertices[3];
		glm::vec2 texcoords[3];
		glm::vec3 normals[3];
		
		for (int j = 0; j < 3; j++) {
			vertices[j] = mesh->vertices[face->vertex[j].position];
			texcoords[j] = face->vertex[j].texture == -1 ? glm::vec2(0) : mesh->textureCoordinates[face->vertex[j].texture];
			normals[j] = face->vertex[j].normal == -1 ? glm::vec3(0) : mesh->normals[face->vertex[j].normal];
		}

		for (int j = 0; j < 3; j++) {
			HandVertex vertex;
			vertex.position = vertices[j];
			vertex.normal = normals[j];
			this->handVertexBuffer->Add(vertex);
		}
	}

	this->handVertexBuffer->Update();
	delete mesh;

	// this->projectionMatrix = glm::perspective(toradians(60.0f), 1920.0f / 1080.0f, 1.0f, 100.0f);
	this->projectionMatrix = glm::ortho(
		-8.0f, +8.0f,
		-4.5f, +4.5f,
		-6.0f, 100.0f
	);

	this->viewMatrix = glm::lookAt(
		glm::vec3(0.0f, 0.0f, -8.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
}

void LoadingScreen::Draw()
{
	this->modelMatrix = glm::rotate(glm::mat4(), toradians(90.0f), glm::vec3(1, 0, 0));
	this->modelMatrix = glm::rotate(this->modelMatrix, toradians(this->angle), glm::vec3(0, 0, 1));
	this->modelMatrix = glm::rotate(this->modelMatrix, toradians(-45.0f), glm::vec3(0, 1, 0));
	// this->modelMatrix = glm::scale(this->modelMatrix, glm::vec3(2.0f));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	this->handShader->Use();
	glUniformMatrix4fv(this->handShader->GetUniformLocation("uProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));
	glUniformMatrix4fv(this->handShader->GetUniformLocation("uViewMatrix"), 1, GL_FALSE, glm::value_ptr(this->viewMatrix));
	glUniformMatrix4fv(this->handShader->GetUniformLocation("uModelMatrix"), 1, GL_FALSE, glm::value_ptr(this->modelMatrix));

	this->handVertexBuffer->Draw(GL_TRIANGLES);
}

void LoadingScreen::Update()
{
	if (this->handShader == NULL)
		this->Initialise();

	this->angle = wrapangledeg(this->angle + 2);
}