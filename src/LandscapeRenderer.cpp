#include "Camera.h"
#include "LandscapeRenderer.h"
#include "LightSource.h"
#include "OrcaShader.h"
#include "TerrainStyle.h"
#include "Util/MathExtensions.hpp"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>
#include <lodepng/lodepng.h>

using namespace IntelOrca::PopSS;

#define LAND_BLOCK_VERTICES_PER_CELL		2
#define LAND_BLOCK_FACES_PER_CELL			4
#define LAND_BLOCK_INDICES_PER_CELL			(LAND_BLOCK_FACES_PER_CELL * 3)
#define LAND_BLOCK_SIZE						8
#define LAND_BLOCK_SIZE_SQUARED				(LAND_BLOCK_SIZE * LAND_BLOCK_SIZE)
#define LAND_BLOCK_STRIDE					((LAND_BLOCK_SIZE + 1) * LAND_BLOCK_VERTICES_PER_CELL)
#define LAND_BLOCK_DATA_SIZE				(((LAND_BLOCK_SIZE + 1) * (LAND_BLOCK_SIZE + 1)) * LAND_BLOCK_VERTICES_PER_CELL)
#define LAND_BLOCK_INDEX_DATA_SIZE			(LAND_BLOCK_SIZE_SQUARED * LAND_BLOCK_INDICES_PER_CELL)

#define WATER_BLOCK_VERTICES_PER_CELL		1
#define WATER_BLOCK_FACES_PER_CELL			2
#define WATER_BLOCK_INDICES_PER_CELL		(WATER_BLOCK_FACES_PER_CELL * 3)
#define WATER_BLOCK_SIZE					8
#define WATER_BLOCK_SIZE_SQUARED			(WATER_BLOCK_SIZE * WATER_BLOCK_SIZE)
#define WATER_BLOCK_STRIDE					((WATER_BLOCK_SIZE + 1) * WATER_BLOCK_VERTICES_PER_CELL)
#define WATER_BLOCK_DATA_SIZE				(((WATER_BLOCK_SIZE + 1) * (WATER_BLOCK_SIZE + 1)) * WATER_BLOCK_VERTICES_PER_CELL)
#define WATER_BLOCK_INDEX_DATA_SIZE			(WATER_BLOCK_SIZE_SQUARED * WATER_BLOCK_INDICES_PER_CELL)


// const float LandscapeRenderer::SphereRatio = 0.0;
const float LandscapeRenderer::SphereRatio = 0.00002f;
const float LandscapeRenderer::TextureMapSize = 1.0f / 2.0f;

const VertexAttribPointerInfo LandShaderVertexInfo[] = {
	{ "VertexPosition",			GL_FLOAT,			3,	offsetof(LandVertex, position)		},
	{ "VertexNormal",			GL_FLOAT,			3,	offsetof(LandVertex, normal)		},
	{ "VertexTextureCoords",	GL_FLOAT,			2,	offsetof(LandVertex, texcoords)		},
	{ "VertexTexture",			GL_UNSIGNED_BYTE,	1,	offsetof(LandVertex, texture)		},
	{ "VertexMaterial",			GL_FLOAT,			4,	offsetof(LandVertex, material)		},
	{ NULL }
};

const VertexAttribPointerInfo WaterShaderVertexInfo[] = {
	{ "VertexPosition",			GL_FLOAT,			3,	offsetof(WaterVertex, position)		},
	{ NULL }
};

bool LoadTexture(GLuint texture, const char *path)
{
	GLubyte* bits;
	unsigned int error, width, height;

	error = lodepng_decode_file(&bits, &width, &height, path, LCT_RGBA, 8);
	if (error != 0) {
		fprintf(stderr, "Unable to read %s, %s.", path, lodepng_error_text(error));
		return false;
	}
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);

	glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

LandscapeRenderer::LandscapeRenderer()
{
	this->debugRenderType = DEBUG_LANDSCAPE_RENDER_TYPE_NONE;
	this->lastDebugRenderType = this->debugRenderType;

	this->landViewSize = 128;
	this->oceanViewSize = 52;

	this->time = 0;

	this->landShader = NULL;
	this->waterShader = NULL;
}

LandscapeRenderer::~LandscapeRenderer()
{
	SafeDelete(this->landShader);
	SafeDelete(this->landVertices);
	SafeDelete(this->landVertexIndices);

	SafeDelete(this->waterShader);
	SafeDelete(this->waterVertices);
	SafeDelete(this->waterVertexIndices);
}

