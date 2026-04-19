#ifndef __INTERFACE_REVERB_H__
#define __INTERFACE_REVERB_H__

enum EReverbType{
	REVERB_TYPE_AUDITORIUM = 0,
	REVERB_TYPE_HALLWAY = 1,
	REVERB_TYPE_CAVE = 2,
	REVERB_TYPE_ALLEY = 3,
	REVERB_TYPE_HANGAR = 4,
	REVERB_TYPE_STONE_CORRIDOR = 5,
	MAX_REVERB_TYPE = 6,
};

class IReverb{
public:
	virtual ~IReverb(){};

	virtual bool SetReverb(EReverbType nReverbType) = 0;
	virtual bool Process(short* pwData,int nDataCount) = 0;
	virtual bool SetFac(float fFac) = 0;
};

#ifdef X_AUDIO_ENGINE_EXPORTS
#define X_AUDIO_ENGINE_API __declspec(dllexport)
#else
#define X_AUDIO_ENGINE_API __declspec(dllimport)
#endif

//extern "C" X_AUDIO_ENGINE_API IReverb* CreateReverb();
//extern "C" X_AUDIO_ENGINE_API void DestroyReverb(IReverb*);
extern "C" IReverb* CreateReverb();
extern "C" void DestroyReverb(IReverb*);

#endif //__INTERFACE_REVERB_H__
