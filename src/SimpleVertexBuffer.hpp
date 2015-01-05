#pragma once

#include "OrcaShader.h"
#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

template<typename T>
class SimpleVertexBuffer {
public:
	GLenum usage;

	SimpleVertexBuffer() {
		this->usage = GL_STATIC_DRAW;

		glGenBuffers(1, &this->vbo);
		glGenVertexArrays(1, &this->vao);
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBindVertexArray(this->vao);
	}

	SimpleVertexBuffer(OrcaShader *shader, const VertexAttribPointerInfo *vertexInfo) : SimpleVertexBuffer() {
		this->Initialise(shader, vertexInfo);
	}

	~SimpleVertexBuffer() {
		glDeleteBuffers(1, &this->vbo);
		glDeleteVertexArrays(1, &this->vao);
	}

	void Initialise(OrcaShader *shader, const VertexAttribPointerInfo *vertexInfo) {
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBindVertexArray(this->vao);
		shader->SetVertexAttribPointer(sizeof(T), vertexInfo);
	}

	void Clear() {
		this->vertices.clear();
	}

	void Add(const T &vertex) {
		this->vertices.push_back(vertex);
	}

	void Update() {
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(T), this->vertices.data(), this->usage);
	}

	void Draw(GLenum mode) {
		glBindVertexArray(this->vao);
		this->Draw(mode, 0, this->vertices.size());
	}

	void Draw(GLenum mode, int first, int count) {
		glBindVertexArray(this->vao);
		glDrawArrays(mode, first, count);
	}

	int& operator[] (int index) {
		return this->vertices[index];
	}

private:
	GLuint vao;
	GLuint vbo;
	std::vector<T> vertices;
};

} }