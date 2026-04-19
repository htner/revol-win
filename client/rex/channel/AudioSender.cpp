#include "stdafx.h"


#include <shlobj.h>
#include "../resource.h"

#include "common/nsox/nulog.h"
#include "AudioSender.h"
#include "speexencoder.h"

#include "audio/iaudiocodec.h"

#include "AudioReciver.h"
#include "audio/AudioOutput.h"
#include "helper/usystem.h"
#include "boostlite/format.hpp"
#include "audio/IAudioEngine.h"
#include "audio/AudioPacker.h"
#include "NetMusicClient.h"
#include "WASLoopBack.h"
#include "audio/ISecGain.h"
#include "audio/IReverb.h"
#include "../configmgr.h"
#include "../ConfigDefine.h"
#include "../SoundMgr.h"
#include "../rex_def.h"
#include "MP3Recorder.h"
#include "RcBbcodeLink.h"
#include "ProtoDriver.h"
#include "CharFormat.h"

#include "../../nxctrl/nxsourcebase.h"
#include "../../nxctrl/RichEditWrap.h"

#include "interface/InterfaceQuery.h"
#include "interface/channel/IChannelView.h"
#include "interface/IMsgHistory.h"
#include "../keywordFilter.h"

#include "olsrv/olservice.h"
#include "olsrv/inject.h"
#include "rdLogic/olVoiceHelper.h"
#include "rdLogic/AudioSender.inl"
#include "audio/IAudioMixer.h"
#include "ProcessInfo.h"
#include "video/IVideo.h"

extern int GetRcEnvConfigInt(LPCTSTR lpSesstion,LPCTSTR lpKey,int nDefValue);

#define REFRESH_MIN_TIME		1*1000
#define REFRESH_SAME_MIN_TIME	10*1000

#define SMILE_CHARCOUNT	8

const float AUDIO_UP_STEP  = 0.1f;
const float AUDIO_DOWN_STEP = 0.2f; 
int VOLUME_DEF_SOUND = 100;
int VOLUME_DEF_VOICE = 100;
using namespace protocol::session;

UINT SEQNO = 1000;

AudioSender::AudioSender()
{
    _start			= FALSE;
    _delayStopCtr	= 0;
    _vadVolume		= 30;
    _isSpeaking	= FALSE;
	_speakMode		= -1;
	_isKaraok		= FALSE;
	_isMute		= FALSE;

	_sampleType	= DEFAULT_AUDIO_TYPE;
	_sampleRate	= DEFAULT_AUDIO_RATE;

	_pNetMusic		= NULL;
	_numChannels		= 1;

	_WASLoop = NULL;
	_channel = NULL;
	_voiceStateMgr = NULL;

	_piSecGain = NULL;

	_isCustomVAD = true;	

	_piReverb = NULL;

	_isReverb = false;
	_isBackgound = false;
	_listVoice.clear();
	_stopCount = 0;
	_canSpeek = false;

	_qc = 0;
	INSTALL_INTERFACE(IAudioSender,IAudioSender_Name,this);
	OL_REG_AUSENDER_HADLE();
	
	_piSoundChannger = NULL;
	_hLibSoundChanger = NULL;
	_voiceStateSpeeking = false;
	_voiceStateSpeekingOld = false;
	_volume = _volumeOld = 0;

#ifdef REX_ENUS
	_packCount = 1;
#else 
	_packCount = 2;
#endif

	_packCount = GetRcEnvConfigInt(_T("RCAudio"), _T("PackCount"), _packCount);

	_hLibVideo = NULL;
	_piVideo = NULL;

	_musicMixer = NULL;
}

AudioSender::~AudioSender(void)
{
	OL_UNREG_AUSENDER_HADLE();
	UNINSTALL_INTERFACE(IAudioSender,IAudioSender_Name,this);

	if(_piVideo)
	{
		_piVideo->stopVideo();
		_piVideo->destroy();
		_piVideo = NULL;
	}

    _delayStopCtr = 0;

	if(_pNetMusic)
		delete _pNetMusic;

	if(_WASLoop)
		delete _WASLoop;

	CTimer::Instance()->UnRegisterListener(this);
	DestroySecGain(_piSecGain);

	DestroyReverb(_piReverb);

	ReleaseResampler(RCApp::Inst()->GetUid());

	if(_piSoundChannger)
		_piSoundChannger->destroy();

	_piSoundChannger = NULL;
	unloadSoundChangerDLL();

	if(_musicMixer)
		_musicMixer->destroy();

	unloadSoundChangerDLL();
}

void AudioSender::SaveAudio1(const std::string& data)
{
	static unsigned int s_numReadConfig = 0;
	static FILE * pf1 = NULL;
	static bool s_isWrite = false;
	s_numReadConfig ++;

	if(!data.empty() && 0 == s_numReadConfig % 200)
	{		
		s_isWrite = GetRcEnvConfigInt(_T("RCAudio"), _T("SaveAudio1"), 0);

		if(s_isWrite && !pf1)
			pf1 = fopen("audio1.pcm", "wb");
	}

	if(s_isWrite && pf1)
		fwrite(data.data(),1,data.size(), pf1);
}

