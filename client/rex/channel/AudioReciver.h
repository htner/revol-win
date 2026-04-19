#pragma once

#include "macros.h"
#include "audio/samples.h"
#include "proto/IProto.h"
#include "proto/isession.h"
#include "audio/IAudioEngine.h"
#include "ChannelMemMisc.h"
#include <map>
#include <list>
#include "interface/channel/IAudioReciver.h"
#include "CriticalSectionAuto.h"


class IChannelView;
class IAudioSender;
class IVoiceStateMgr;

class AudioReciver : public xproto::IXObject
				   , public IXAudioEngineEvent
				   , public IAudioReciver
{
	typedef CHN_STD_LIST(HULI) USERLIST;
	typedef CHN_STD_MAP(HULI,LocalStringA) USERMAP;	
public:
	virtual void onSpeaker();
	virtual void onStopSpeaker();
	virtual void onStreamData(int seq, double bufferTime, double bufferLength);


	AudioReciver();
	virtual ~AudioReciver(void);
	void Init();
	void SetMute(BOOL Mute);
	BOOL IsMute();
public:
	virtual void SampleAudioData(const short* pCleanAudioData,	
		int nNumSample,
		int nSamplingFreq,
		int nChannels,
		int nVad);

	virtual  bool PlayAudioData(short* pPlayData, 
		int nNumSample,
		int nSamplingFreq,
		int nChannels);

	void OnAudioChanged(int eDev,double dfValue){}

	void getVoices();

	void UpdateVoiceUser();
	void OnShowVolume(int volume);

	
protected:			
	USERLIST  __broadcastList;
	USERLIST  __androidList;
	USERMAP   __voices;
	double                               __xTime;
	double                               __yTime;
	double                               __zTime;

	typedef enum __tagEDecodeType{
		E_DECODE_TYPE_DEFAULT = 0,
		E_DECODE_TYPE_BROADCAST = 1,
		E_DECODE_TYPE_ANDROID = 2
	}EDecodeType;

	void decode(protocol::session::XVoiceData* pvd,EDecodeType decodeType);

	UINT							__isMute;
	IChannelView*					__channelView;
	IVoiceStateMgr*					__voiceStateMgr;
	IAudioSender*					__audioSender;	
	unsigned int					__emptyCount;
	std::string						__tmp;
	std::string						__voiceDataBuf;
	UINT							__micUser;


//////////////////////////////////////////////////////////////////////////////////////
	int					__lastSeq;
	int					__delayCount;
	int                 __streamCount;
	double              __lastAdjustTime;
	int                 __protoSeq;
	double              __lastStreamTime;
	int                 __syncVideoLog;
	int                 __lateFac;
	int                 __earlyFac;
	int                 __netErrorCount;
	int                 __earlyCount;
	int                 __lateCount;

	CCriticalSection	__cs;

	struct SeqAndVoice{
		unsigned int seq;
		std::string payload;

	};

	std::list<SeqAndVoice* > __listSyncVideo;

	void pushForSyncVideo(int seq,const std::string& payload);
	void getForSyncVideo();
	void clearSyncData();
//////////////////////////////////////////////////////////////////////////////////////
};