void LandscapeRenderer::Initialise()
{
	this->InitialiseLandBlocks();
	this->InitialiseLandShader();

	this->InitialiseWaterBlocks();
	this->InitialiseWaterShader();

	this->UpdateLandAllSubBlocks();
	this->UpdateWaterAllSubBlocks();

	memset(this->terrainTextures, 0, sizeof(this->terrainTextures));
	glGenTextures(5, this->terrainTextures);
	
	LoadTexture(this->terrainTextures[0], "data/textures/sand.png");
	LoadTexture(this->terrainTextures[1], "data/textures/grass.png");
	LoadTexture(this->terrainTextures[2], "data/textures/snow.png");
	LoadTexture(this->terrainTextures[3], "data/textures/cliff.png");
	LoadTexture(this->terrainTextures[4], "data/textures/dirt.png");

	GenerateShadowTexture();
}

void LandscapeRenderer::Render(const Camera *camera)
{
	// Get the camera view matrix
	this->projectionMatrix = camera->Get3dProjectionMatrix();
	this->modelViewMatrix = camera->Get3dViewMatrix();

	glClear(GL_DEPTH_BUFFER_BIT);

	// glBlendFunc(GL_BLEND_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	switch (this->debugRenderType) {
	case DEBUG_LANDSCAPE_RENDER_TYPE_NONE:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case DEBUG_LANDSCAPE_RENDER_TYPE_WIREFRAME:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case DEBUG_LANDSCAPE_RENDER_TYPE_POINTS:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	this->UpdateDirtyBlocks();

	this->RenderLand(camera);
	this->RenderWater(camera);

	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	this->time += 1.0 / 60;
	this->lastDebugRenderType = this->debugRenderType;
}

void LandscapeRenderer::SetLightSources(const Camera *camera, OrcaShader *shader)
{
	LightSource alight, *light = &alight;

	light->position = glm::vec3(0.0f, 2048.0f, 0.0f) + glm::vec3(camera->target.x, 0, camera->target.z);
	light->ambient = glm::vec4(0.05f);
	light->diffuse = glm::vec4(0.4f);
	light->specular = glm::vec4(0.0f);

	glUniform1i(glGetUniformLocation(shader->program, "InputLightSourcesCount"), 2);
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[0].Specular"), 1, glm::value_ptr(light->specular));

	light->position = glm::vec3(-1.0f, 0.1f, 1.0f) * World::SkyDomeRadius + glm::vec3(camera->target.x, 0, camera->target.z);
	light->ambient = glm::vec4(0.0f);
	light->diffuse = glm::vec4(0.5f, 0.5f, 0.0f, 0.0f);
	light->specular = glm::vec4(0.25f, 0.25f, 0.0f, 0.0f);

	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(shader->program, "InputLightSources[1].Specular"), 1, glm::value_ptr(light->specular));
}

