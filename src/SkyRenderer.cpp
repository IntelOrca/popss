#include "Camera.h"
#include "SkyRenderer.h"

using namespace IntelOrca::PopSS;

const VertexAttribPointerInfo SkyShaderVertexInfo[] = {
	{ "VertexPosition",			GL_FLOAT,			4,	offsetof(SkyVertex, position)		},
	{ NULL }
};

SkyRenderer::SkyRenderer()
{
	this->skyShader = NULL;
	this->skyVertexBuffer = NULL;
}

SkyRenderer::~SkyRenderer()
{
	if (this->skyShader != NULL) delete this->skyShader;
	if (this->skyVertexBuffer != NULL) delete this->skyVertexBuffer;
}

void SkyRenderer::Initialise()
{
	this->skyShader = OrcaShader::FromPath("fog.vert", "fog.frag");
	this->skyVertexBuffer = new SimpleVertexBuffer<SkyVertex>(this->skyShader, SkyShaderVertexInfo);

	// Set vertices
	SimpleVertexBuffer<SkyVertex> *vb = this->skyVertexBuffer;
	glm::vec4 points[4] = {
		{ -1.0, +1.0, 1.0, 1.0 },
		{ -1.0, -1.0, 1.0, 1.0 },
		{ +1.0, -1.0, 1.0, 1.0 },
		{ +1.0, +1.0, 1.0, 1.0 }
	};
	vb->Clear();
	vb->Add({ points[0] });
	vb->Add({ points[1] });
	vb->Add({ points[2] });
	vb->Add({ points[0] });
	vb->Add({ points[2] });
	vb->Add({ points[3] });
	vb->Update();
}

void SkyRenderer::Render(const Camera *camera)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);

	this->skyShader->Use();
	this->skyVertexBuffer->Draw(GL_TRIANGLES);
}