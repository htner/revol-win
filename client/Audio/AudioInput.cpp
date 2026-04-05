#include "atlbase.h"
#include "audio/audioinput.h"
#include "common/nsox/nulog.h"
#include "audio/samples.h"
#include "mixer.h"
#include "atlsync.h"
#include "common/xstring.h"
#include "audio/IAudioEngine.h"
#include "helper/ushell.h"




static bool g_Vista = helper::ushell::isWindowsVistaAbove();
void SelectWaveout(BOOL select, int nMixerID)
{		
		CMixer_Input mixer;
		if(!mixer.Create((InputDevice)IN_WAVEOUT,nMixerID))
		{
				return  ;
		}
		if(select){
				mixer.Select();			
		}
		
		if(!mixer.CanSelect())
			mixer.SetMute(!select);
		
		mixer.Close();			
}

void SetOutputMicrophone(bool open, int nMixerID)
{
	CMixer op;
	if(!op.Create(OUT_MICROPHONE,nMixerID))
	{				
		return ;
	}
	if(open){
		//if(op.GetVolume() < 20) op.SetVolume(50);
	}
	op.SetMute(!open);
	op.Close();
}

void SelectMicphone(bool select, int nMixerID)
{	
		CMicrophone mp;
		if(mp.Create(nMixerID)){
				if(select) 
					mp.Select();		

				if(!mp.CanSelect())
					mp.SetMute(!select);
				mp.Close();
		}				
}

AudioInput::AudioInput(){
		__nSoftwareVolume = 100;
}
AudioInput::~AudioInput()
{
}

AudioInput* AudioInput::inst()
{
		static AudioInput instance;
		return &instance;
}

UINT AudioInput::GetMixerID()
{
	UINT mixerID = 0;
	int waveInID = GetAudioEngine()->GetRecorderID();

	HWAVEIN hWaveIn;
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag      = WAVE_FORMAT_PCM ;
	waveFormat.nChannels       = 1;
	waveFormat.nSamplesPerSec  = 48000;
	waveFormat.wBitsPerSample  = 16;
	waveFormat.nBlockAlign     = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize          = 0 ;
	if(MMSYSERR_NOERROR == waveInOpen(&hWaveIn, waveInID, &waveFormat, 0, 0, CALLBACK_NULL))
	{
		if(MMSYSERR_NOERROR != mixerGetID((HMIXEROBJ)hWaveIn, &mixerID, MIXER_OBJECTF_HWAVEIN))
		{
			mixerID = 0;
		}

		waveInClose(hWaveIn);
	}

	return mixerID;
}
void	AudioInput::selectKalaok()
{
		int nMixerID = GetMixerID();
		//open wave out mix
		SelectWaveout(true,nMixerID);
		//open wave out microphone
		SetOutputMicrophone(true,nMixerID);
}

bool	AudioInput::canHardwareMixer()
{
		if(g_Vista)
		{
			return true;
		}

		int nMixerID = GetMixerID();
		CMixer_Input mixer;
		if(!mixer.Create((InputDevice)IN_WAVEOUT,nMixerID))
		{
			return  false;
		}

		mixer.Close();

		return true;
}

void	AudioInput::stopKalaok()
{
	return;
//	deviceGuid
		//close wave out microphone
		int nMixerID = GetMixerID();
		SelectMicphone(true,nMixerID);
		SelectWaveout(false,nMixerID);
		SetOutputMicrophone(false,nMixerID);
}

void	AudioInput::SetMicMute(bool isMute)
{
		int nMixerID = GetMixerID();

		CMicrophone mp;
		if(mp.Create(nMixerID)){
			mp.SetMute(isMute);
			mp.Close();
		}				
}

bool	AudioInput::GetMicMute()
{
		int nMixerID = GetMixerID();
		bool isMute = false;
		CMicrophone mp;
		if(mp.Create(nMixerID)){
			isMute = mp.GetMute();
			mp.Close();
		}			

		return isMute;
}


void	AudioInput::SetMicrophoneBoost(bool boost)
{
		CMicrophoneEnhance microphoneEnhance;
		microphoneEnhance.SetEnhance(boost);
}

bool AudioInput::setSysVolume(UINT volume)
{
	CMicrophone mp;
	int nMixerID = GetMixerID();
	if(mp.Create(nMixerID))
	{
		mp.SetVolume(volume);
		mp.Close();
		return true;
	}else
	{
		return false;
	}
}

UINT  AudioInput::getSysVolume(void)
{
	int volume = 100;
	int nMixerID = GetMixerID();
	CMicrophone mp;
	if(mp.Create(nMixerID))
	{
		volume = mp.GetVolume();
		mp.Close();
	}

	return volume;
}

bool  AudioInput::setSoftVolume(UINT vol)
{
	if(vol > 100)
		return false;
	__nSoftwareVolume = vol;

	return true;
}

UINT AudioInput::getSoftVolume()
{
	return __nSoftwareVolume;
}

