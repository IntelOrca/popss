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
	this->lastDebugRenderType = this->debugRenderType;

	this->landViewSize = 128;
	this->oceanViewSize = 52;

	this->time = 0;

	this->landShader = NULL;
	this->landVertexBuffer = NULL;

	this->waterShader = NULL;
	this->waterVertexBuffer = NULL;
}

LandscapeRenderer::~LandscapeRenderer()
{
	if (this->landShader != NULL) delete this->landShader;
	if (this->landVertexBuffer != NULL) delete this->landVertexBuffer;

	if (this->waterShader != NULL) delete this->waterShader;
	if (this->waterVertexBuffer != NULL) delete this->waterVertexBuffer;
}

void LandscapeRenderer::Initialise()
{
	this->InitialiseLandShader();
	this->InitialiseWaterShader();

	memset(this->terrainTextures, 0, sizeof(this->terrainTextures));
	glGenTextures(5, this->terrainTextures);
	
	LoadTexture(this->terrainTextures[0], "data/textures/sand.png");
	LoadTexture(this->terrainTextures[1], "data/textures/grass.png");
	LoadTexture(this->terrainTextures[2], "data/textures/snow.png");
	LoadTexture(this->terrainTextures[3], "data/textures/cliff.png");
	LoadTexture(this->terrainTextures[4], "data/textures/dirt.png");

	glGenTextures(1, &this->waterTexture);
	LoadTexture(this->waterTexture, "data/textures/water.png");

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

void LandscapeRenderer::InitialiseLandShader()
{
	SafeDelete(this->landShader);

	this->landShader = OrcaShader::FromPath(
		"land.vert",
		this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_NONE ?
			"land.frag" : "land_wireframe.frag"
	);
	
	this->landShaderUniform.projectionMatrix = this->landShader->GetUniformLocation("ProjectionMatrix");
	this->landShaderUniform.modelViewMatrix = this->landShader->GetUniformLocation("ModelViewMatrix");
	this->landShaderUniform.sphereRatio = this->landShader->GetUniformLocation("InputSphereRatio");
	this->landShaderUniform.cameraTarget = this->landShader->GetUniformLocation("InputCameraTarget");
	this->landShaderUniform.highlightActive = this->landShader->GetUniformLocation("InputHighlightActive");
	this->landShaderUniform.highlight00 = this->landShader->GetUniformLocation("InputHighlight00");
	this->landShaderUniform.highlight11 = this->landShader->GetUniformLocation("InputHighlight11");

	if (this->landVertexBuffer == NULL)
		this->landVertexBuffer = new SimpleVertexBuffer<LandVertex>();

	this->landVertexBuffer->Initialise(this->landShader, LandShaderVertexInfo);
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
	glUniformMatrix4fv(this->landShaderUniform.modelViewMatrix, 1, GL_FALSE, glm::value_ptr(this->modelViewMatrix));
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

	if (camera->viewHasChanged)
		UpdateLandPrimitives(camera);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE) {
		GLint uniformColour = this->landShader->GetUniformLocation("uColour");

		glUniform4f(uniformColour, 0, 0, 0, 1);
		this->landVertexBuffer->Draw(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_POINTS ? GL_POINT : GL_LINE);
		glUniform4f(uniformColour, 1, 1, 1, 1);
		this->landVertexBuffer->Draw(GL_TRIANGLES);
	} else {
		this->landVertexBuffer->Draw(GL_TRIANGLES);
	}
}

void LandscapeRenderer::UpdateLandPrimitives(const Camera *camera)
{
	float translateX, translateZ;
	int landOriginX, landOriginZ;

	this->landVertexBuffer->Clear();

	landOriginX = camera->target.x / World::TileSize;
	landOriginZ = camera->target.z / World::TileSize;
	translateX = camera->target.x - (landOriginX * World::TileSize);
	translateZ = camera->target.z - (landOriginZ * World::TileSize);

	for (int landOffsetZ = -this->landViewSize; landOffsetZ <= this->landViewSize; landOffsetZ++) {
		for (int landOffsetX = -this->landViewSize; landOffsetX <= this->landViewSize; landOffsetX++) {
			int landX = this->world->TileWrap(landOriginX + landOffsetX);
			int landZ = this->world->TileWrap(landOriginZ + landOffsetZ);

			float lengthFromCentrePoint = glm::length(glm::vec2(landOffsetX, landOffsetZ));
			if (lengthFromCentrePoint > this->landViewSize)
				continue;

			float angle = todegrees(atan2(landOffsetZ, -landOffsetX));
			float cameraAngle = wrapangledeg(camera->rotation - 90);

			if (lengthFromCentrePoint > camera->zoom / 80.0f)
				if (abs(smallestangledeltadeg(angle, cameraAngle)) > 90.0f)
					continue;

			float vx = camera->target.x + (landOffsetX * World::TileSize) - translateX;
			float vz = camera->target.z + (landOffsetZ * World::TileSize) - translateZ;
			AddLandQuad(landX, landZ, vx, vz);
		}
	}

	this->landVertexBuffer->Update();
}