void LandscapeRenderer::GenerateShadowTexture()
{
	const World *world = this->world;
	const int worldSize = world->size * World::TileSize;
	const int resolution = 256;
	const int lookBack = 6;
	const int lookBackStep = 64;
	const glm::vec2 direction = glm::normalize(glm::vec2(0.5, 0.5));

	glGenTextures(1, &this->shadowTexture);
	glBindTexture(GL_TEXTURE_2D, this->shadowTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	unsigned int bits[resolution * resolution];
	for (int texZ = 0; texZ < resolution; texZ++) {
		int z = (texZ * worldSize) / resolution;
		for (int texX = 0; texX < resolution; texX++) {
			int x = (texX * worldSize) / resolution;

			bits[texX + texZ * resolution] = 0;

			unsigned int height = world->GetHeight(x, z);
			unsigned int occlusionHeight = 0;
			unsigned int heightCheck = height + (lookBackStep * 2) * lookBack;
			for (int i = lookBack * World::TileSize; i > 0; i -= lookBackStep) {
				glm::vec2 lbp = glm::vec2(x, z) - (direction * (float)i);

				occlusionHeight = world->GetHeight((int)lbp.x, (int)lbp.y);
				if (occlusionHeight > heightCheck)
					bits[texX + texZ * resolution] = 0xFFFFFFFF;

				// float shadowAmount = clamp(occlusionHeight / (float)heightCheck, 0.0f, 1.0f);
				// if (shadowAmount < 0.75f)
				// 	shadowAmount = 0.0f;
				// else
				// 	shadowAmount = (shadowAmount - 0.75f) / 0.25f;
				// 
				// unsigned char rgb = (unsigned char)(shadowAmount * 255);
				// if (rgb != 0) {
				// 	unsigned char *rgba = (unsigned char*)&bits[texX + texZ * resolution];
				// 	rgba[0] = rgba[1] = rgba[2] = rgb;
				// 	rgba[3] = 0xFF;
				// }

				heightCheck -= 64;
			}
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);
}

void LandscapeRenderer::SetDirtyTile(int x, int z)
{
	SetDirtyTile(x, z, x, z);
}

void LandscapeRenderer::SetDirtyTile(int x0, int z0, int x1, int z1)
{
	const World *world = this->world;
	int size = this->landBlocksPerRow;
	for (int z = z0; z <= z1; z++) {
		for (int x = x0; x <= x1; x++) {
			int blockX = world->TileWrap(x) / LAND_BLOCK_SIZE;
			int blockZ = world->TileWrap(z) / LAND_BLOCK_SIZE;

			this->dirtyLandBlocks[blockX + blockZ * size] = true;
		}
	}

	size = this->waterBlocksPerRow;
	for (int z = z0; z <= z1; z++) {
		for (int x = x0; x <= x1; x++) {
			int blockX = world->TileWrap(x) / WATER_BLOCK_SIZE;
			int blockZ = world->TileWrap(z) / WATER_BLOCK_SIZE;

			this->dirtyWaterBlocks[blockX + blockZ * size] = true;
		}
	}
}

void LandscapeRenderer::UpdateDirtyBlocks()
{
	int size = this->landBlocksPerRow;
	for (int z = 0; z < size; z++) {
		for (int x = 0; x < size; x++) {
			if (this->dirtyLandBlocks[x + z * size]) {
				this->UpdateLandSubBlock(x, z);
				this->dirtyLandBlocks[x + z * size] = false;
			}
		}
	}

	size = this->waterBlocksPerRow;
	for (int z = 0; z < size; z++) {
		for (int x = 0; x < size; x++) {
			if (this->dirtyWaterBlocks[x + z * size]) {
				this->UpdateWaterSubBlock(x, z);
				this->dirtyWaterBlocks[x + z * size] = false;
			}
		}
	}
}

#pragma region Land

void LandscapeRenderer::InitialiseLandShader()
{
	if (this->landShader == NULL) {
		glGenVertexArrays(1, &this->glLandVAO);
	} else {
		delete this->landShader;
	}

	this->landShader = OrcaShader::FromPath(
		"land.vert",
		this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_NONE ?
			"land.frag" : "land_wireframe.frag"
	);
	
	this->landShaderUniform.projectionMatrix = this->landShader->GetUniformLocation("ProjectionMatrix");
	this->landShaderUniform.viewMatrix = this->landShader->GetUniformLocation("ViewMatrix");
	this->landShaderUniform.modelMatrix = this->landShader->GetUniformLocation("ModelMatrix");
	this->landShaderUniform.sphereRatio = this->landShader->GetUniformLocation("InputSphereRatio");
	this->landShaderUniform.cameraTarget = this->landShader->GetUniformLocation("InputCameraTarget");
	this->landShaderUniform.highlightActive = this->landShader->GetUniformLocation("InputHighlightActive");
	this->landShaderUniform.highlight00 = this->landShader->GetUniformLocation("InputHighlight00");
	this->landShaderUniform.highlight11 = this->landShader->GetUniformLocation("InputHighlight11");

	glBindBuffer(GL_ARRAY_BUFFER, this->glLandVBO);
	glBindVertexArray(this->glLandVAO);
	this->landShader->SetVertexAttribPointer(sizeof(LandVertex), LandShaderVertexInfo);
}

void LandscapeRenderer::InitialiseLandBlocks()
{
	this->landBlocksPerRow = this->world->size / LAND_BLOCK_SIZE;
	this->numLandBlocks = this->landBlocksPerRow * this->landBlocksPerRow;
	this->totalLandVertexBufferSize = this->numLandBlocks * LAND_BLOCK_DATA_SIZE;
	this->landVertices = new LandVertex[this->totalLandVertexBufferSize];
	this->landVertexIndices = new std::vector<uint32>[this->numLandBlocks];

	this->dirtyLandBlocks = new bool[this->numLandBlocks];
	memset(this->dirtyLandBlocks, 0, this->numLandBlocks * sizeof(bool));

	glGenBuffers(1, &this->glLandIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glLandIndexVBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		this->numLandBlocks * LAND_BLOCK_INDEX_DATA_SIZE * sizeof(uint32),
		NULL,
		GL_STATIC_DRAW
	);

	glGenBuffers(1, &this->glLandVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->glLandVBO);
	glBufferData(GL_ARRAY_BUFFER, this->totalLandVertexBufferSize * sizeof(LandVertex), NULL, GL_STATIC_DRAW);
}

void LandscapeRenderer::UpdateLandAllSubBlocks()
{
	const int blocksPerRow = this->landBlocksPerRow;

	for (int z = 0; z < blocksPerRow; z++)
		for (int x = 0; x < blocksPerRow; x++)
			this->UpdateLandSubBlock(x, z);
}

void LandscapeRenderer::UpdateLandSubBlock(int blockX, int blockZ)
{
	int landX = blockX * LAND_BLOCK_SIZE;
	int landZ = blockZ * LAND_BLOCK_SIZE;

	// Vertex data
	int blockOffset = this->GetLandBlockBaseVertexIndex(blockX, blockZ);

	for (int z = 0; z < LAND_BLOCK_SIZE + 1; z++) {
		for (int x = 0; x < LAND_BLOCK_SIZE + 1; x++) {
			int index = blockOffset + this->GetLandBlockVertexIndex(x, z);
			GetLandVertex(landX + x, landZ + z, &this->landVertices[index], &this->landVertices[index + 1]);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->glLandVBO);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		blockOffset * sizeof(LandVertex),
		LAND_BLOCK_DATA_SIZE * sizeof(LandVertex),
		&this->landVertices[blockOffset]
	);

	// Vertex index data
	int indexBlockOffset = this->GetLandBlockBaseVertexIndexIndex(blockX, blockZ);

	std::vector<uint32> *blockIndices = &this->landVertexIndices[blockX + blockZ * this->landBlocksPerRow];
	blockIndices->clear();

	for (int z = 0; z < LAND_BLOCK_SIZE; z++) {
		for (int x = 0; x < LAND_BLOCK_SIZE; x++) {
			int index = blockOffset + this->GetLandBlockVertexIndex(x, z);
			UpdateLandSubBlockTileIndices(blockIndices, landX + x, landZ + z, index);
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glLandIndexVBO);
	glBufferSubData(
		GL_ELEMENT_ARRAY_BUFFER,
		indexBlockOffset * sizeof(uint32),
		blockIndices->size() * sizeof(uint32),
		blockIndices->data()
	);
}

void LandscapeRenderer::GetLandVertex(int landX, int landZ, LandVertex *topLeft, LandVertex *centre)
{
	const WorldTile *tile = this->world->GetTile(landX, landZ);
	const TerrainStyle *terrainStyle = &this->world->terrainStyles[tile->terrain];

	int x = landX * World::TileSize;
	int z = landZ * World::TileSize;
	int y = tile->height;

	glm::vec2 texCoords = glm::vec2(landX * TextureMapSize, landZ * TextureMapSize);
	glm::vec4 material = glm::vec4(
		terrainStyle->ambientReflectivity,
		terrainStyle->diffuseReflectivity,
		terrainStyle->specularReflectivity,
		terrainStyle->shininess
	);

	*topLeft = {
		glm::vec3(x, y, z),
		tile->lightNormal,
		texCoords,
		terrainStyle->textureIndex,
		material
	};

	int cx = x + (World::TileSize / 2);
	int cz = z + (World::TileSize / 2);
	int cy = tile->height;

	int heights[4] = {
		this->world->GetTile(landX + 0, landZ + 0)->height,
		this->world->GetTile(landX + 0, landZ + 1)->height,
		this->world->GetTile(landX + 1, landZ + 1)->height,
		this->world->GetTile(landX + 1, landZ + 0)->height
	};
	int totalLandPoints = 0;
	for (int i = 0; i < 4; i++)
		if (heights[i] != 0)
			totalLandPoints++;

	if ((heights[0] == 0 || heights[1] == 0 || heights[2] == 0 || heights[3] == 0) && totalLandPoints < 2)
		cy = 0;
	else
		cy = (heights[0] + heights[1] + heights[2] + heights[3]) / 4;

	*centre = {
		glm::vec3(cx, cy, cz),
		tile->lightNormal,
		texCoords + glm::vec2((TextureMapSize / 2), (TextureMapSize / 2)),
		terrainStyle->textureIndex,
		material
	};
}

void LandscapeRenderer::UpdateLandSubBlockTileIndices(std::vector<uint32> *blockIndices, int landX, int landZ, int baseIndex)
{
	int landX0 = landX + 0;
	int landX1 = landX + 1;
	int landZ0 = landZ + 0;
	int landZ1 = landZ + 1;

	// Get heights
	int tile00 = this->world->GetTile(landX0, landZ0)->height;
	int tile01 = this->world->GetTile(landX0, landZ1)->height;
	int tile10 = this->world->GetTile(landX1, landZ0)->height;
	int tile11 = this->world->GetTile(landX1, landZ1)->height;

	// Get vertex indicies
	int tile00index = baseIndex;
	int tile10index = baseIndex + LAND_BLOCK_VERTICES_PER_CELL;
	int tile01index = baseIndex + LAND_BLOCK_STRIDE;
	int tile11index = baseIndex + LAND_BLOCK_VERTICES_PER_CELL + LAND_BLOCK_STRIDE;
	int tileCentreindex = baseIndex + 1;

	int totalLandPoints =
		(tile00 > 0 ? 1 : 0) +
		(tile01 > 0 ? 1 : 0) +
		(tile10 > 0 ? 1 : 0) +
		(tile11 > 0 ? 1 : 0);
	
	bool dothem[4] = { false };
	if (totalLandPoints >= 2)
		dothem[0] = dothem[1] = dothem[2] = dothem[3] = true;

	if (tile00 > 0) {
		dothem[0] = true;
		dothem[1] = true;
	}
	if (tile01 > 0) {
		dothem[1] = true;
		dothem[2] = true;
	}
	if (tile10 > 0) {
		dothem[0] = true;
		dothem[3] = true;
	}
	if (tile11 > 0) {
		dothem[2] = true;
		dothem[3] = true;
	}

	if (dothem[0]) {
		blockIndices->push_back(tile00index);
		blockIndices->push_back(tileCentreindex);
		blockIndices->push_back(tile10index);
	}

	if (dothem[1]) {
		blockIndices->push_back(tile00index);
		blockIndices->push_back(tile01index);
		blockIndices->push_back(tileCentreindex);
	}

	if (dothem[2]) {
		blockIndices->push_back(tile01index);
		blockIndices->push_back(tile11index);
		blockIndices->push_back(tileCentreindex);
	}

	if (dothem[3]) {
		blockIndices->push_back(tile11index);
		blockIndices->push_back(tile10index);
		blockIndices->push_back(tileCentreindex);
	}
}

void LandscapeRenderer::RenderLand(const Camera *camera)
{
	if (this->debugRenderType != this->lastDebugRenderType)
		InitialiseLandShader();

	// Bind terrain textures
	for (int i = 0; i < 8; i++) {
		if (this->terrainTextures[i] != 0) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, this->terrainTextures[i]);
		}
	}

	// Shadow texture
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, this->shadowTexture);

	// Activate the land shader and set inputs
	this->landShader->Use();

	glUniformMatrix4fv(this->landShaderUniform.projectionMatrix, 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));
	glUniformMatrix4fv(this->landShaderUniform.viewMatrix, 1, GL_FALSE, glm::value_ptr(this->modelViewMatrix));
	glUniform1f(this->landShaderUniform.sphereRatio, SphereRatio);
	glUniform3f(this->landShaderUniform.cameraTarget, camera->target.x, camera->target.y, camera->target.z);

	if (this->world->landHighlightActive) {
		glm::ivec3 highlight00 = this->world->landHighlightSource;
		glm::ivec3 highlight11 = this->world->landHighlightTarget;

		int tmp;
		if (highlight00.x > highlight11.x) {
			tmp = highlight00.x;
			highlight00.x = highlight11.x;
			highlight11.x = tmp;
		}

		if (highlight00.z > highlight11.z) {
			tmp = highlight00.z;
			highlight00.z = highlight11.z;
			highlight11.z = tmp;
		}

		glUniform1i(this->landShaderUniform.highlightActive, 1);
		glUniform2f(this->landShaderUniform.highlight00, highlight00.x, highlight00.z);
		glUniform2f(this->landShaderUniform.highlight11, highlight11.x, highlight11.z);
	} else {
		glUniform1i(this->landShaderUniform.highlightActive, 0);
	}

	SetLightSources(camera, this->landShader);

	char name[32];
	for (int i = 0; i < 8; i++) {
		sprintf(name, "InputTexture[%d]", i);
		glUniform1i(glGetUniformLocation(this->landShader->program, name), i);
	}

	glUniform1i(glGetUniformLocation(this->landShader->program, "uShadowTexture"), 8);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE) {
		GLint uniformColour = this->landShader->GetUniformLocation("uColour");
	
		glUniform4f(uniformColour, 0, 0, 0, 1);
		DrawVisibleLandBlocks(camera);
	
		glPolygonMode(GL_FRONT_AND_BACK, this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_POINTS ? GL_POINT : GL_LINE);
		glUniform4f(uniformColour, 0, 0.5f, 0, 1);
		DrawVisibleLandBlocks(camera);
	} else {
		DrawVisibleLandBlocks(camera);
	}
}

