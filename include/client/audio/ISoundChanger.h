#ifndef __I_SOUND_CHANGER_H__
#define __I_SOUND_CHANGER_H__
#include <string>

class ISoundChanger
{
public:
	/*pitch -12~12*/
	virtual void setPitch(float pitch) = 0; 
	virtual void pushData(const std::string& data) = 0;
	virtual void pushData(unsigned char*data, int len) = 0;
	virtual int getDataLen() = 0;
		virtual bool getProcessData(unsigned char* buf, int len) = 0;
	virtual void destroy() = 0;
};

#ifdef _SOUND_CHANGER_DLL_EXPORTS_
#define SOUNDCHANGER_API __declspec(dllexport)
#else 
#define SOUNDCHANGER_API __declspec(dllimport)
#endif

extern "C" SOUNDCHANGER_API ISoundChanger* createSoundChanger(int sampleRate, int channels);

#endif //__I_SOUND_CHANGER_H__
