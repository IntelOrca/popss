#pragma once

#include "LightSource.h"
#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class Camera;
class OrcaShader;
class LightManager {
public:
	LightSource natural;
	LightSource sun;

	void RegisterLightSource(const LightSource *lightSource);
	void UnregisterLightSource(const LightSource *lightSource);

	void SetLightSources(const Camera *camera, OrcaShader *shader);
	void SetLightSource(const Camera *camera, OrcaShader *shader, int index, const LightSource *light);
	
private:
	std::list<const LightSource*> lightSources;
};

} }