void LandscapeRenderer::DrawVisibleLandBlocks(const Camera *camera)
{
	int translateAmount = LAND_BLOCK_SIZE * this->landBlocksPerRow * World::TileSize;

	int argh = 128 * World::TileSize;
	for (int z = camera->target.z - argh; z < camera->target.z + argh; z += World::TileSize * LAND_BLOCK_SIZE) {
		for (int x = camera->target.x - argh; x < camera->target.x + argh; x += World::TileSize * LAND_BLOCK_SIZE) {
			int landX = x / World::TileSize;
			int landZ = z / World::TileSize;

			int blockX = landX / LAND_BLOCK_SIZE;
			int blockZ = landZ / LAND_BLOCK_SIZE;

			int translateX = 0;
			int translateZ = 0;
			while (blockX < 0) {
				blockX += this->landBlocksPerRow;
				translateX -= translateAmount;
			}
			while (blockX >= this->landBlocksPerRow) {
				blockX -= this->landBlocksPerRow;
				translateX += translateAmount;
			}
			while (blockZ < 0) {
				blockZ += this->landBlocksPerRow;
				translateZ -= translateAmount;
			}
			while (blockZ >= this->landBlocksPerRow) {
				blockZ -= this->landBlocksPerRow;
				translateZ += translateAmount;
			}

			glm::mat4 modelMatrix = glm::translate(glm::mat4(), glm::vec3(translateX, 0, translateZ));
			glUniformMatrix4fv(this->landShaderUniform.modelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			this->DrawLandSubBlock(blockX, blockZ);
		}
	}
}

void LandscapeRenderer::DrawLandSubBlock(int blockX, int blockZ)
{
	std::vector<uint32> *blockIndices = &this->landVertexIndices[blockX + blockZ * this->landBlocksPerRow];
	if (blockIndices->size() == 0)
		return;

	int blockOffset = this->GetLandBlockBaseVertexIndex(blockX, blockZ);

	assert(blockOffset >= 0);
	assert(blockOffset + LAND_BLOCK_DATA_SIZE <= this->totalLandVertexBufferSize);

	glBindBuffer(GL_ARRAY_BUFFER, this->glLandVBO);
	glBindVertexArray(this->glLandVAO);

	int indexBlockOffset = this->GetLandBlockBaseVertexIndexIndex(blockX, blockZ);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glLandIndexVBO);
	glDrawElements(GL_TRIANGLES, blockIndices->size(), GL_UNSIGNED_INT, (void*)(indexBlockOffset * sizeof(uint32)));
}

