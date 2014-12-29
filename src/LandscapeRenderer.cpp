#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "LandscapeRenderer.h"
#include "TerrainStyle.h"
#include "Util/MathExtensions.hpp"
#include "World.h"

using namespace IntelOrca::PopSS;

const float LandscapeRenderer::SphereRatio = 0.00002f;
const float LandscapeRenderer::TextureMapSize = 1.0f / 4.0f;

LandscapeRenderer::LandscapeRenderer()
{
	this->landViewSize = 76;
	this->oceanViewSize = 52;
}

LandscapeRenderer::~LandscapeRenderer()
{

}

void LandscapeRenderer::Render(const Camera *camera)
{
	// Get the camera view matrix
	this->projectionMatrix = camera->Get3dProjectionMatrix();
	this->modelViewMatrix = camera->Get3dViewMatrix();

	glClear(GL_DEPTH_BUFFER_BIT);

	glBlendFunc(GL_BLEND_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(this->projectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(this->modelViewMatrix));

	this->RenderLand(camera);


	// this->RenderOcean(camera);
	// this->RenderObjects(camera);
}



void LandscapeRenderer::RenderArea(const Camera *camera, int viewSize, bool water)
{
	// Begin generating the vertices
	glBegin(GL_TRIANGLES);

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
	glEnd();
}


void LandscapeRenderer::RenderLand(const Camera *camera)
{
	// Bind terrain textures
	// for (int i = 0; i < _terrainTextures.Length; i++)
	//	_terrainTextures[i].Bind(i);

	// Activate the land shader and set inputs
	// _landShaderProgram.Activate();
	// _landShaderProgram.SetUniform("ModelViewMatrix", _modelViewMatrix);
	// _landShaderProgram.SetUniform("ProjectionMatrix", _projectionMatrix);

	// _landShaderProgram.SetUniform("InputSphereRatio", SphereRatio);

	// RendererUtil.SetLightSources(_landShaderProgram, "InputLightSources", _world.LightSourceManager);

	// for (int i = 0; i < 8; i++)
	//	_landShaderProgram.SetUniform("InputTexture[" + i + "]", i);

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
		tile->height,
		// terrainStyle->textureIndex,
		glm::vec4(
			terrainStyle->ambientReflectivity,
			terrainStyle->diffuseReflectivity,
			terrainStyle->specularReflectivity,
			terrainStyle->shininess
		)
	);
}

void LandscapeRenderer::AddVertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv, int texture, glm::vec4 material)
{
	glm::vec4 colour = GetColourFromLand(texture);

	glColor3f(colour.a, colour.g, colour.b);
	glVertex3f(position.x, position.y, position.z);
	glNormal3f(normal.x, normal.y, normal.z);
	glTexCoord2f(uv.s, uv.t);
	// _vertexBuffer.AddValues(position, normal, uv, (float)texture, material);
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