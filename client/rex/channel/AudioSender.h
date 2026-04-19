#pragma once

#include "audio/AudioInput.h"
#include "ChatSender.h"
#include "proto/IProto.h"
#include <set>
#include "../KeyHooker.h"
#include "proto/iconfig.h"
#include "listBuf.h"
#include "interface/channel/IAudioSender.h"
#include "interface/channel/IChannelView.h"
#include "interface/InterfaceQuery.h"
#include "AlertView.h"
#include "audio/ISoundChanger.h"



class NetMusicClient;
class WASLoopBack;
class ISecGain;
class IReverb;
class IChannelView;
class IVoiceStateMgr;
class IAudioMixer;
class IVideo;
NAMESPACE_USE(protocol::session)

class AudioSender : public IKeyEvent				  
				  , public xproto::IConfigEvent
				  , public ITimerSenseObj
				  , public IAudioSender
{	
public:
	AudioSender();
	virtual ~AudioSender(void);

	void Init();

	bool Start();
	void Stop(bool bForce=false);
	
	void OpenBackgound(bool bOpen,bool bIKALA = false);
	bool IsOpenBackgroud()const;

	void InitSpeakConfig();
	void OnKeyPress(std::set<int>& keys);	

	void MuteSelf(bool bMute);
	void StopVoice();
	void OnShowVolume(int volume);
	void SetQuality(uint32_t quality);
	uint32_t GetQuality(){return _qc;}

	inline void SetBroadCastChannel(const HULI &chId){
		_bcChId = chId;
	}
	inline int GetSampleType(){
		return _sampleType;
	}
	inline int GetSampleRate(){
		return _sampleRate;
	}
	inline int GetChannelCount(){
		return _numChannels;
	}

	virtual void on_capture_data(const char* p, UINT len);
	virtual	void on_capture_start();
	virtual void on_capture_stop();
	virtual void notifyPlayerProcID(DWORD procID,LPCTSTR lpProcName);
	virtual int changeVoiceEffect(int value = 0);
public://IKeyEvent	
	virtual void		OnKeyDown(int wKey);
	virtual void		ScanBegin();
	virtual void		ScanEnd();

public://xproto::IConfigEvent
	virtual void onConfigChanged();

protected:
	void sendAudioPack(const std::string& data, int volume);
	void sendAudio(std::string& data);
	void sendBroadcast(const std::string& data, int volume);
	void TimeArrive();
	void UpdateAudioMode(bool bForce = false);

	///////overlay function///////////////////////////////////////////////////////////////////
protected:
	void OnOLTalk2Channel(LPCTSTR lpMsg);	
	//////////////////////////////////////////////////////////////////////////
public:
	//from IAudioSender
	virtual int getSampleType();
	virtual int getSampleRate();
	virtual int getChannelCount();
	virtual void onPublish(const xstring& msg, 
				const xstring & token, 
				const xstring & deviceName, 
				int qualityType);
	virtual void onStopPublish();
	virtual BOOL isStart();

	virtual void push_audio_data(char* pData, UINT nLen);
public:	
	bool processMusicMode(std::string & data);
	void processSoundChanger(std::string & data);
protected:
	ISoundChanger* loadSoundChangerDLL();
	void unloadSoundChangerDLL();

	void loadVideo();
	void unloadVideo();

	std::set<int>	_keys;
	BOOL			_start;
	int				_delayStopCtr;
	int				_vadVolume;
	int				_nKey;
	BOOL			_isSpeaking;
	int				_speakMode;
	BOOL			_isKaraok;
	int				_sampleType;
	int				_sampleRate;
	BOOL			_isMute ;
	HULI			_bcChId;
	NetMusicClient*	_pNetMusic;
	int				_numChannels;
	bool			_isSoftwareMode;
	WASLoopBack*    _WASLoop;
	IChannelView*	_channel;
	IVoiceStateMgr* _voiceStateMgr;
	CListBuf        _listBuf;
	std::string     _broadcastBuf;
	std::string     _sampleBuf;
	std::string     _tempBuf;

	ISecGain		*_piSecGain;

	bool			_isCustomVAD;

	bool			_isReverb;
	IReverb			*_piReverb;
	bool			_isBackgound;
	std::vector<std::string> _listVoice;
	uint32_t		_qc;
	int             _stopCount;
	ISoundChanger*  _piSoundChannger;
	float           _soundChange;
	float           _soundChangeFac;
	int             _packCount;
	void			SaveAudio1(const std::string& data);
	void			SaveAudio2(const std::string& data);

	int				_volume,_volumeOld;
	bool			_voiceStateSpeeking,_voiceStateSpeekingOld;
	bool			_canSpeek;

	IVideo*			_piVideo;
	HMODULE         _hLibSoundChanger;
	HMODULE         _hLibVideo;

	IAudioMixer*    _musicMixer;
};

//////////////////////////////////////////////////////////////////////////
///////////////////CRcChatSender///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CRcChatSender : public CSendChatHelper
					, public ITimerSenseObj
{
public:
	CRcChatSender();
	~CRcChatSender();
public:
	virtual void AuthSendChat(const xstring& msg);
protected:
	virtual void InteranlSendChat(const xstring& textLocal,const xstring& textNet);
	virtual void SendText(IChannelView* __channel,IChannelDisplayView* view,TextChat& tc);
	virtual bool AuthChecker(IChannelView*,IChannelDisplayView*,LPCTSTR);
protected:
	virtual IChannelDisplayView* GetDisplayView();
public:
	virtual void TimeArrive();
public:
	void ResetState();

protected:	
	xstring		  __lastMsg;
	DWORD		  __lastMsgTick,__enterChTick;
	DWORD		  __curTick;
};

class CRcChannelChatSender : public CRcChatSender
{
public:
	CRcChannelChatSender();
	~CRcChannelChatSender();
protected:
	////////////////overlay funciton////////////////////////////////////
protected:
	void OnOLSendMsg2Channel(LPCTSTR lpMsg);
};

/************************************************************************/
/*                                                                      */
/************************************************************************/

class CRcMemberChatSender : public CRcChatSender
{
public:
	CRcMemberChatSender(){
	};
	~CRcMemberChatSender(){};
protected:
	virtual bool AuthChecker(IChannelView* __channel,IChannelDisplayView* view,LPCTSTR lpMsg);
	virtual void SendText(IChannelView* __channel,IChannelDisplayView* view,TextChat& tc);
protected:
	virtual IChannelDisplayView* GetDisplayView();
};
