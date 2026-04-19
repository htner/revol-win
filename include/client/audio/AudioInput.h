#pragma once

#include "windef.h"
#include <common/xstring.h>
#include <map>


class CMicrophone;
class  AudioInput
{
private:
		AudioInput();
		~AudioInput();
public:
		static AudioInput* inst();

		void	selectKalaok();
		void	stopKalaok();

		bool	setSysVolume(UINT vol);
		UINT	getSysVolume();

		bool	setSoftVolume(UINT vol);
		UINT	getSoftVolume();

		void	SetMicMute(bool isMute);
		bool	GetMicMute();

		void	SetMicrophoneBoost(bool boost);

		bool	canHardwareMixer();
protected:
		UINT	GetMixerID();
		bool					__stoping;

		UINT					__nSoftwareVolume;
};