void AudioSender::SaveAudio2(const std::string& data)
{
	static unsigned int s_numReadConfig = 0;
	static FILE * pf2 = NULL;
	static bool s_isWrite = false;
	s_numReadConfig ++;

	if(!data.empty() && 0 == s_numReadConfig % 200)
	{
		s_isWrite = GetRcEnvConfigInt(_T("RCAudio"), _T("SaveAudio2"), 0);

		if(s_isWrite && !pf2)
			pf2 = fopen("audio2.pcm", "wb");
	}

	if(s_isWrite && pf2)
		fwrite(data.data(),1,data.size(), pf2);
}

void AudioSender::UpdateAudioMode(bool bForce)
{
	if(nsox::nu_auto_ptr<xproto::IConfig> spConfig = CConfigMgr::Instance()->getConfig(CFG_SESSION_USER))
	{
		int mode = spConfig->getInt(USER_SpeakMode, 
			SM_KEYDOWN);

		int vadVolume = spConfig->getInt(USER_EnviromentNoisy, EVNNOISY_DEF);
		//_isCustomVAD = spConfig->getInt(USER_CustomSenstivity,0);							//����Ҫ�����l̎��ģ�Kȥ�ֱ��Ƿ����քӣ��@��RC�O��ȥ���ơ�

		audio::IAudioProcessor* processor = AudioUtil::GetProcessor(RCApp::Inst()->GetUid(), _sampleType);
		if(processor)
		{
			float fFac = 1.0;

			BOOL isAutoAudioEffect = spConfig->getInt(USER_AutoAudioEffect, TRUE);
			BOOL isMicEnarge = TRUE;
			BOOL isNS = TRUE;
			if(isAutoAudioEffect)
			{
				if(_sampleRate != 16000)
				{
					//isMicEnarge = FALSE;
					isNS = FALSE;
				}
			}else{
				isMicEnarge = spConfig->getInt(USER_MicEnlarge, TRUE);
				isNS = spConfig->getInt(USER_NoiseContrl, TRUE);
			}

			processor->SetRate(SPEEX_FRAME_SAMPLES(_sampleType, _sampleRate), _sampleRate, _numChannels);
			if(isMicEnarge)
			{
				fFac = 3.0;
				processor->EnableAGC(TRUE);	
			}
			else
				processor->EnableAGC(FALSE);

			processor->EnableNS(isNS);

			if(_piSecGain)
				_piSecGain->SetSecGain(fFac);
		}

		_isReverb = spConfig->getInt(USER_Reverberation, FALSE);
		int indexReverb = spConfig->getInt(USER_ReverberationIndex, 0); 
		int nFac = spConfig->getInt(USER_ReverberationVolumn, 50); 
		if(_isReverb && _piReverb)
		{
			_piReverb->SetReverb((EReverbType)indexReverb);
			_piReverb->SetFac((float)nFac/100.0f);
		}

		if( _speakMode == mode &&
			vadVolume == _vadVolume
			&& !bForce )
			return;

		_vadVolume = vadVolume;

		if(_speakMode != mode)
		{
			_speakMode = mode;
		}

		if(processor){
			if(mode == SM_FREE ||
				mode == SM_KARAOK || 
				mode == SM_BROADCAST)//free speaking
			{				
				processor->SetVADVolume(_vadVolume, _isCustomVAD);
				processor->OpenVAD();

				Start();
			}else if(mode == SM_KEYDOWN){
				processor->CloseVAD();
				_stopCount = 0;
				Stop();
			}
		}

	}
}


void AudioSender::SetQuality(uint32_t quality)
{
	if(_qc == quality)
		return;

	_qc = quality;
	int sampleRate,sampleType;
	if(quality){
		sampleType	= GET_SAMPLET_TYPE(quality);
		sampleRate	= GET_SAMPLET_RATE(quality);
		
		//sampleType	= quality >> 24;;
		//sampleRate	= quality & 0xffffff;
	}else
	{
		sampleType	= DEFAULT_AUDIO_TYPE;//audio::ACT_SPEEX;
		sampleRate	= DEFAULT_AUDIO_RATE;//SAMPLE_RATE_16000;
	}	

	if(sampleRate == 16000 && GetRcEnvConfigInt(_T("RCAudio"), _T("forceOpus"), 0))
	{
		sampleType = audio::ACT_OPUS_MONO;
	}

	int nChannels = 1;
	if(sampleType == audio::ACT_LAME_STEREO || sampleType == audio::ACT_CELT11_STEREO || \
		sampleType == audio::ACT_HEAAC4K || sampleType == audio::ACT_HEAAC || sampleType == audio::ACT_OPUS_STEREO)
		nChannels = 2;

	if( _sampleType != sampleType ||
		_sampleRate != sampleRate ||
		_numChannels != nChannels)
	{
		_sampleType = sampleType;
		_sampleRate = sampleRate;
		_numChannels = nChannels;

		if(_piSoundChannger)
		{
			_piSoundChannger->destroy();
			_piSoundChannger = NULL;
		}

		Stop();
		_listVoice.clear();
		_broadcastBuf.clear();
		_sampleBuf.clear();
		_listBuf.clear();

		UpdateAudioMode(true);
	}

}


void AudioSender::MuteSelf(bool bMute)
{
	_isMute = bMute;

	bool isStop = true;
	if( _speakMode == SM_KARAOK && _isSoftwareMode)
		isStop = false;

	if(_isMute && isStop)
	{
		StopVoice();		
	}
}

void AudioSender::Init()
{
	InterfaceQuery<IChannelView> channel(IChannelView_Name);
	_channel = channel;

	InterfaceQuery<IVoiceStateMgr> state(IVoiceStateMgr_Name);
	_voiceStateMgr = state;

	_piSecGain = CreateSecGain();
	

	_piReverb = CreateReverb();

	CTimer::Instance()->RegisterListener(this,40);

}

