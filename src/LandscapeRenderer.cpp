#include "Camera.h"
#include "LandscapeRenderer.h"
#include "LightSource.h"
#include "OrcaShader.h"
#include "TerrainStyle.h"
#include "Util/MathExtensions.hpp"
#include "World.h"
#include "WorldObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <lodepng/lodepng.h>

using namespace IntelOrca::PopSS;

// const float LandscapeRenderer::SphereRatio = 0.00002f;
const float LandscapeRenderer::SphereRatio = 0.0f;
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
	{ "VertexNormal",			GL_FLOAT,			3,	offsetof(WaterVertex, normal)		},
	{ "VertexTextureCoords",	GL_FLOAT,			2,	offsetof(WaterVertex, texcoords)	},
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
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

LandscapeRenderer::LandscapeRenderer()
{
	this->debugRenderType = DEBUG_LANDSCAPE_RENDER_TYPE_NONE;

	this->landViewSize = 76;
	this->oceanViewSize = 52;

	this->time = 0;
}

LandscapeRenderer::~LandscapeRenderer()
{

}

void LandscapeRenderer::Initialise()
{
	this->landShader = OrcaShader::FromPath("land.vert", "land.frag");
	this->landShaderUniform.projectionMatrix = this->landShader->GetUniformLocation("ProjectionMatrix");
	this->landShaderUniform.modelViewMatrix = this->landShader->GetUniformLocation("ModelViewMatrix");
	this->landShaderUniform.sphereRatio = this->landShader->GetUniformLocation("InputSphereRatio");
	this->landShaderUniform.highlightActive = this->landShader->GetUniformLocation("InputHighlightActive");
	this->landShaderUniform.highlight00 = this->landShader->GetUniformLocation("InputHighlight00");
	this->landShaderUniform.highlight11 = this->landShader->GetUniformLocation("InputHighlight11");

	// Vertex buffer
	glGenBuffers(1, &this->landVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->landVBO);

	// Vertex array
	glGenVertexArrays(1, &this->landVAO);
	glBindVertexArray(this->landVAO);

	this->landShader->SetVertexAttribPointer(sizeof(LandVertex), LandShaderVertexInfo);


	this->waterShader = OrcaShader::FromPath("water.vert", "water.frag");
	this->waterShaderUniform.projectionMatrix = this->waterShader->GetUniformLocation("ProjectionMatrix");
	this->waterShaderUniform.modelViewMatrix = this->waterShader->GetUniformLocation("ModelViewMatrix");
	this->waterShaderUniform.sphereRatio = this->waterShader->GetUniformLocation("InputSphereRatio");

	glGenBuffers(1, &this->waterVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->waterVBO);

	// Vertex array
	glGenVertexArrays(1, &this->waterVAO);
	glBindVertexArray(this->waterVAO);

	this->waterShader->SetVertexAttribPointer(sizeof(WaterVertex), WaterShaderVertexInfo);

	memset(this->terrainTextures, 0, sizeof(this->terrainTextures));
	glGenTextures(5, this->terrainTextures);
	
	LoadTexture(this->terrainTextures[0], "data/textures/sand.png");
	LoadTexture(this->terrainTextures[1], "data/textures/grass.png");
	LoadTexture(this->terrainTextures[2], "data/textures/snow.png");
	LoadTexture(this->terrainTextures[3], "data/textures/cliff.png");
	LoadTexture(this->terrainTextures[4], "data/textures/dirt.png");

	glGenTextures(1, &this->waterTexture);
	LoadTexture(this->waterTexture, "data/textures/water.png");
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

	this->RenderLand(camera);
	this->RenderOcean(camera);

	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	this->RenderObjects(camera);	

	this->time++;
}



