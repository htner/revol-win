#include "RTCEngine.h"

extern "C" struct IAudioEngine* GetAudioEngine()
{
	static RTCEngine e;
	return &e;
}