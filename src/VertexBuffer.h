#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class VertexBuffer {
public:
	VertexBuffer(const int *vectorCounts);
	~VertexBuffer();

	void Begin();
	void End();
	void Draw(GLenum type);

	void AddValue(int index, float value);
	void AddValue(int index, const glm::vec2 &value);
	void AddValue(int index, const glm::vec3 &value);
	void AddValue(int index, const glm::vec4 &value);

private:
	int numBuffers;
	int *vectorCounts;
	
	GLuint glId;
	GLuint *glBufferIds;
	std::vector<float> *data;
	
	int numPoints;
	bool noRender;

	void GenerateBuffers(const int *vectorCounts);
	void SetBufferData(int index, std::vector<float> *data);
};

} }