void LandscapeRenderer::AddLandQuad(int landX, int landZ, float x, float z)
{
	WorldTile *tile00 = this->world->GetTile(landX + 0, landZ + 0);
	WorldTile *tile01 = this->world->GetTile(landX + 0, landZ + 1);
	WorldTile *tile10 = this->world->GetTile(landX + 1, landZ + 0);
	WorldTile *tile11 = this->world->GetTile(landX + 1, landZ + 1);

	// \--/
	// |\/|
	// |/\|
	// /--\

	int totalLandPoints =
		(tile00->height > 0 ? 1 : 0) +
		(tile01->height > 0 ? 1 : 0) +
		(tile10->height > 0 ? 1 : 0) +
		(tile11->height > 0 ? 1 : 0);
	
	bool dothem[4] = { false };
	if (totalLandPoints >= 2)
		dothem[0] = dothem[1] = dothem[2] = dothem[3] = true;

	if (tile00->height > 0) {
		dothem[0] = true;
		dothem[1] = true;
	}
	if (tile01->height > 0) {
		dothem[1] = true;
		dothem[2] = true;
	}
	if (tile10->height > 0) {
		dothem[0] = true;
		dothem[3] = true;
	}
	if (tile11->height > 0) {
		dothem[2] = true;
		dothem[3] = true;
	}


	if (dothem[0]) {
		float offsetsX[3] = { 0, 0.5, 1 };
		float offsetsZ[3] = { 0, 0.5, 0 };
		AddLandTriangle(x, z, landX, landZ, offsetsX, offsetsZ);
	}

	if (dothem[1]) {
		float offsetsX[3] = { 0, 0, 0.5 };
		float offsetsZ[3] = { 0, 1, 0.5 };
		AddLandTriangle(x, z, landX, landZ, offsetsX, offsetsZ);
	}

	if (dothem[2]) {
		float offsetsX[3] = { 0, 1, 0.5 };
		float offsetsZ[3] = { 1, 1, 0.5 };
		AddLandTriangle(x, z, landX, landZ, offsetsX, offsetsZ);
	}

	if (dothem[3]) {
		float offsetsX[3] = { 1, 1, 0.5 };
		float offsetsZ[3] = { 1, 0, 0.5 };
		AddLandTriangle(x, z, landX, landZ, offsetsX, offsetsZ);
	}
}

void LandscapeRenderer::AddLandTriangle(float x, float z, int landX, int landZ, const float *offsetsX, const float *offsetsZ)
{
	// Calculate UV coordinates
	glm::vec2 baseUV = GetTextureUV(landX, landZ);
	glm::vec2 uv[3];
	for (int i = 0; i < 3; i++)
		uv[i] = glm::vec2(baseUV.s + TextureMapSize * offsetsX[i], baseUV.t + TextureMapSize * offsetsZ[i]);

	for (int i = 0; i < 3; i++) {
		const WorldTile *tile = this->world->GetTile(landX + offsetsX[i], landZ + offsetsZ[i]);
		int height = tile->height;

		if (offsetsX[i] == 0.5f && offsetsZ[i] == 0.5f) {
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
				height = 0;
			else
				height = (heights[0] + heights[1] + heights[2] + heights[3]) / 4;
		}

		this->AddLandVertex(
			glm::vec3(x + (offsetsX[i] * World::TileSize), height, z + (offsetsZ[i] * World::TileSize)),
			uv[i],
			tile
		);
	}
}

