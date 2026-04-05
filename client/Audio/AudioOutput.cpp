#include "audio/iaudioresampler.h"
#include "audio/AudioOutput.h"
#include "common/nsox/nulog.h"
#include "audio/samples.h"
#include "mixer.h"
#include "ks.h"
#include "ksmedia.h"
#include "algorithm"
#include <math.h>
#include "audio/IAudioEngine.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
 AudioOutput::AudioOutput(){
		__nVolume = 100;

		__effect.mask = DXEffect::DXE_NONE;
}

 AudioOutput::~AudioOutput()
{
}

 AudioOutput* AudioOutput::inst()
{
		static AudioOutput instance;
		return &instance;
}

bool AudioOutput::setSoftVolume(UINT volume)
{
	if(volume > 100)
		return false;

	__nVolume = volume; 
	return true;
}

UINT AudioOutput::GetMixerID()
{
	UINT mixerID = 0;
	int waveOutID = GetAudioEngine()->GetPlayerID();
	
	HWAVEOUT hWaveOut;
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag      = WAVE_FORMAT_PCM ;
	waveFormat.nChannels       = 1;
	waveFormat.nSamplesPerSec  = 48000;
	waveFormat.wBitsPerSample  = 16;
	waveFormat.nBlockAlign     = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize          = 0 ;
	if(MMSYSERR_NOERROR == waveOutOpen(&hWaveOut, waveOutID, &waveFormat, 0, 0, CALLBACK_NULL))
	{
		if(MMSYSERR_NOERROR != mixerGetID((HMIXEROBJ)hWaveOut, &mixerID, MIXER_OBJECTF_HWAVEOUT))
		{
			mixerID = 0;
		}

		waveOutClose(hWaveOut);
	}

	return mixerID;
}

bool AudioOutput::setSysVolume(UINT vol)
{
	CMixer cm;
	int nMixerID = GetMixerID();
	if(cm.Create(OUT_SPEAKER,nMixerID))
	{
		cm.SetVolume(vol);
		cm.Close();
		return true;
	}else
		return false;
}

UINT AudioOutput::getSysVolume()
{
	CMixer cm;
	int nVolume = 100;
	int nMixerID = GetMixerID();
	if(cm.Create(OUT_SPEAKER,nMixerID))
	{
		nVolume = cm.GetVolume();
		cm.Close();
	}
	return nVolume;
}

void AudioOutput::SetSysMute(bool isMute)
{
	CMixer cm;
	int nMixerID = GetMixerID();
	if(cm.Create(OUT_SPEAKER,nMixerID))
	{
		cm.SetMute2(isMute);
		cm.Close();
	}
}

bool AudioOutput::GetSysMute()
{
	CMixer cm;
	bool isMute = false;
	int nMixerID = GetMixerID();

	if(cm.Create(OUT_SPEAKER,nMixerID))
	{
		isMute = cm.GetMute2();
		cm.Close();
	}

	return isMute;
}

UINT  AudioOutput::getSoftVolume(void)
{
	if( __nVolume == -1 )
	{
		return 100;
	}
 	return __nVolume;
}

void	AudioOutput::EnableEffect(DXEffect effect)
{
		__effect = effect;
}
void	AudioOutput::DisableEffect()
{
	__effect.mask = DXEffect::DXE_NONE;
}
DXEffect AudioOutput::getEffect()
{	
		return __effect;
}