int LandscapeRenderer::GetLandBlockBaseVertexIndex(int blockX, int blockZ) const
{
	return (blockX + blockZ * this->landBlocksPerRow) * LAND_BLOCK_DATA_SIZE;
}

int LandscapeRenderer::GetLandBlockBaseVertexIndexIndex(int blockX, int blockZ) const
{
	return (blockX + blockZ * this->landBlocksPerRow) * LAND_BLOCK_INDEX_DATA_SIZE;
}

int LandscapeRenderer::GetLandBlockVertexIndex(int x, int z) const
{
	return (x * LAND_BLOCK_VERTICES_PER_CELL) + z * ((LAND_BLOCK_SIZE + 1) * LAND_BLOCK_VERTICES_PER_CELL);
}

#pragma endregion

#pragma region Water

void LandscapeRenderer::InitialiseWaterShader()
{
	if (this->waterShader == NULL) {
		glGenVertexArrays(1, &this->glWaterVAO);
	} else {
		delete this->waterShader;
	}

	this->waterShader = OrcaShader::FromPath(
		"water.vert",
		this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_NONE ?
			"water.frag" : "land_wireframe.frag"
	);
	
	this->waterShaderUniform.projectionMatrix = this->waterShader->GetUniformLocation("ProjectionMatrix");
	this->waterShaderUniform.viewMatrix = this->waterShader->GetUniformLocation("ViewMatrix");
	this->waterShaderUniform.modelMatrix = this->waterShader->GetUniformLocation("ModelMatrix");
	this->waterShaderUniform.sphereRatio = this->waterShader->GetUniformLocation("InputSphereRatio");
	this->waterShaderUniform.cameraTarget = this->waterShader->GetUniformLocation("InputCameraTarget");

	glBindBuffer(GL_ARRAY_BUFFER, this->glWaterVBO);
	glBindVertexArray(this->glWaterVAO);
	this->waterShader->SetVertexAttribPointer(sizeof(WaterVertex), WaterShaderVertexInfo);
}

