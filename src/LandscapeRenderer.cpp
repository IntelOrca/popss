#include "Camera.h"
#include "LandscapeRenderer.h"
#include "LightSource.h"
#include "OrcaShader.h"
#include "TerrainStyle.h"
#include "Util/MathExtensions.hpp"
#include "VertexBuffer.h"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <lodepng/lodepng.h>

using namespace IntelOrca::PopSS;

const float LandscapeRenderer::SphereRatio = 0.00002f;
const float LandscapeRenderer::TextureMapSize = 1.0f / 4.0f;

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	return true;
}

LandscapeRenderer::LandscapeRenderer()
{
	this->landViewSize = 76;
	this->oceanViewSize = 52;
}

LandscapeRenderer::~LandscapeRenderer()
{

}

void LandscapeRenderer::Initialise()
{
	int landVectorCounts[] = { 3, 3, 2, 1, 4, 0 };
	this->landVertexBuffer = new VertexBuffer(landVectorCounts);

	this->landShader = OrcaShader::FromPath("land.vert", "land.frag");

	memset(this->terrainTextures, 0, sizeof(this->terrainTextures));
	glGenTextures(5, this->terrainTextures);
	
	LoadTexture(this->terrainTextures[0], "data/textures/sand.png");
	LoadTexture(this->terrainTextures[1], "data/textures/grass.png");
	LoadTexture(this->terrainTextures[2], "data/textures/snow.png");
	LoadTexture(this->terrainTextures[3], "data/textures/cliff.png");
	LoadTexture(this->terrainTextures[4], "data/textures/dirt.png");
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
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// glMatrixMode(GL_PROJECTION);
	// glLoadMatrixf(glm::value_ptr(this->projectionMatrix));

	// glMatrixMode(GL_MODELVIEW);
	// glLoadMatrixf(glm::value_ptr(this->modelViewMatrix));

	this->RenderLand(camera);


	// this->RenderOcean(camera);
	// this->RenderObjects(camera);
}



void LandscapeRenderer::RenderArea(const Camera *camera, int viewSize, bool water)
{
	// Begin generating the vertices
	this->landVertexBuffer->Begin();

	float translateX, translateZ;
	int landOriginX, landOriginZ;

	splitdecimalcomponents(camera->target.x, &landOriginX, &translateX);
	splitdecimalcomponents(camera->target.z, &landOriginZ, &translateZ);

	for (int landOffsetZ = -viewSize; landOffsetZ <= viewSize; landOffsetZ++) {
		for (int landOffsetX = -viewSize; landOffsetX <= viewSize; landOffsetX++) {
			float lengthFromCentrePoint = glm::length(glm::vec2(landOffsetX, landOffsetZ));
			if (lengthFromCentrePoint > viewSize)
				continue;

			double angle = atan2(landOffsetZ, landOffsetX) / M_PI * 180.0;
			double cameraAngle = wrapangledeg(camera->rotation - 90);

			if (lengthFromCentrePoint > camera->zoom / 80.0)
				if (abs(smallestangledeltadeg(angle, cameraAngle)) > 90)
					continue;

			float vx = (landOffsetX - translateX) * World::TileSize;
			float vz = (landOffsetZ - translateZ) * World::TileSize;
			if (!water)
				RenderLandQuad(landOriginX + landOffsetX, landOriginZ + landOffsetZ, vx, vz);
		}
	}

	// Finish generation and render primitives
	this->landVertexBuffer->End();
	this->landVertexBuffer->Draw(GL_TRIANGLES);
}


void LandscapeRenderer::RenderLand(const Camera *camera)
{
	// Bind terrain textures
	for (int i = 0; i < 8; i++) {
		if (this->terrainTextures[i] != 0) {
			glBindTexture(GL_TEXTURE_2D, i);
			glActiveTexture(GL_TEXTURE0 + i);
		}
	}

	// Activate the land shader and set inputs
	this->landShader->Use();

	glUniformMatrix4fv(glGetUniformLocation(this->landShader->program, "ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(this->landShader->program, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(this->modelViewMatrix));
	glUniform1f(glGetUniformLocation(this->landShader->program, "InputSphereRatio"), SphereRatio);

	LightSource alight, *light = &alight;

	light->position = glm::vec3(0.0f, 2048.0f, 0.0f);
	light->ambient = glm::vec4(0.05f);
	light->diffuse = glm::vec4(0.4f);
	light->specular = glm::vec4(0.0f);

	glUniform1i(glGetUniformLocation(this->landShader->program, "InputLightSourcesCount"), 2);
	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[0].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[0].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[0].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[0].Specular"), 1, glm::value_ptr(light->specular));

	light->position = glm::vec3(-1.0f, 0.1f, 1.0f) * World::SkyDomeRadius;
	light->ambient = glm::vec4(0.0f);
	light->diffuse = glm::vec4(0.5f, 0.5f, 0.0f, 0.0f);
	light->specular = glm::vec4(0.25f, 0.25f, 0.0f, 0.0f);

	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[1].Position"), 1, glm::value_ptr(light->position));
	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[1].Ambient"), 1, glm::value_ptr(light->ambient));
	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[1].Diffuse"), 1, glm::value_ptr(light->diffuse));
	glUniform3fv(glGetUniformLocation(this->landShader->program, "InputLightSources[1].Specular"), 1, glm::value_ptr(light->specular));

	char name[32];
	for (int i = 0; i < 8; i++) {
		sprintf(name, "InputTexture[%d]", i);
		glUniform1i(glGetUniformLocation(this->landShader->program, name), i);
	}

	RenderArea(camera, this->landViewSize, false);
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

void LandscapeRenderer::AddVertex(glm::vec3 position, glm::vec2 uv, const WorldTile *tile)
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

void LandscapeRenderer::AddVertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv, int texture, const glm::vec4 &material)
{
	// glm::vec4 colour = GetColourFromLand(texture);

	this->landVertexBuffer->AddValue(0, position);
	this->landVertexBuffer->AddValue(1, normal);
	this->landVertexBuffer->AddValue(2, uv);
	this->landVertexBuffer->AddValue(3, texture);
	this->landVertexBuffer->AddValue(4, material);
}

glm::vec2 LandscapeRenderer::GetTextureUV(int landX, int landZ)
{
	return glm::vec2(
		fmod(this->world->MapWrap(landX) * TextureMapSize, 1.0f),
		fmod(this->world->MapWrap(landZ) * TextureMapSize, 1.0f)
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