void LandscapeRenderer::RenderArea(const Camera *camera, int viewSize, bool water)
{
	float translateX, translateZ;
	int landOriginX, landOriginZ;

	landOriginX = camera->target.x / World::TileSize;
	landOriginZ = camera->target.z / World::TileSize;
	translateX = camera->target.x - (landOriginX * World::TileSize);
	translateZ = camera->target.z - (landOriginZ * World::TileSize);

	for (int landOffsetZ = -viewSize; landOffsetZ <= viewSize; landOffsetZ++) {
		for (int landOffsetX = -viewSize; landOffsetX <= viewSize; landOffsetX++) {
			int landX = this->world->TileWrap(landOriginX + landOffsetX);
			int landZ = this->world->TileWrap(landOriginZ + landOffsetZ);

			float lengthFromCentrePoint = glm::length(glm::vec2(landOffsetX, landOffsetZ));
			if (lengthFromCentrePoint > viewSize)
				continue;

			float angle = todegrees(atan2(landOffsetZ, -landOffsetX));
			float cameraAngle = wrapangledeg(camera->rotation - 90);

			if (lengthFromCentrePoint > camera->zoom / 80.0f)
				if (abs(smallestangledeltadeg(angle, cameraAngle)) > 90.0f)
					continue;

			float vx = camera->target.x + (landOffsetX * World::TileSize) - translateX;
			float vz = camera->target.z + (landOffsetZ * World::TileSize) - translateZ;
			if (!water)
				RenderLandQuad(landX, landZ, vx, vz);
			else
				RenderOceanQuad(landX, landZ, vx, vz);
		}
	}
}


void LandscapeRenderer::RenderLand(const Camera *camera)
{
	// Bind terrain textures
	for (int i = 0; i < 8; i++) {
		if (this->terrainTextures[i] != 0) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, this->terrainTextures[i]);
		}
	}

	// Activate the land shader and set inputs
	this->landShader->Use();

	glUniformMatrix4fv(this->landShaderUniform.projectionMatrix, 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));
	glUniformMatrix4fv(this->landShaderUniform.modelViewMatrix, 1, GL_FALSE, glm::value_ptr(this->modelViewMatrix));
	glUniform1f(this->landShaderUniform.sphereRatio, SphereRatio);

	if (this->world->landHighlightActive) {
		glm::ivec3 highlight00 = this->world->landHighlightSource - (glm::ivec3)camera->target;
		glm::ivec3 highlight11 = this->world->landHighlightTarget - (glm::ivec3)camera->target;
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

	if (camera->viewHasChanged) {
		landVertices.clear();

		RenderArea(camera, this->landViewSize, false);

		glBindBuffer(GL_ARRAY_BUFFER, this->landVBO);
		glBufferData(GL_ARRAY_BUFFER, this->landVertices.size() * sizeof(LandVertex), this->landVertices.data(), GL_STATIC_DRAW);
	}

	glBindVertexArray(this->landVAO);
	glDrawArrays(GL_TRIANGLES, 0, this->landVertices.size());
}

void LandscapeRenderer::RenderLandQuad(int landX, int landZ, float x, float z)
{
	WorldTile *tile00 = this->world->GetTile(landX + 0, landZ + 0);
	WorldTile *tile01 = this->world->GetTile(landX + 0, landZ + 1);
	WorldTile *tile10 = this->world->GetTile(landX + 1, landZ + 0);
	WorldTile *tile11 = this->world->GetTile(landX + 1, landZ + 1);

	int totalHeightA = tile00->height + tile01->height + tile11->height;
	int totalHeightB = tile00->height + tile11->height + tile10->height;

	if (totalHeightA > 0)
		RenderLandTriangle000111(x, z, landX, landZ);
	if (totalHeightB > 0)
		RenderLandTriangle001110(x, z, landX, landZ);
}

void LandscapeRenderer::RenderLandTriangle000111(float vx, float vz, int landX, int landZ)
{
	// Offset indicies
	int offsetsX[3] = { 0, 0, 1 };
	int offsetsZ[3] = { 0, 1, 1 };

	// Calculate UV coordinates
	glm::vec2 baseUV = GetTextureUV(landX, landZ);
	glm::vec2 uv[3] = {
		baseUV,
		glm::vec2(baseUV.x, baseUV.y + TextureMapSize),
		glm::vec2(baseUV.x + TextureMapSize, baseUV.y + TextureMapSize)
	};

	RenderLandTriangle(vx, vz, landX, landZ, offsetsX, offsetsZ, uv);
}