void LandscapeRenderer::InitialiseWaterBlocks()
{
	this->waterBlocksPerRow = this->world->size / WATER_BLOCK_SIZE;
	this->numWaterBlocks = this->waterBlocksPerRow * this->waterBlocksPerRow;
	this->totalWaterVertexBufferSize = this->numWaterBlocks * WATER_BLOCK_DATA_SIZE;
	this->waterVertices = new WaterVertex[this->totalWaterVertexBufferSize];
	this->waterVertexIndices = new std::vector<uint32>[this->numWaterBlocks];

	this->dirtyWaterBlocks = new bool[this->numWaterBlocks];
	memset(this->dirtyWaterBlocks, 0, this->numWaterBlocks * sizeof(bool));

	glGenBuffers(1, &this->glWaterIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glWaterIndexVBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		this->numWaterBlocks * WATER_BLOCK_INDEX_DATA_SIZE * sizeof(uint32),
		NULL,
		GL_STATIC_DRAW
	);

	glGenBuffers(1, &this->glWaterVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->glWaterVBO);
	glBufferData(GL_ARRAY_BUFFER, this->totalWaterVertexBufferSize * sizeof(WaterVertex), NULL, GL_STATIC_DRAW);
}

void LandscapeRenderer::UpdateWaterAllSubBlocks()
{
	const int blocksPerRow = this->landBlocksPerRow;

	for (int z = 0; z < blocksPerRow; z++)
		for (int x = 0; x < blocksPerRow; x++)
			this->UpdateWaterSubBlock(x, z);
}

