#include "VertexBuffer.h"

using namespace IntelOrca::PopSS;

VertexBuffer::VertexBuffer(const int *vectorCounts)
{
	this->GenerateBuffers(vectorCounts);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteVertexArrays(1, &this->glId);
	glDeleteBuffers(this->numBuffers, this->glBufferIds);

	delete[] this->vectorCounts;
	delete[] this->glBufferIds;
	delete[] this->data;
}

void VertexBuffer::GenerateBuffers(const int *vectorCounts)
{
	this->numBuffers = 0;
	for (const int *vectorCount = vectorCounts; *vectorCount != 0; vectorCount++)
		this->numBuffers++;

	this->vectorCounts = new int[this->numBuffers];
	memcpy(this->vectorCounts, vectorCounts, this->numBuffers * sizeof(int));

	// Generate the vertex array
	glGenVertexArrays(1, &this->glId);
	glBindVertexArray(this->glId);

	// Generate the buffers
	this->glBufferIds = new GLuint[this->numBuffers];
	this->data = new std::vector<float>[this->numBuffers];
	glGenBuffers(this->numBuffers, this->glBufferIds);
	for (int i = 0; i < this->numBuffers; i++) {
		glEnableVertexAttribArray(i);
		glBindBuffer(GL_ARRAY_BUFFER, this->glBufferIds[i]);
		glVertexAttribPointer(i, this->vectorCounts[i], GL_FLOAT, GL_FALSE, 0, NULL);
	}
}

void VertexBuffer::SetBufferData(int index, std::vector<float> *data)
{
	glBindBuffer(GL_ARRAY_BUFFER, this->glBufferIds[index]);
	glBufferData(GL_ARRAY_BUFFER, data->size() * sizeof(float), data->data(), GL_DYNAMIC_DRAW);
}

void VertexBuffer::Begin()
{
	for (int i = 0; i < this->numBuffers; i++)
		this->data[i].clear();
}

void VertexBuffer::End()
{
	// Check buffers are same size
	this->numPoints = 0;
	for (int i = 0; i < this->numBuffers; i++) {
		int numPoints = data[i].size() / this->vectorCounts[i];
		if (i == 0)
			this->numPoints = numPoints;
		else if (this->numPoints != numPoints)
			fprintf(stderr, "Buffer sizes are inconsistant\n");
	}
	if (this->numPoints == 0) {
		this->noRender = true;
		return;
		// fprintf(stderr, "Buffer size of 0\n");
	}

	// Set buffers
	for (int i = 0; i < this->numBuffers; i++)
		this->SetBufferData(i, &this->data[i]);
	this->noRender = false;
}

void VertexBuffer::Draw(GLenum type)
{
	glBindVertexArray(this->glId);
	glDrawArrays(type, 0, this->numPoints);
}

void VertexBuffer::AddValue(int index, float value)
{
	this->data[index].push_back(value);
}

void VertexBuffer::AddValue(int index, const glm::vec2 &value)
{
	this->data[index].push_back(value.x);
	this->data[index].push_back(value.y);
}

void VertexBuffer::AddValue(int index, const glm::vec3 &value)
{
	this->data[index].push_back(value.x);
	this->data[index].push_back(value.y);
	this->data[index].push_back(value.z);
}

void VertexBuffer::AddValue(int index, const glm::vec4 &value)
{
	this->data[index].push_back(value.x);
	this->data[index].push_back(value.y);
	this->data[index].push_back(value.z);
	this->data[index].push_back(value.w);
}