void LandscapeRenderer::RenderLandTriangle001110(float vx, float vz, int landX, int landZ)
{
	// Offset indicies
	int offsetsX[3] = { 0, 1, 1 };
	int offsetsZ[3] = { 0, 1, 0 };

	// Calculate UV coordinates
	glm::vec2 baseUV = GetTextureUV(landX, landZ);
	glm::vec2 uv[3] = {
		baseUV,
		glm::vec2(baseUV.x + TextureMapSize, baseUV.y + TextureMapSize),
		glm::vec2(baseUV.x + TextureMapSize, baseUV.y)
	};

	RenderLandTriangle(vx, vz, landX, landZ, offsetsX, offsetsZ, uv);
}

void LandscapeRenderer::RenderLandTriangle(float vx, float vz, int landX, int landZ, const int *offsetsX, const int *offsetsZ, const glm::vec2 *uv)
{
	for (int i = 0; i < 3; i++) {
		const WorldTile *tile = this->world->GetTile(landX + offsetsX[i], landZ + offsetsZ[i]);
		this->AddVertex(
			glm::vec3(vx + (offsetsX[i] * World::TileSize), tile->height, vz + (offsetsZ[i] * World::TileSize)),
			uv[i],
			tile
		);
	}
}

void LandscapeRenderer::RenderOcean(const Camera *camera)
{
	// Bind water texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->waterTexture);

	// Activate the ocean shader and set inputs
	this->waterShader->Use();

	glUniformMatrix4fv(this->waterShaderUniform.projectionMatrix, 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));
	glUniformMatrix4fv(this->waterShaderUniform.modelViewMatrix, 1, GL_FALSE, glm::value_ptr(this->modelViewMatrix));
	glUniform1f(this->waterShaderUniform.sphereRatio, SphereRatio);

	this->SetLightSources(camera, this->waterShader);

	glUniform1i(this->waterShader->GetUniformLocation("InputTexture0"), 0);

	if (camera->viewHasChanged) {
		waterVertices.clear();

		RenderArea(camera, this->oceanViewSize, true);

		glBindBuffer(GL_ARRAY_BUFFER, this->waterVBO);
		glBufferData(GL_ARRAY_BUFFER, this->waterVertices.size() * sizeof(WaterVertex), this->waterVertices.data(), GL_STATIC_DRAW);
	}

	glBindVertexArray(this->waterVAO);
	glDrawArrays(GL_TRIANGLES, 0, this->waterVertices.size());
}

void LandscapeRenderer::RenderOceanQuad(int landX, int landZ, float vx, float vz)
{
	const int wtpt = (int)(World::TileSize / World::OceanTileSize);

	WorldTile *tile00 = this->world->GetTile(landX + 0, landZ + 0);
	WorldTile *tile01 = this->world->GetTile(landX + 0, landZ + 1);
	WorldTile *tile10 = this->world->GetTile(landX + 1, landZ + 0);
	WorldTile *tile11 = this->world->GetTile(landX + 1, landZ + 1);

	int totalHeightA = tile00->height + tile01->height + tile11->height;
	int totalHeightB = tile00->height + tile11->height + tile10->height;

	for (int z = 0; z < wtpt; z++) {
		for (int x = 0; x < wtpt; x++) {
			float tvx = vx + (x * World::OceanTileSize);
			float tvz = vz + (z * World::OceanTileSize);

			if ((z - x >= 0 && totalHeightA <= 0) || (z - x < 0 && totalHeightB <= 0))
				RenderOceanTriangle000111(tvx, tvz, (landX * wtpt) + x, (landZ * wtpt) + z);
			if ((x - z >= 0 && totalHeightB <= 0) || (x - z < 0 && totalHeightA <= 0))
				RenderOceanTriangle001110(tvx, tvz, (landX * wtpt) + x, (landZ * wtpt) + z);
		}
	}
}

void LandscapeRenderer::RenderOceanTriangle000111(float vx, float vz, int oceanX, int oceanZ)
{
	// Offset indicies
	int offsetsX[] = { 0, 0, 1 };
	int offsetsZ[] = { 0, 1, 1 };

	// Calculate UV coordinates
	glm::vec2 baseUV = GetTextureUV(oceanX, oceanZ);
	glm::vec2 uv[] = {
		baseUV,
		glm::vec2(baseUV.x, baseUV.y + TextureMapSize),
		glm::vec2(baseUV.x + TextureMapSize, baseUV.y + TextureMapSize)
	};

	RenderOceanTriangle(vx, vz, oceanX, oceanZ, offsetsX, offsetsZ, uv);
}

