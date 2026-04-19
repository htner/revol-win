#pragma once

#include <string>
#include "atlbase.h"
#include <common/xstring.h>
#include <map>
#include <list>


#define DEFAULT_SAMPLE_RATE 44100

class audio_output_thread;



struct DXEffect
{
	enum DXEMASK{
		DXE_NONE		= 0,
		DXE_GAIN		= 1 << 0,
		DXE_ATTACK		= 1 << 1,
		DXE_RELEASE		= 1 << 2,
		DXE_THRESHOLD	= 1 << 3,
		DXE_RATIO		= 1 << 4,
		DXE_PREDELAY	= 1 << 5,
		DXE_ALL			= 0xffff
	};
	DXEffect(){
		mask		= DXE_NONE;
		fGain		= 0.0f;
		fAttack		= 10.0f;
		fRelease	= 200.0f;
		fThreshold	= -20.0f;
		fRatio		= 3.0f;
		fPredelay	= 4.0f;
	}
	DWORD	mask;
	FLOAT   fGain;
    FLOAT   fAttack;
    FLOAT   fRelease;
    FLOAT   fThreshold;
    FLOAT   fRatio;
    FLOAT   fPredelay;
};


class  AudioOutput
{
private:
		AudioOutput();
		~AudioOutput();
		
public:
		static AudioOutput* inst();

		bool	setSoftVolume(UINT vol);
		UINT	getSoftVolume();

		bool	setSysVolume(UINT vol);
		UINT	getSysVolume();

		void	EnableEffect(DXEffect effect);
		void	DisableEffect();
		DXEffect getEffect();

		void	SetSysMute(bool isMute);
		bool	GetSysMute();
protected:
		UINT	GetMixerID();
		LONG					__nVolume;
		CRITICAL_SECTION		__cs;
		DXEffect				__effect;
};





