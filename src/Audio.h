#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS { namespace Audio {

enum SOUND_TYPE {
	SOUND_TYPE_COUNT
};

void Initialise();
void PlayMusic();
void PlaySound(SOUND_TYPE type);

} } }