void LandscapeRenderer::RenderOceanTriangle001110(float vx, float vz, int oceanX, int oceanZ)
{
	// Offset indicies
	int offsetsX[] = { 0, 1, 1 };
	int offsetsZ[] = { 0, 1, 0 };

	// Calculate UV coordinates
	glm::vec2 baseUV = GetTextureUV(oceanX, oceanZ);
	glm::vec2 uv[] = {
		baseUV,
		glm::vec2(baseUV.x + TextureMapSize, baseUV.y + TextureMapSize),
		glm::vec2(baseUV.x + TextureMapSize, baseUV.y)
	};
	 
	RenderOceanTriangle(vx, vz, oceanX, oceanZ, offsetsX, offsetsZ, uv);
}

void LandscapeRenderer::RenderOceanTriangle(float vx, float vz, int oceanX, int oceanZ, const int *offsetsX, const int *offsetsZ, const glm::vec2 *uv)
{
	for (int i = 0; i < 3; i++) {
		int ox = oceanX + offsetsX[i];
		int oz = oceanZ + offsetsZ[i];

		float h = (float)GetWaveHeight(ox, oz);
		glm::vec3 normal = World::CalculateNormal(
			GetWaveHeight(ox, oz),
			GetWaveHeight(ox - 1, oz),
			GetWaveHeight(ox + 1, oz),
			GetWaveHeight(ox, oz - 1),
			GetWaveHeight(ox, oz + 1)
		);

		this->waterVertices.push_back({
			glm::vec3(vx + (offsetsX[i] * World::OceanTileSize), h, vz + (offsetsZ[i] * World::OceanTileSize)),
			normal,
			uv[i]
		});
	}
}

float LandscapeRenderer::GetWaveHeight(int oceanX, int oceanZ)
{
	int wtpt = (int)(World::TileSize / World::OceanTileSize);
	int landX = oceanX / wtpt;
	int landZ = oceanZ / wtpt;
	if (this->world->GetTile(landX, landZ)->shore)
		return 0.0f;

	float phase = oceanZ / (float)(M_PI * 32);
	float waveHeight = 16.0f;
	return sin(this->time * 0.1f + phase) * waveHeight + waveHeight;
}

void LandscapeRenderer::AddVertex(const glm::vec3 &position, const glm::vec2 &uv, const WorldTile *tile)
{
	const TerrainStyle *terrainStyle;

	if (tile->terrain == 255) {
		TerrainStyle plainTerrainStyle;

		terrainStyle = &plainTerrainStyle;
	} else {
		terrainStyle = &this->world->terrainStyles[tile->terrain];
	}	

	this->AddVertex(
		position,
		tile->lightNormal,
		uv,
		terrainStyle->textureIndex,
		glm::vec4(
			terrainStyle->ambientReflectivity,
			terrainStyle->diffuseReflectivity,
			terrainStyle->specularReflectivity,
			terrainStyle->shininess
		)
	);
}

void LandscapeRenderer::AddVertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &uv, int texture, const glm::vec4 &material)
{
	// glm::vec4 colour = GetColourFromLand(texture);

	this->landVertices.push_back({
		position,
		normal,
		uv,
		texture,
		material
	});

	// this->landVertexBuffer->AddValue(0, position);
	// this->landVertexBuffer->AddValue(1, normal);
	// this->landVertexBuffer->AddValue(2, uv);
	// this->landVertexBuffer->AddValue(3, texture);
	// this->landVertexBuffer->AddValue(4, material);
}

glm::vec2 LandscapeRenderer::GetTextureUV(int landX, int landZ)
{
	return glm::vec2(
		fmod(this->world->TileWrap(landX) * TextureMapSize, 1.0f),
		fmod(this->world->TileWrap(landZ) * TextureMapSize, 1.0f)
	);
}

glm::vec4 LandscapeRenderer::GetColourFromLand(int height)
{
	if (height == 0 && false)
		return glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	else {
		float c = 0.5f + ((height / 1024.0f) * 0.5f);
		return glm::vec4(c, c, c, c);
	}
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

void LandscapeRenderer::RenderObjects(const Camera *camera)
{
	for (WorldObject *worldObject : this->world->objects) {
		worldObject->Draw();
	}
}