void LandscapeRenderer::AddLandVertex(const glm::vec3 &position, const glm::vec2 &uv, const WorldTile *tile)
{
	const TerrainStyle *terrainStyle;

	if (tile->terrain == 255) {
		TerrainStyle plainTerrainStyle;

		terrainStyle = &plainTerrainStyle;
	} else {
		terrainStyle = &this->world->terrainStyles[tile->terrain];
	}	

	this->AddLandVertex(
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

void LandscapeRenderer::AddLandVertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &uv, int texture, const glm::vec4 &material)
{
	// glm::vec4 colour = GetColourFromLand(texture);

	this->landVertexBuffer->Add({
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

void LandscapeRenderer::InitialiseWaterShader()
{
	SafeDelete(this->waterShader);

	this->waterShader = OrcaShader::FromPath(
		"water.vert",
		this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_NONE ?
			"water.frag" : "land_wireframe.frag"
	);
	
	this->waterShaderUniform.projectionMatrix = this->waterShader->GetUniformLocation("ProjectionMatrix");
	this->waterShaderUniform.modelViewMatrix = this->waterShader->GetUniformLocation("ModelViewMatrix");
	this->waterShaderUniform.sphereRatio = this->waterShader->GetUniformLocation("InputSphereRatio");
	this->waterShaderUniform.cameraTarget = this->waterShader->GetUniformLocation("InputCameraTarget");

	if (this->waterVertexBuffer == NULL)
		this->waterVertexBuffer = new SimpleVertexBuffer<WaterVertex>();

	this->waterVertexBuffer->Initialise(this->waterShader, WaterShaderVertexInfo);
}

void LandscapeRenderer::RenderWater(const Camera *camera)
{
	if (this->debugRenderType != this->lastDebugRenderType)
		InitialiseWaterShader();

	// Bind water texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->waterTexture);

	// Activate the ocean shader and set inputs
	this->waterShader->Use();

	glUniformMatrix4fv(this->waterShaderUniform.projectionMatrix, 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));
	glUniformMatrix4fv(this->waterShaderUniform.modelViewMatrix, 1, GL_FALSE, glm::value_ptr(this->modelViewMatrix));
	glUniform1f(this->waterShaderUniform.sphereRatio, SphereRatio);
	glUniform3f(this->waterShaderUniform.cameraTarget, camera->target.x, camera->target.y, camera->target.z);

	this->SetLightSources(camera, this->waterShader);
	
	glUniform3fv(waterShader->GetUniformLocation("InputCameraPosition"), 1, glm::value_ptr(camera->eye));
	glUniform1f(waterShader->GetUniformLocation("iGlobalTime"), this->time);

	glUniform1i(this->waterShader->GetUniformLocation("InputTexture0"), 0);

	if (camera->viewHasChanged)
		UpdateWaterPrimitives(camera);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (this->debugRenderType != DEBUG_LANDSCAPE_RENDER_TYPE_NONE) {
		GLint uniformColour = this->waterShader->GetUniformLocation("uColour");

		glUniform4f(uniformColour, 0, 0, 0, 1);
		this->waterVertexBuffer->Draw(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, this->debugRenderType == DEBUG_LANDSCAPE_RENDER_TYPE_POINTS ? GL_POINT : GL_LINE);
		glUniform4f(uniformColour, 0, 0, 1, 1);
		this->waterVertexBuffer->Draw(GL_TRIANGLES);
	} else {
		this->waterVertexBuffer->Draw(GL_TRIANGLES);
	}
}

void LandscapeRenderer::UpdateWaterPrimitives(const Camera *camera)
{
	this->waterVertexBuffer->Clear();

	float viewSize = this->landViewSize;
	float translateX, translateZ;
	int landOriginX, landOriginZ;

	landOriginX = camera->target.x / World::TileSize;
	landOriginZ = camera->target.z / World::TileSize;
	translateX = camera->target.x - (landOriginX * World::TileSize);
	translateZ = camera->target.z - (landOriginZ * World::TileSize);

	float offsetsX[] = { 0, 0, 1, 0, 1, 1 };
	float offsetsZ[] = { 0, 1, 1, 0, 1, 0 };

	for (int landOffsetZ = -viewSize; landOffsetZ <= viewSize; landOffsetZ++) {
		for (int landOffsetX = -viewSize; landOffsetX <= viewSize; landOffsetX++) {
			int landX = this->world->TileWrap(landOriginX + landOffsetX);
			int landZ = this->world->TileWrap(landOriginZ + landOffsetZ);

			if (this->world->GetTile(landX + 0, landZ + 0)->height != 0 &&
				this->world->GetTile(landX + 0, landZ + 1)->height != 0 &&
				this->world->GetTile(landX + 1, landZ + 1)->height != 0 &&
				this->world->GetTile(landX + 1, landZ + 0)->height != 0
			) {
				continue;
			}

			// float lengthFromCentrePoint = glm::length(glm::vec2(landOffsetX, landOffsetZ));
			// if (lengthFromCentrePoint > viewSize)
			// 	continue;
			// 
			// float angle = todegrees(atan2(landOffsetZ, -landOffsetX));
			// float cameraAngle = wrapangledeg(camera->rotation - 90);
			// 
			// if (lengthFromCentrePoint > camera->zoom / 80.0f)
			// 	if (abs(smallestangledeltadeg(angle, cameraAngle)) > 90.0f)
			// 		continue;

			float vx = camera->target.x + (landOffsetX * World::TileSize) - translateX;
			float vz = camera->target.z + (landOffsetZ * World::TileSize) - translateZ;

			int height = this->world->GetTile(landX, landZ)->height;

			float uvSize = 1 / 2.0f;
			glm::vec2 baseUV = glm::vec2(landX, landZ) * uvSize;
			for (int i = 0; i < 6; i++) {
				this->waterVertexBuffer->Add({
					{ vx + offsetsX[i] * World::TileSize, 0, vz + offsetsZ[i] * World::TileSize },
					{ 0, 0, 0 },
					{ baseUV.s + offsetsX[i] * uvSize, baseUV.t + offsetsZ[i] * uvSize }
				});
			}
		}
	}

	this->waterVertexBuffer->Update();
}