void LandscapeRenderer::UpdateWaterSubBlock(int blockX, int blockZ)
{
	int landX = blockX * WATER_BLOCK_SIZE;
	int landZ = blockZ * WATER_BLOCK_SIZE;

	// Vertex data
	int blockOffset = this->GetWaterBlockBaseVertexIndex(blockX, blockZ);

	for (int z = 0; z < WATER_BLOCK_SIZE + 1; z++) {
		for (int x = 0; x < WATER_BLOCK_SIZE + 1; x++) {
			int index = blockOffset + this->GetWaterBlockVertexIndex(x, z);
			GetWaterVertex(landX + x, landZ + z, &this->waterVertices[index]);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->glWaterVBO);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		blockOffset * sizeof(WaterVertex),
		WATER_BLOCK_DATA_SIZE * sizeof(WaterVertex),
		&this->waterVertices[blockOffset]
	);

	// Vertex index data
	int indexBlockOffset = this->GetWaterBlockBaseVertexIndexIndex(blockX, blockZ);

	std::vector<uint32> *blockIndices = &this->waterVertexIndices[blockX + blockZ * this->waterBlocksPerRow];
	blockIndices->clear();

	for (int z = 0; z < WATER_BLOCK_SIZE; z++) {
		for (int x = 0; x < WATER_BLOCK_SIZE; x++) {
			int index = blockOffset + this->GetWaterBlockVertexIndex(x, z);
			UpdateWaterSubBlockTileIndices(blockIndices, landX + x, landZ + z, index);
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glWaterIndexVBO);
	glBufferSubData(
		GL_ELEMENT_ARRAY_BUFFER,
		indexBlockOffset * sizeof(uint32),
		blockIndices->size() * sizeof(uint32),
		blockIndices->data()
	);
}

void LandscapeRenderer::UpdateWaterSubBlockTileIndices(std::vector<uint32> *blockIndices, int landX, int landZ, int baseIndex)
{
	int landX0 = landX + 0;
	int landX1 = landX + 1;
	int landZ0 = landZ + 0;
	int landZ1 = landZ + 1;

	// Get heights
	int tile00 = this->world->GetTile(landX0, landZ0)->height;
	int tile01 = this->world->GetTile(landX0, landZ1)->height;
	int tile10 = this->world->GetTile(landX1, landZ0)->height;
	int tile11 = this->world->GetTile(landX1, landZ1)->height;

	if (tile00 != 0 && tile01 != 0 && tile10 != 0 && tile11 != 0)
		return;

	// Get vertex indicies
	int tile00index = baseIndex;
	int tile10index = baseIndex + WATER_BLOCK_VERTICES_PER_CELL;
	int tile01index = baseIndex + WATER_BLOCK_STRIDE;
	int tile11index = baseIndex + WATER_BLOCK_STRIDE + WATER_BLOCK_VERTICES_PER_CELL;

	blockIndices->push_back(tile00index);
	blockIndices->push_back(tile01index);
	blockIndices->push_back(tile10index);

	blockIndices->push_back(tile10index);
	blockIndices->push_back(tile01index);
	blockIndices->push_back(tile11index);
}

void LandscapeRenderer::GetWaterVertex(int landX, int landZ, WaterVertex *topLeft)
{
	int x = landX * World::TileSize;
	int z = landZ * World::TileSize;

	*topLeft = {
		{ x, 0, z }
	};
}

void LandscapeRenderer::RenderWater(const Camera *camera)
{
	if (this->debugRenderType != this->lastDebugRenderType)
		InitialiseWaterShader();

	// Shadow texture
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, this->shadowTexture);

	// Activate the ocean shader and set inputs
	this->waterShader->Use();

	glUniformMatrix4fv(this->waterShaderUniform.projectionMatrix, 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));
	glUniformMatrix4fv(this->waterShaderUniform.viewMatrix, 1, GL_FALSE, glm::value_ptr(this->modelViewMatrix));
	glUniform1f(this->waterShaderUniform.sphereRatio, SphereRatio);
	glUniform3f(this->waterShaderUniform.cameraTarget, camera->target.x, camera->target.y, camera->target.z);

	this->SetLightSources(camera, this->waterShader);
	
	glUniform3fv(waterShader->GetUniformLocation("InputCameraPosition"), 1, glm::value_ptr(camera->eye));
	glUniform1f(waterShader->GetUniformLocation("iGlobalTime"), this->time);

	glUniform1i(glGetUniformLocation(this->landShader->program, "uShadowTexture"), 8);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE) {
		GLint uniformColour = this->landShader->GetUniformLocation("uColour");
	
		glUniform4f(uniformColour, 0, 0, 0, 1);
		this->DrawVisibleWaterBlocks(camera);
	
		glPolygonMode(GL_FRONT_AND_BACK, this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_POINTS ? GL_POINT : GL_LINE);
		glUniform4f(uniformColour, 0, 0, 0.75f, 1);
		this->DrawVisibleWaterBlocks(camera);
	} else {
		this->DrawVisibleWaterBlocks(camera);
	}
}