bool AudioSender::Start()
{
	if(!_channel)
		return false;
    
    if(!_channel->checkAudioPerm())
		return false;

    if(!_start && GetAudioEngine())
    {
        _delayStopCtr = 0;
        _start = TRUE;

        GetAudioEngine()->StartRecordForCPU();		

		CSoundMgr::Inst()->DoPlayEventSound(CSoundMgr::MICKEY_DOWN);
    }

	_stopCount = 3;
	return true;
}

void AudioSender::Stop(bool bForce/*=false*/)
{
	if(_isKaraok) return;  //karaok mode

	if(bForce || _stopCount <= 0)
	{
		if(_start){
			_start = FALSE;

			if(!_isBackgound /*&& !_rtmpVideo*/ && GetAudioEngine())
				GetAudioEngine()->StopRecordForCPU();

			OnShowVolume(0);
			StopVoice();	

			CSoundMgr::Inst()->DoPlayEventSound(CSoundMgr::MICKEY_UP);

			_voiceStateSpeeking = false;
			_volume = 0;
		}

	}else
		_stopCount --;
}

// init speakmode config
void AudioSender::InitSpeakConfig()
{
	//////////////////////////////////////////////////////////////////////////				//���MȺ�ĕr�������Z���O�á�
	if( nsox::nu_auto_ptr<xproto::IConfig> spConfig = CConfigMgr::Instance()->getConfig(CFG_SESSION_USER) ) 
	{
		DWORD dwKey_def;
		dwKey_def = spConfig->getInt(USER_HotKeySpeak_Def, 0);
		if ( dwKey_def != 0 )			//��ʼ������I���@����ϵ�y�O�õģ�def
			spConfig->setInt(USER_HotKeySpeak,dwKey_def);


		//dwKey_def = spConfig->getInt(USER_EnviromentNoisy_Def, -1);		//��ʼ�������Б���ʼֵ���@����ϵ�y�O�õģ�def
		//if(dwKey_def != -1)
		//	spConfig->setInt(USER_EnviromentNoisy,dwKey_def);

		dwKey_def = spConfig->getInt(USER_SpeakMode_Def,-1);					//���Iģʽ���@����ϵ�y�O�õ�,�����Б�ģʽ��def
		if(dwKey_def != -1)
		{
			spConfig->setInt(USER_SpeakMode,dwKey_def);	
			spConfig->setInt(USER_SpeakModeLast, dwKey_def);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	onConfigChanged();
}

void AudioSender::OnKeyPress(std::set<int>& keys)
{		
	static bool g_outputKey = GetRcEnvConfigInt(_T("debug"),_T("outputkey"),0);
	if( g_outputKey )
	{
		if(keys.size())
		{
			//OutputDebugString(_T("KeyPress: "));
			std::set<int>::iterator it = keys.begin();
			std::set<int>::iterator itEnd = keys.end();
			for (; it != itEnd; it++)
			{
				int k = *it;
				//xstring s = str(boost::wformat(_T("0x%02x,")) % k);
				//OutputDebugString(s.c_str());
			}
			//OutputDebugString(_T("\n"));
		}
	}

    nsox::nu_auto_ptr<xproto::IConfig> spConfig = CConfigMgr::Instance()->getConfig(CFG_SESSION_USER);
    if(spConfig)
    {
		int mode = spConfig->getInt(USER_SpeakMode, 
									SM_KEYDOWN);

		if(mode == SM_KEYDOWN){
			BOOL bVoiceSilent = spConfig->getInt(USER_MicSilent, 0);
			if (!bVoiceSilent)
			{
				// Speak
				if(KeyHooker::SingleKeyDown(_nKey, keys) && Start()/*_channel->checkAudioPerm()*/){
					//Start();
				}
				else{
					Stop();
				}
			}

		}
		BOOL bConfigChange = FALSE;

		int nKeyVolUp = spConfig->getInt(USER_HotKeyVolUp, 0);
		int nKeyVolDown = spConfig->getInt(USER_HotKeyVolDn, 0);
		int nKeyVolSilent = spConfig->getInt(USER_HotKeyVolSilent, 0);
		int nKeySndSilent = spConfig->getInt(USER_HotKeyMicSilent, 0);
        // volup
        if(nKeyVolUp && KeyHooker::MultiKeyDown(nKeyVolUp, keys))
        {
				int nVol = AudioOutput::inst()->getSoftVolume();
				nVol += 2;
				if(nVol < 0) nVol = 0;
				AudioOutput::inst()->setSoftVolume(nVol);
				spConfig->setInt(USER_PlayerVolume, nVol);
				bConfigChange = TRUE;
        }
        // voldown
        if(nKeyVolDown && KeyHooker::MultiKeyDown(nKeyVolDown, keys))
        {
				int nVol = AudioOutput::inst()->getSoftVolume();
				nVol -= 2;
				if(nVol > 100) nVol = 100;
				AudioOutput::inst()->setSoftVolume(nVol);
				spConfig->setInt(USER_PlayerVolume, nVol);
				bConfigChange = TRUE;
        }

		//Mute/UnMute Mic
		if(nKeySndSilent && KeyHooker::MultiKeyDown(nKeySndSilent, keys))
		{
			_isMute = !_isMute;
			spConfig->setInt(USER_MicSilent, _isMute);						
			bConfigChange = TRUE;

			if(_isMute)
			{
				StopVoice();				
			}
		}
		//Mute/UnMute Speaker sound
		if(nKeyVolSilent && KeyHooker::MultiKeyDown(nKeyVolSilent, keys))
		{
			BOOL bMuteSpeaker = spConfig->getInt(USER_PlayerSilent, 0);
			spConfig->setInt(USER_PlayerSilent, !bMuteSpeaker);
			bConfigChange = TRUE;
		}
		if (bConfigChange)
		{
			_channel->OnConfigChange(spConfig);
		}
    }	 
}

void AudioSender::onConfigChanged() //config changed
{
    if(nsox::nu_auto_ptr<xproto::IConfig> spConfig = CConfigMgr::Instance()->getConfig(CFG_SESSION_USER))
    {
		int nMusicMode = spConfig->getInt(USER_MusicMode, 2);
		_isSoftwareMode = false;
		switch(nMusicMode)
		{
		case MMS_AUTO:
			if(spConfig->getInt(USER_AutoMusicSoft, 1))
				_isSoftwareMode = true;
			break;
		case MMS_HARDWARE:

			break;
		case MMS_SOFTWARE:
			_isSoftwareMode = true;
			break;
		default:
			assert(0);
			break;
		}

		int nVolVoice = spConfig->getInt(USER_MicVolume, VOLUME_DEF_VOICE);
		int nVolSound = spConfig->getInt(USER_PlayerVolume, VOLUME_DEF_SOUND);
		//volume
		if (AudioInput::inst()->getSoftVolume() != nVolVoice)
			AudioInput::inst()->setSoftVolume(nVolVoice);
		if (AudioOutput::inst()->getSoftVolume() != nVolSound)
			AudioOutput::inst()->setSoftVolume(nVolSound);

		//Device
		if(GetAudioEngine())
		{
			GetAudioEngine()->SetPlayerID(spConfig->getInt(USER_PlayerDevice,0));
			GetAudioEngine()->SetRecorderID(spConfig->getInt(USER_MicDevice, 0));
		}

		// add hotkey
		int nKeyVolUp = spConfig->getInt(USER_HotKeyVolUp, 0);
		int nKeyVolDown = spConfig->getInt(USER_HotKeyVolDn, 0);
		int nKeyVolSilent = spConfig->getInt(USER_HotKeyVolSilent, 0);
		int nKeySndSndSilent = spConfig->getInt(USER_HotKeyMicSilent, 0);
		int mode = spConfig->getInt(USER_SpeakMode, SM_KEYDOWN);		
		int nVoiceSilent = spConfig->getInt(USER_MicSilent, 0);

		nKeyVolUp ? KeyHooker::MultiKeyAdd(nKeyVolUp) : 0;
		nKeyVolDown ? KeyHooker::MultiKeyAdd(nKeyVolDown) : 0;
		nKeyVolSilent ? KeyHooker::MultiKeyAdd(nKeyVolSilent) : 0;
		nKeySndSndSilent ? KeyHooker::MultiKeyAdd(nKeySndSndSilent) : 0;


		int nNewKey = spConfig->getInt(USER_HotKeySpeak, HOKEY_DEFAULT);
		if(_nKey != nNewKey)
		{
			_nKey = nNewKey;
			KeyHooker::SingleKeyAdd(_nKey);
		}

		
		_channel->OnConfigChange(spConfig);
		bool bForce = false;
		if (mode == SM_FREE || mode == SM_KARAOK)
			bForce = true;

		UpdateAudioMode(bForce);
		
    }
}

bool AudioSender::IsOpenBackgroud()const
{
	return _isBackgound;
}

void AudioSender::OpenBackgound(bool bOpen,bool bIKALA)
{
	if(_WASLoop)
	{
		WASLoopBack* WASLoop = _WASLoop;
		_WASLoop = NULL;
		WASLoop->Stop();
		delete WASLoop;
	}
	
	if( GetAudioEngine() )
	{
		_isBackgound = bOpen;
		if(!bOpen){
			AudioInput::inst()->stopKalaok();
			if(_pNetMusic)
			{
				delete  _pNetMusic ;
				_pNetMusic = NULL;
			}
		}else{
			if(bIKALA)
			{
				if(!_pNetMusic)
				{
					_pNetMusic = new NetMusicClient(RCApp::Inst()->GetMainWnd(),IKALA_NAME);

					_pNetMusic->SetRate(_sampleRate,_numChannels);
					_pNetMusic->Resume();
				}

				_isSoftwareMode = true;
			}

			if(_isSoftwareMode || g_isVistaAbove)
				AudioInput::inst()->stopKalaok();
			else
				AudioInput::inst()->selectKalaok();

			if(!_isSoftwareMode && g_isVistaAbove )
			{
				_WASLoop = new WASLoopBack;
				assert(_WASLoop);
				_WASLoop->Start(_sampleRate, _numChannels);
			}
		}

		if(_isBackgound /*|| _rtmpVideo*/)
			GetAudioEngine()->StartRecordForCPU();	
	}
}

void AudioSender::OnShowVolume(int volume)
{
	if( volume == 0 && _volumeOld == 0 )
		return;

	
	_channel->updateVolume(volume);	
	_volumeOld = volume;
	
}

void AudioSender::StopVoice()
{	
	_channel->stopVoice();

	SEQNO += 1500;

	_sampleBuf.clear();
}

void AudioSender::TimeArrive()
{
	_voiceStateMgr->pop_voice_state();
	_voiceStateMgr->pop_voice_volume();

	SessOperation::OpCode opCode = SessOperation::VOICE;
	if(_speakMode == SM_BROADCAST)
		opCode = SessOperation::BROADCAST;

	SessOperation op(opCode,_bcChId,RCApp::Inst()->GetUid());
	if(!_channel->canDo(&op))
		_canSpeek = false;
	else
		_canSpeek = true;

	if(_start == FALSE)
	{
		_voiceStateSpeeking = FALSE;
		_volume = 0;
	}

	OnShowVolume(_volume);

	if(_voiceStateSpeekingOld != _voiceStateSpeeking)
	{
		_channel->updateUserVoiceState(RCApp::Inst()->GetUid(),_voiceStateSpeeking);
		_voiceStateSpeekingOld = _voiceStateSpeeking;
	}

}

void AudioSender::loadVideo()
{
	if(_piVideo)
		return;

	if(_hLibVideo == NULL)
	{
#ifdef _DEBUG
		_hLibVideo = LoadLibrary(_T("XVideo_d.dll"));
#else 
		_hLibVideo = LoadLibrary(_T("XVideo.dll"));
#endif
		if(_hLibVideo == NULL)
			return;
	}

	typedef IVideo* createVideo_func();
	createVideo_func* createVideo_F = (createVideo_func*)GetProcAddress(_hLibVideo, "createVideo");
	if(createVideo_F)
		_piVideo = createVideo_F();
}

void AudioSender::unloadVideo()
{
	if(_piVideo)
	{
		_piVideo->stopVideo();
		_piVideo->destroy();
		_piVideo = NULL;
	}
	if(_hLibVideo)
		FreeLibrary(_hLibVideo);
	_hLibVideo = NULL;
}

ISoundChanger* AudioSender::loadSoundChangerDLL()
{
	if(_hLibSoundChanger)
		CloseHandle(_hLibSoundChanger);

	_hLibSoundChanger = LoadLibrary(_T("XSoundChanger.dll"));

	if(_hLibSoundChanger == NULL)
		return NULL;

	typedef ISoundChanger* createSoundChanger_func(int,int);
	createSoundChanger_func* createSoundChanger_f = (createSoundChanger_func*)GetProcAddress(_hLibSoundChanger, "createSoundChanger");

	if(createSoundChanger_f == NULL)
		return NULL;

	return createSoundChanger_f(_sampleRate, _numChannels);
}

void AudioSender::unloadSoundChangerDLL()
{
	if(_hLibSoundChanger)
		FreeLibrary(_hLibSoundChanger);
	_hLibSoundChanger = NULL;
}

int AudioSender::changeVoiceEffect(int v)
{
	_soundChangeFac = v;
	return _soundChangeFac;
}

typedef ISoundChanger* createSoundChanger_func(int sampleRate, int channels);
void AudioSender::processSoundChanger(std::string& data)
{
	if(_sampleRate != 48000)
		return;

	//int soundChange = GetRcEnvConfigInt(_T("RCAudio"), _T("soundchange"), 0);	
	int soundChange = _soundChangeFac;
	if(soundChange == 0)
	{
		_soundChange = soundChange;
		return;
	}

	if(_piSoundChannger == NULL)
		_piSoundChannger = loadSoundChangerDLL();

	if(_piSoundChannger == NULL)
		return;

	if(_soundChange != soundChange)
	{
		_piSoundChannger->setPitch((float)soundChange/10.0f);
		_soundChange = soundChange;
	}

	_piSoundChannger->pushData((unsigned char*)data.data(),data.length());

	int len = _piSoundChannger->getDataLen();
	if(_piSoundChannger->getDataLen() < data.size())
		return;

	_piSoundChannger->getProcessData((unsigned char*)data.data(),data.size());
}

void AudioSender::on_capture_data(const char* p, UINT len)
{
	if(!_channel)return;

	if(!_start && !_isBackgound /*&& !_rtmpVideo*/)
	{
		_voiceStateSpeeking = false;
		_volume = 0;
		_listVoice.clear();
		return;
	}

	if(_isMute &&  !_isBackgound /*&& !_rtmpVideo*/)	
	{
		_volume = 0; //OnShowVolume(0);
		_listVoice.clear();
		_voiceStateSpeeking = false;
		return;
	}
	else if(_isMute){
		memset((void*)p, 0, len);
	}

	std::string micData(p, len);

	SaveAudio1(micData);
	int isSpeech = SpeexEncoder::Process(RCApp::Inst()->GetUid(), micData, _sampleType, _sampleRate, _numChannels);

	if(_speakMode == SM_FREE && !_isBackgound /*&& !_rtmpVideo*/)
	{
		if(!isSpeech){
			_voiceStateSpeeking = false;
			_volume = 0;

			if(!_isSpeaking) 
				return;
			_isSpeaking = FALSE;
			OL_I_AM_TALKING(_isSpeaking);
			StopVoice();

			return;
		}
	}

	//sound changer
	processSoundChanger(micData);

	_isSpeaking = TRUE;
	OL_I_AM_TALKING(_isSpeaking);
	//music mode
	if(!processMusicMode(micData)) 
	{
		_voiceStateSpeeking = false;
		_volume = 0; //OnShowVolume(0);
		return;
	}
	
	
	if(!_canSpeek)
	{
		_voiceStateSpeeking = false;
		_volume = 0; //OnShowVolume(0);
		return ;
	}
	_voiceStateSpeeking = true;

	if (_sampleRate != 48000 && _piSecGain)
		_piSecGain->Process((short*)micData.data(), len/2);
 	
	if (_numChannels == 2 && _isReverb && _piReverb){
 		_piReverb->Process((short*)micData.data(), len/2);
 	}

	AudioUtil::ProcessVolume((short*)micData.data(), micData.size()/2, AudioInput::inst()->getSoftVolume());
	////////////////////////////////////////////////////////
	CMP3Recorder::GetInstance().PushData(RCApp::Inst()->GetUid(),micData);
	SaveAudio2(micData);
	
	sendAudio(micData);
}

void AudioSender::sendAudio(std::string& data)
{
	_volume = AudioUtil::CalcSoundVolume((short*)data.data(), data.size()/2);
	//OnShowVolume(volume);

	if(_speakMode == SM_BROADCAST &&  _sampleRate == 48000)
	{
		_sampleBuf.clear();
		sendBroadcast(data, _volume);
		return;
	}

	_broadcastBuf.clear();
	if(_sampleRate == 48000)
	{
		SpeexEncoder::Encode(RCApp::Inst()->GetUid(), _sampleType, _sampleRate, _numChannels, data);
		sendAudioPack(data, _volume);
		return;
	}

	assert(_sampleRate == 16000);
	_sampleBuf.append(data);

	int frameLen = SPEEX_PACKET_SIZE(_sampleType, _sampleRate);

	if(_sampleBuf.size() >= frameLen)
	{
		//assert(_sampleBuf.size() == frameLen);

		int sampleType = _sampleType;
		if(_speakMode == SM_BROADCAST)
			sampleType = audio::ACT_SPEEX;

		SpeexEncoder::Encode(RCApp::Inst()->GetUid(), sampleType, _sampleRate, _numChannels, _sampleBuf);
		sendAudioPack(_sampleBuf, _volume);
		_sampleBuf.clear();
	}
}

bool AudioSender::processMusicMode(std::string & data)
{
	if(!_isBackgound)
	{
		return true;
	}

	if(_musicMixer == NULL)
		_musicMixer = createAudioMixer();

	if(_musicMixer == NULL)
		return false;

	int len = data.size();
	std::string music;
	if(_isSoftwareMode)
	{
		if(_pNetMusic && _pNetMusic->ReadData(music, len)){
			if(!_isMute)
				_musicMixer->mix((short*)data.data(),(short*)music.data(), len / 2);			
			else
				data = music;
		}
	}
	else if(g_isVistaAbove && _WASLoop)
	{
		if(_WASLoop->GetData(music, len))
		{
			if(!_isMute)
				_musicMixer->mix((short*)data.data(),(short*)music.data(), len/2);					
			else 
				data = music;
		}
	}else{
		//�����XPϵ�y������ģʽ���K���o����ֱ�Ӳ��l�͔���
		if(_isMute)
			return false;
	}

	return true;
}

void AudioSender::sendAudioPack(const std::string& data,int  volume)
{
	if(_piVideo && _sampleRate == 48000 && _speakMode != SM_BROADCAST)
	{
		_piVideo->pushAudioData(data.data(),data.size());
		return;
	}

	std::string voice;

	_listVoice.push_back(data);
	if(_listVoice.size() < _packCount)
		return;

	audio::DefaultPacker pk;
	pk.pack(_listVoice, voice);


	SEQ_ADD(SEQNO);
	if(_speakMode == SM_BROADCAST){
		_channel->guanboVoice(voice, _bcChId, SEQNO, volume);
	}else{
		_channel->sendVoice(voice, SEQNO);
	}
	_listVoice.clear();
}

void AudioSender::on_capture_start()
{
}

void AudioSender::on_capture_stop()
{
	OnShowVolume(0);
	StopVoice();    
}

void AudioSender::notifyPlayerProcID(DWORD procID,LPCTSTR lpProcName)
{
	if(_pNetMusic)
		delete _pNetMusic;

	_pNetMusic = new NetMusicClient(RCApp::Inst()->GetMainWnd(),lpProcName);
	if(_pNetMusic)
	{
		Sleep(300);
		
		_pNetMusic->InjectDll(procID);
		_pNetMusic->SetRate(_sampleRate,_numChannels);
		_pNetMusic->Resume();
	}
}

void AudioSender::ScanBegin()
{
	_keys.clear();
}
void AudioSender::ScanEnd()
{
	OnKeyPress(_keys);
}
void AudioSender::OnKeyDown(int wKey)
{
	_keys.insert(wKey);

}

int AudioSender::getSampleType()
{
	return GetSampleType();
}
int AudioSender::getSampleRate()
{
	return GetSampleRate();
}
int AudioSender::getChannelCount()
{
	return GetChannelCount();
}

void AudioSender::onPublish(const xstring& msg, const xstring & token, const xstring & deviceName, int qualityType)
{
	xstring url = msg + _T("/mp4:") + token;
	//xstring url = msg + _T("/") + token;
	GetAudioEngine()->StartRecordForCPU();	
	if(_piVideo)
	{
		_piVideo->stopVideo();
		_piVideo->destroy();
		_piVideo = NULL;
	}

	loadVideo();
	if(_piVideo)
		_piVideo->startVideo(url.c_str(),deviceName.c_str(),qualityType);
}

void AudioSender::onStopPublish()
{
	if(_piVideo)
	{
		IVideo* v = _piVideo;
		_piVideo = NULL;
		v->destroy();
	}
}

BOOL AudioSender::isStart()
{
	return _start;
}

void AudioSender::push_audio_data(char* pData, UINT nLen)
{
	on_capture_data(pData,nLen);
}


void AudioSender::sendBroadcast(const std::string & data , int volume)
{
	int uid = RCApp::Inst()->GetUid();
	
	audio::IAudioResampler* resampler = AudioCreateResampler(uid);
	AudioUtil::Stereo2Mono((short*)data.data(), data.size()/2);
	resampler->SetRate(1, _sampleRate, 16000);
	std::string xx;
	_broadcastBuf.append(data.data(), data.size()/2);
	const int BROADCAST_BUF_LEN = 3840*FRAMES_PER_PACKET/2;

	while(_broadcastBuf.size() > BROADCAST_BUF_LEN)
	{
		if(!resampler->Resample(_broadcastBuf.data(), BROADCAST_BUF_LEN, xx))
		{
			assert(false);
			continue;
		}

		assert(xx.size() == 1280);
		_broadcastBuf.erase(0, BROADCAST_BUF_LEN);
		SpeexEncoder::Encode(uid, audio::ACT_SPEEX, 16000, 1, xx);

		sendAudioPack(xx,volume);
	}
}

////////////////////////CRcChatSender//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define THREAD_TIMER_SPAN	200
CRcChatSender::CRcChatSender()
{
	__lastMsgTick = 0;
	__enterChTick = 0;
	__curTick = 0;
	CTimer::Instance()->RegisterListener(this,THREAD_TIMER_SPAN);
}

CRcChatSender::~CRcChatSender()
{	
	CTimer::Instance()->UnRegisterListener(this);
}

void CRcChatSender::TimeArrive()
{
	__curTick += THREAD_TIMER_SPAN;
}

IChannelDisplayView* CRcChatSender::GetDisplayView()
{
	InterfaceQuery<IChannelDisplayView> view(IChannelDisplayView_Name);
	return view;
}
void CRcChatSender::AuthSendChat(const xstring& msg)
{
	InteranlSendChat(msg,msg);
}
void CRcChatSender::InteranlSendChat(const xstring& textLocal,const xstring& textNet)
{
	
	InterfaceQuery<IChannelView> __channel(IChannelView_Name);
	IChannelDisplayView* view = GetDisplayView();

	if(!__channel || !view){return;}

	if(textNet.empty())
		return;

	if(AuthChecker(__channel,view,textNet.c_str()))
	{

#ifndef _DEBUG		
		{			
			if(__curTick - __lastMsgTick <= REFRESH_MIN_TIME) {
				view->displayNotifyMsg(LTS_(IDS_NOTTOREFRESH));			
				return;
			}

			if(__lastMsg == textNet &&
				(__curTick - __lastMsgTick <= REFRESH_SAME_MIN_TIME)) {
					view->displayNotifyMsg(LTS_(IDS_NOTSENDSAMETEXT));
					return;
			}
			__lastMsgTick = __curTick;
			__lastMsg = textNet;
		}
		
#endif // _DEBUG

		_InputEditer->GetRichedit()->clear();

		CCharFormat cf;		
		_InputEditer->GetRichedit()->getDefaultCharFormat(cf);

		LOGFONT lf;
		DWORD clrText = 0;
		cf.GetLogFont(lf);
		TextChat tc;
		tc.msg = textNet;
		tc.setLogfont(lf);		
		tc.clr = cf.crTextColor;
		SendText(__channel,view,tc);
		UID_T uid = RCApp::Inst()->GetUid();
		view->displayChannelMsg(uid, __channel->getCurChannel(), 
								__channel->getUserNick(uid), textLocal, lf, tc.clr);
	}
}

void CRcChatSender::SendText(IChannelView* __channel,IChannelDisplayView* view,TextChat& tc)
{
	//�ڰl�͵ă��ݲ����^�V, �������@ʾ�ĕr���^�VUrl.
	//if(IChannelInfo* lpSessInfo = __channel->getSession()->getChannelInfo(__channel->getCurChannel()))
	//{
	//	if(lpSessInfo->getTxtLimitFormat() == protocol::session::tfl_urllimit 
	//		&& !__channel->isMember(RCApp::Inst()->GetUid()))
	//	{
	//		
	//		CUrlFilter xx;
	//		tc.msg = xx.filter(tc.msg);
	//	}
	//}

	LOGFONT lf = tc.toLogfont();
	ChannelRoler roler = NUL_ROLE;
	UID_T userid = RCApp::Inst()->GetUid();
	roler = (ChannelRoler)__channel->getUserRole( userid,__channel->getCurChannel());
	bool bManager = roler >= CMANAGER;
	bool bOnMic = __channel->getMicQueueUserId() == RCApp::Inst()->GetUid();
	if (!view->isBusy() || bManager || bOnMic)
		__channel->sendText(tc.msg, lf, tc.clr);
	__lastMsgTick = __curTick;
}


void CRcChatSender::ResetState()
{
	__lastMsg.clear();
	__lastMsgTick = __curTick;
	__enterChTick = __curTick;
}

bool CRcChatSender::AuthChecker(IChannelView* channel,IChannelDisplayView* view,LPCTSTR lpMsg)
{
		
	if( _tcslen(lpMsg) > MAX_CHANNEL_MSG_LEN )
	{
		xstring tip = LTS_(IDS_CHANNEL_MSG_SUPERMAX);
		view->displayNotifyMsg(tip.c_str());
		return false;
	}

	SessOperation op(SessOperation::TEXT);
	protocol::session::ISession* session = channel->getSession();
	if( !channel->canDo(&op) )
	{		 
		xstring tip;
		if ( session->getSessionList()->isUserTextableInCh(RCApp::Inst()->GetUid()))
		{
			tip = LTS_(IDS_CAN_NOT_TEXT);
		}
		else
		{
			tip = LTS_(IDS_TIP_TEXTLIMIT);
		}
		view->displayNotifyMsg(tip.c_str());
		return false;
	}
	HULI cid = channel->getCurChannel();
	ChannelRoler role = session->getSessionList()->getRoler( RCApp::Inst()->GetUid(), cid );
	if( role >= CMANAGER)
		return true;

	protocol::session::IChannelInfo* lpChannelInfo = session->getChannelInfo(cid);
	if( !lpChannelInfo )return false;

	protocol::session::TxtFormatLimit ltm = lpChannelInfo->getTxtLimitFormat(); 

	if( role <= MEMBER && 
		ltm == protocol::session::tfl_onlymanager)
	{
		view->displayNotifyMsg((LPCTSTR)(LTS_(IDS_ONLY_MANAGER_TEXT)));
		return false;
	}

	if ( role < MEMBER)
	{
		if ( ltm == protocol::session::tfl_vislimit)
		{
			view->displayNotifyMsg((LPCTSTR)(LTS_(IDS_LIMIT_GUEST_TEXT)));
			return false;
		}

		int nText = lpChannelInfo->getTxtMaxLength();
		int nSecondtWait = lpChannelInfo->getTxtInitWait();
		int nSecondLimitInterval = lpChannelInfo->getTxtInterval();
		if (nText > 0)
		{
			int	lgTxt = _InputEditer->GetRichedit()->getPureTextLen();
			lgTxt += SMILE_CHARCOUNT * _InputEditer->GetRichedit()->getObjCount();
			if( lgTxt <=0 )
				lgTxt = _tcslen(lpMsg);

			if( lgTxt <= 0 ) {
				return false;
			}else if (lgTxt > nText)
			{
				xstring msg = str( boost::wformat(LTS_(IDS_LIMITWORD)) % nText);
				view->displayNotifyMsg(msg.c_str());
				return false;
			}

		}

		if( nSecondtWait > 0)
		{
			DWORD dwTime = __curTick - __enterChTick;
			if( dwTime < nSecondtWait * 1000)
			{
				xstring msg = str( boost::wformat(LTS_(IDS_LIMITWAIT)) % nSecondtWait);
				view->displayNotifyMsg(msg.c_str());
				return false;
			}
		}

		if( nSecondLimitInterval > 0 ) 
		{
			if( __curTick - __lastMsgTick < nSecondLimitInterval * 1000 )
			{
				xstring msg = str( boost::wformat(LTS_(IDS_GUEST_SENDMSG_WAIT)) 
					% nSecondLimitInterval);
				view->displayNotifyMsg(msg.c_str());
				return false;
			}					
		}		
	}	
	return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CRcChannelChatSender::CRcChannelChatSender()
{
	
	OL_REG_CHATSENDER_HADLE();
}
CRcChannelChatSender::~CRcChannelChatSender()
{	
	OL_UNREG_CHATSENDER_HADLE();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool CRcMemberChatSender::AuthChecker(IChannelView* __channel,IChannelDisplayView* view,LPCTSTR lpMsg)
{
	ISession* sess = __channel->getSession();
	if(!sess){return false;}	
	{
		BOOL bPerm = __channel->canDo(&SessOperation(SessOperation::MEMBER_TEXT));
		if( !bPerm ) {
			CAlertView* pView = new CAlertView(IDS_NOT_MEMBER, IDS_TIP,
												CAlertView::AVF_BUTTONOK| CAlertView::AVF_ICONALERT);
			
			ModalMgr_DoModal(pView, dynamic_cast<nxctrl::IObject*>((HMSG)__channel->getMsgFilter()));
			return false;
		}
	}

	return true;
}

void CRcMemberChatSender::SendText(IChannelView* __channel,IChannelDisplayView* view,TextChat& tc)
{
	//save into db
	InterfaceQuery<IMsgHistory> history(IMsgHistory_Name);
	if(history){
		CCharFormat cf(tc.toLogfont(), tc.getColor());
		HULI uid = RCApp::Inst()->GetUid();
		GroupMsgUnit gmUnit;
		gmUnit._sid = __channel->getCurRealServerID();
		gmUnit._uid = uid;
		gmUnit._nick = __channel->getUserNick(uid);
		gmUnit._msg = CControlWordFilter::Filter(tc.msg);
		gmUnit._cf = cf;
		gmUnit._type = GroupMsgUnit::GT_NORMAL;
		gmUnit._time = time(NULL);
		history->InsertGroupMsg(gmUnit);
	}

	//send and display
	LOGFONT lf = tc.toLogfont();
	__channel->sendMemberText(tc.msg, tc.toLogfont(), tc.clr);	
}

IChannelDisplayView* CRcMemberChatSender::GetDisplayView()
{
	InterfaceQuery<IChannelMemberDisplayView> view(IChannelMemberDisplayView_Name);
	return view;
}