void LandscapeRenderer::DrawVisibleWaterBlocks(const Camera *camera)
{
	int translateAmount = WATER_BLOCK_SIZE * this->waterBlocksPerRow * World::TileSize;

	int argh = 128 * World::TileSize;
	for (int z = camera->target.z - argh; z < camera->target.z + argh; z += World::TileSize * WATER_BLOCK_SIZE) {
		for (int x = camera->target.x - argh; x < camera->target.x + argh; x += World::TileSize * WATER_BLOCK_SIZE) {
			int landX = x / World::TileSize;
			int landZ = z / World::TileSize;

			int blockX = landX / WATER_BLOCK_SIZE;
			int blockZ = landZ / WATER_BLOCK_SIZE;

			int translateX = 0;
			int translateZ = 0;
			while (blockX < 0) {
				blockX += this->waterBlocksPerRow;
				translateX -= translateAmount;
			}
			while (blockX >= this->waterBlocksPerRow) {
				blockX -= this->waterBlocksPerRow;
				translateX += translateAmount;
			}
			while (blockZ < 0) {
				blockZ += this->waterBlocksPerRow;
				translateZ -= translateAmount;
			}
			while (blockZ >= this->waterBlocksPerRow) {
				blockZ -= this->waterBlocksPerRow;
				translateZ += translateAmount;
			}

			glm::mat4 modelMatrix = glm::translate(glm::mat4(), glm::vec3(translateX, 0, translateZ));
			glUniformMatrix4fv(this->waterShaderUniform.modelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			this->DrawWaterSubBlock(blockX, blockZ);
		}
	}
}

void LandscapeRenderer::DrawWaterSubBlock(int blockX, int blockZ)
{
	std::vector<uint32> *blockIndices = &this->waterVertexIndices[blockX + blockZ * this->waterBlocksPerRow];
	if (blockIndices->size() == 0)
		return;

	int blockOffset = this->GetWaterBlockBaseVertexIndex(blockX, blockZ);

	assert(blockOffset >= 0);
	assert(blockOffset + WATER_BLOCK_DATA_SIZE <= this->totalWaterVertexBufferSize);

	glBindBuffer(GL_ARRAY_BUFFER, this->glWaterVBO);
	glBindVertexArray(this->glWaterVAO);

	int indexBlockOffset = this->GetWaterBlockBaseVertexIndexIndex(blockX, blockZ);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glWaterIndexVBO);
	glDrawElements(GL_TRIANGLES, blockIndices->size(), GL_UNSIGNED_INT, (void*)(indexBlockOffset * sizeof(uint32)));
}

int LandscapeRenderer::GetWaterBlockBaseVertexIndex(int blockX, int blockZ) const
{
	return (blockX + blockZ * this->waterBlocksPerRow) * WATER_BLOCK_DATA_SIZE;
}

int LandscapeRenderer::GetWaterBlockBaseVertexIndexIndex(int blockX, int blockZ) const
{
	return (blockX + blockZ * this->waterBlocksPerRow) * WATER_BLOCK_INDEX_DATA_SIZE;
}

int LandscapeRenderer::GetWaterBlockVertexIndex(int x, int z) const
{
	return (x * WATER_BLOCK_VERTICES_PER_CELL) + z * ((WATER_BLOCK_SIZE + 1) * WATER_BLOCK_VERTICES_PER_CELL);
}

#pragma endregion