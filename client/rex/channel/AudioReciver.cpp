#include "stdafx.h"
#include "audioreciver.h"
#include "audio/iaudioresampler.h"
#include "protocol/pmsg.h"
#include "audio/iaudiocodec.h"
#include "common/nsox/nulog.h"
#include "speexencoder.h"
#include "MP3Recorder.h"
#include "audio/AudioPacker.h"
#include <audio/AudioInput.h>
#include <audio/AudioOutput.h>

#include <vector>

#include "interface/InterfaceQuery.h"
#include "interface/channel/IAudioSender.h"
#include "interface/channel/IChannelview.h"

const int PACK_TIMES = 1;
//////////////////////////////////////////////////////////////////////////
const int MAX_PLAYER = 5;

extern int GetRcEnvConfigInt(LPCTSTR lpSesstion,LPCTSTR lpKey,int nDefValue);

class CUserVoiceStateMgr : public IVoiceStateMgr
{
public:
	DEFINE_NEW_DELETE_OPERATOR();
	OBJECT_LEAK_CHECK(CUserVoiceStateMgr);
	CUserVoiceStateMgr()
	{
		InterfaceQuery<IChannelView> channel(IChannelView_Name);
		__channel = channel;
		memset(__pushUserList, 0, sizeof(__pushUserList));
		memset(__playUserList, 0, sizeof(__playUserList));

		_voiceVolumeOld  = _voiceVolume = 0;
	}
	virtual ~CUserVoiceStateMgr(){

	}
	virtual void push_voice_state(const std::list<HULI>& users)
	{
		std::list<HULI>::const_iterator it = users.begin();
		int count = 0;
		memset(__pushUserList, 0, sizeof(__pushUserList));
		for(; it != users.end(); it++)
		{
			if(count >= MAX_PLAYER)
				break;

			__pushUserList[count++] = *it;
		}
	}
	virtual void pop_voice_state()
	{
		HULI tempUserList[MAX_PLAYER];
		memcpy(tempUserList, __pushUserList, sizeof(tempUserList));

		for(int i = 0; i < MAX_PLAYER; i ++)
		{
			HULI uid = __playUserList[i];
			if(uid == 0)
				break;

			for(int j = 0; j < MAX_PLAYER; j ++)
			{
				if(tempUserList[j] == uid)
					break;

				if(tempUserList[j] == 0 || j == MAX_PLAYER - 1)
				{
					__channel->updateUserVoiceState(uid,false);
					break;
				}
			}
		}

		memcpy(__playUserList, tempUserList, sizeof(tempUserList));
		for(int i = 0; i < MAX_PLAYER; i ++)
		{
			HULI uid = __playUserList[i];
			if(uid == 0)
				break;

			__channel->updateUserVoiceState(uid,true);
		}
	}
	virtual void push_voice_volume(int vol)
	{
		_voiceVolume = vol;
	};
	virtual void pop_voice_volume()
	{
		if( _voiceVolumeOld == 0 && _voiceVolume == 0 )
			return;

		{
			__channel->updateVolume(_voiceVolume,false);	
			_voiceVolumeOld  = _voiceVolume;
		}
	}

protected:
	int					_voiceVolume,_voiceVolumeOld;
	IChannelView*		__channel;
	HULI __pushUserList[MAX_PLAYER];
	HULI __playUserList[MAX_PLAYER];
};
//////////////////////////////////////////////////////////////////////////

AudioReciver::AudioReciver()
{
	__isMute			= FALSE;
	__channelView	= NULL;
	__audioSender	= NULL;	
	__voiceStateMgr = NULL;
	__emptyCount    = 0;
	__zTime         = 0;
	__xTime         = 0;
	__yTime         = 0;
	__micUser       = 0;
	__lastSeq		= 0;
	__delayCount	= 0;
	__lastAdjustTime = 0;
	__lastStreamTime = 0;
	__streamCount = 0;
	__lateFac = 0;
	__earlyFac = 0;
	__netErrorCount = 0;
	__earlyCount = 0;
	__lateCount = 0;

	__syncVideoLog = GetRcEnvConfigInt(_T("RCAudio"), _T("SyncVideoLog"), 0);
	INSTALL_INTERFACE(IAudioReciver,IAudioReciver_Name,this);
}

AudioReciver::~AudioReciver(void)
{	
	UNINSTALL_INTERFACE(IVoiceStateMgr,IVoiceStateMgr_Name,__voiceStateMgr);
	if( __voiceStateMgr )
		delete  __voiceStateMgr ;

	clearSyncData();
	UNINSTALL_INTERFACE(IAudioReciver,IAudioReciver_Name,this);

}

void AudioReciver::Init()
{
	InterfaceQuery<IChannelView> channel(IChannelView_Name);
	__channelView = channel;

	InterfaceQuery<IAudioSender> sender(IAudioSender_Name);
	__audioSender = sender;

	__voiceStateMgr = new CUserVoiceStateMgr;
	INSTALL_INTERFACE(IVoiceStateMgr,IVoiceStateMgr_Name,__voiceStateMgr);
}

void AudioReciver::onSpeaker()
{
	if(__micUser != 0)
	{
		__channelView->updateUserVoiceState(__micUser,false);
		__micUser = 0;

		clearSyncData();
	}

	HULI micUser = __channelView->getMicQueueUserId();

	if(micUser != __micUser)
	{
		__earlyCount = 0;
		__lateCount = 0;
		__lastAdjustTime = 0;
		__lastSeq = 0;
	}

	__micUser = micUser;
	__channelView->updateUserVoiceState(__micUser,true);
}

void AudioReciver::onStopSpeaker()
{
	__channelView->updateUserVoiceState(__micUser,false);
	__micUser = 0;
	__lastSeq = 0;
	__lastAdjustTime = 0;
	__earlyCount = 0;
	__lateCount = 0;
	clearSyncData();
}


void AudioReciver::onStreamData(int seq, double bufferTime, double bufferLength)
{
	pubfunc::CCriticalSectionAuto lock(&__cs);
	char arrBuf[512] = {0};


	if(__syncVideoLog)
	{
		sprintf(arrBuf, "receive onStreamData,seq = %d, __streamCount = %d,  bufferLength = %f  \n" ,seq ,__streamCount,bufferLength);
		::OutputDebugStringA(arrBuf);
	}

	if(bufferLength > 20)
	{
		if(__syncVideoLog)
		{
			sprintf(arrBuf, "Error BufferLength bufferLength = %f \n", bufferLength);
			::OutputDebugStringA(arrBuf);
		}
		return;
	}

	if(__lastStreamTime > 0 && (timeGetTime() - __lastStreamTime > 2 *1000))
	{
		__lastStreamTime = timeGetTime();

		__netErrorCount ++;
		__lastAdjustTime = timeGetTime() + (30 * 1000 * __netErrorCount);
		__streamCount = 50;

		if(__syncVideoLog)
		{
			sprintf(arrBuf, "receive streamData is too late -- > __netErrorCount = %d ...... \n", __netErrorCount);
			::OutputDebugStringA(arrBuf);
		}
		return;
	}

	__lastStreamTime = timeGetTime();
	if(__streamCount-- > 0)
		return;
	__streamCount = 0;

	__lastStreamTime = timeGetTime();
	if(__lastSeq == seq)
	{
		if(__syncVideoLog)
		{
			::OutputDebugStringA("onStreamData __lastSeq == seq  \n");
		}
		return;
	}

	__lastSeq = seq;

	if(timeGetTime() < __lastAdjustTime)
	{
	//	::OutputDebugStringA("have no adjust .....");
		return;
	}

	int currentSeq = __protoSeq;
	if(!__listSyncVideo.empty())
	{
		currentSeq = (*__listSyncVideo.begin())->seq;
	}

	if(currentSeq <= 0)
	{
		if(__syncVideoLog)
		{
			::OutputDebugStringA("onStreamData currentSeq <= 0  \n");
		}

		return;
	}

	const static double TIME_OF_AAC = 1024.0 * 8 / (48000 * 2 * 2); 
	double delay = (double)(currentSeq - __lastSeq) * TIME_OF_AAC /*+ bufferLength*/;

	if(__syncVideoLog)
	{
		if(__syncVideoLog)
		{
			sprintf(arrBuf, "delay = %f , __listSyncVideo.size = %d, currentSeq = %d, __lastSeq = %d, bufferLength = %f \n ", 
			delay,__listSyncVideo.size(),currentSeq, __lastSeq, bufferLength );
			::OutputDebugStringA(arrBuf);
		}
	}

	if(delay > 1.0)
	{
		if(__listSyncVideo.size() >= 1000)
		{
			if(__syncVideoLog)
			{
				::OutputDebugStringA("onStreamData  __listSyncVideo.size() >= 100 \n");
			}
			return ;
		}

		__lateFac ++;
		__earlyFac = 0;

		if(__syncVideoLog)
		{
			sprintf(arrBuf, "delay > 1.0,  delay = %f , __lateFac = %d , __earlyFac = %d \n ", delay, __lateFac ,__earlyFac);
			::OutputDebugStringA(arrBuf);
		}

		if(__lateFac > 10)
		{
			if(__syncVideoLog)
			{
				::OutputDebugStringA("delay > 1.0 .....adjust...............................\n");
				::OutputDebugStringA("......................................................\n");
				::OutputDebugStringA("......................................................\n");
				::OutputDebugStringA("......................................................\n");
			}

			__lateFac = 0;
			__delayCount = delay/TIME_OF_AAC;
			__lateCount++;
			__lastAdjustTime = timeGetTime() + (30 * 1000 * __lateCount);
		}
		
	}else if (delay < -1.0)
	{
		__lateFac  = 0;
		__earlyFac++;

		if(__syncVideoLog)
		{
			sprintf(arrBuf, "delay < -1.0,  delay = %f , __lateFac = %d , __earlyFac = %d \n ", delay, __lateFac ,__earlyFac);
			::OutputDebugStringA(arrBuf);
		}

		if(__earlyFac > 10 )
		{
			__earlyFac = 0;

			if(__earlyCount++ > 10)
				__earlyCount = 10;

			__lastAdjustTime = timeGetTime() + (60 * 1000 * __earlyCount);
			int deleteCount = -delay * 22.5;

			if(__syncVideoLog)
			{
				sprintf(arrBuf, "...............__earlyCount = %d .................\n",__earlyCount);
				::OutputDebugStringA("delay < -1.0.....adjust...............................\n");
				::OutputDebugStringA(arrBuf);
				::OutputDebugStringA("......................................................\n");
				::OutputDebugStringA("......................................................\n");
			}

			if(__listSyncVideo.size() <= 1)
			{
				deleteCount -= 11;
				for(int i = 0; i < deleteCount; i ++)
				{
					protocol::session::XVoiceData* pvd = __channelView->getSession()->fetchVoices(protocol::session::using_chaninfo);

					if(pvd == NULL)
					{
						break;
					}

					if(__syncVideoLog)
					{
						sprintf(arrBuf, "onStreamData getFetch deleteCount = %d, i = %d \n ",deleteCount, i);
						::OutputDebugStringA(arrBuf);
					}

					__channelView->getSession()->releaseVoice(pvd);
				}
			}else{
				int i = 0;
				for(int i = 0; i < deleteCount; i ++)
				{
					if(__listSyncVideo.empty())
						break;

					SeqAndVoice* v = *__listSyncVideo.begin();
					delete v;
					__listSyncVideo.pop_front();
				}

			}
		}
	}else{
		__lateFac  = 0;
		__earlyFac = 0;
	}

	if(timeGetTime() - __lastAdjustTime > 1000*60*5)
	{
		if(__earlyCount > 1)
			__earlyCount = 1;
		if(__lateCount > 1)
			__lateCount = 1;
	}
}

void AudioReciver::clearSyncData()
{
	pubfunc::CCriticalSectionAuto lock(&__cs);
	while(!__listSyncVideo.empty())
	{
		SeqAndVoice* v = *__listSyncVideo.begin();
		delete v;
		__listSyncVideo.pop_front();
	}
}

void AudioReciver::pushForSyncVideo(int seq,const std::string &payload)
{
	pubfunc::CCriticalSectionAuto lock(&__cs);

	SeqAndVoice* v = new SeqAndVoice();
	v->seq = seq;
	v->payload = payload;
	__protoSeq = seq;

	__listSyncVideo.push_back(v);
}

void AudioReciver::getForSyncVideo()
{
	pubfunc::CCriticalSectionAuto lock(&__cs);

	__delayCount --;
	if(__listSyncVideo.empty() || __delayCount > 0)
		return;

	__delayCount = 0;
	SeqAndVoice* v = *__listSyncVideo.begin();
	int currentSeq = v->seq;
	unsigned int current = timeGetTime();

	SpeexEncoder::Decode(__micUser, __audioSender->getSampleType(),__audioSender->getSampleRate(), __audioSender->getChannelCount(),v->payload);
	LocalStringA& vvv = __voices[__micUser];
	vvv.append(v->payload.c_str(),v->payload.size());

	__listSyncVideo.pop_front();
	delete v;
}

void AudioReciver::SetMute(BOOL mute){
	if (mute == __isMute)
		return ;

	__isMute = mute;
}

BOOL AudioReciver::IsMute()
{
	return __isMute;
}

void AudioReciver::SampleAudioData(const short* pCleanAudioData,	
								   int nNumSample,
								   int nSamplingFreq,
								   int nChannels,
								   int nVad
								   )
{
	__audioSender->push_audio_data((char*)pCleanAudioData, nNumSample * sizeof(short)*nChannels);
}

void AudioReciver::getVoices()
{

	const double TIME_OF_AAC = 1024.0 * 8 * 1000 / (48000 * 2 * 2); 
	const double TIME_OF_SPEEX =  20.0 * FRAMES_PER_PACKET; 

	double callbackTime = 20;
	if(__audioSender->getChannelCount() == 2)
		callbackTime = TIME_OF_AAC;
	__zTime += callbackTime;

	//process broadcast and android
	while(__zTime > __xTime)
	{
		__xTime += (TIME_OF_SPEEX * PACK_TIMES);
		protocol::session::XVoiceData* pVoiceBroadcast = __channelView->getSession()->fetchVoices(protocol::session::using_broadcast);
		protocol::session::XVoiceData* pVoiceAndroid = __channelView->getSession()->fetchVoices(protocol::session::using_type);

		if( pVoiceBroadcast == NULL )
		{
			if(!__broadcastList.empty())
			{
				USERMAP::iterator item = __voices.find(*__broadcastList.begin());
				if(item != __voices.end())
				{
					//ReleaseResampler(item->first);
					__voices.erase(item);
				}

				__broadcastList.pop_front();
			}
		}else{
			__broadcastList.clear();		
			for(protocol::session::XVoiceData* p = pVoiceBroadcast; p; p = p->next)
			{
				__broadcastList.push_back(p->user);
			}

			decode(pVoiceBroadcast, E_DECODE_TYPE_BROADCAST);
			__channelView->getSession()->releaseVoice(pVoiceBroadcast);
		}

		if(pVoiceAndroid == NULL)
		{
			if(!__androidList.empty())
			{
				USERMAP::iterator item = __voices.find(*__androidList.begin());
				if(item != __voices.end())
				{
					//ReleaseResampler(item->first);
					__voices.erase(item);
				}

				__androidList.pop_front();
			}
		}else{
			__androidList.clear();
			for(protocol::session::XVoiceData* p = pVoiceAndroid; p; p = p->next)
			{
				__androidList.push_back(p->user);
			}


			decode(pVoiceAndroid, E_DECODE_TYPE_ANDROID);

			__channelView->getSession()->releaseVoice(pVoiceAndroid);
		}
	}

	while(__zTime > __yTime)
	{				
		if(__audioSender->getChannelCount() == 2)
			__yTime += (TIME_OF_AAC * PACK_TIMES);
		else
			__yTime += (TIME_OF_SPEEX * PACK_TIMES); 

		protocol::session::XVoiceData* pvd = __channelView->getSession()->fetchVoices(protocol::session::using_chaninfo);

		if(pvd == NULL)
		{
			continue;
		}

		decode(pvd, E_DECODE_TYPE_DEFAULT);
		__channelView->getSession()->releaseVoice(pvd);
		return;
	}
}
void AudioReciver::OnShowVolume(int volume)
{
	__voiceStateMgr->push_voice_volume(volume);	
}

bool AudioReciver::PlayAudioData(short* pPlayData, 
								 int nNumSample,
								 int nSamplingFreq,
								 int nChannels
								 )
{
	getVoices();
	getForSyncVideo();
	UpdateVoiceUser();

	if(__voices.size() == 0)
	{
		OnShowVolume(0);

		__zTime = __xTime = __yTime = 0;
		return false;
	}

	__emptyCount = 0;

	USERMAP::iterator it = __voices.begin();

	CHN_STD_VECTOR(int) vecData;
	vecData.resize(nNumSample * nChannels);
	while(it != __voices.end())
	{
// 		if(it->first)
// 		{
// 			NoticeVoiceDisplay( it->first , 1);
// 		}

		int frameSize = nNumSample * nChannels;
		if(it->second.size() < frameSize  * 2)
		{
			it = __voices.erase(it);
			continue;
		}

		LocalStringA voiceData;
		voiceData.assign(it->second.data(), frameSize  * 2);
		it->second.erase(0, frameSize * 2);
		if(it->second.empty())
		{
			//ReleaseResampler(it->first);
			it = __voices.erase(it);
		}
		else
		{
			it++;
		}

		int nVoiceSize = voiceData.size()/sizeof(short);
		if(nVoiceSize == nNumSample * nChannels)
		{
			short* pwVoice = (short*)(voiceData.data());
			for(int i = 0; i < nNumSample * nChannels; i++)
			{
				if( i < nVoiceSize )
				{
					vecData[i] += *pwVoice++;
				}
			}
		}

	}

	if(IsMute())
	{
		OnShowVolume(0);
		return true;
	}

	int nMax = 0;
	float fFactor = 1.0f;
	for (int i = 0; i < nNumSample * nChannels; i++)
	{
		if (abs(vecData[i])>nMax)
			nMax = abs(vecData[i]);
	}

	if (nMax>32767.f)
	{
		fFactor = 32767.f/nMax;
	}

	extern short dbl2s(double);
	for(int i = 0; i < nNumSample * nChannels; i++ )
	{
		pPlayData[i] = dbl2s(fFactor*vecData[i]);
	}

	AudioUtil::ProcessVolume(pPlayData, nNumSample * nChannels, AudioOutput::inst()->getSoftVolume());

	if(CMP3Recorder::GetInstance().IsRecording())
	{
		std::string recData;  recData.assign((char*)pPlayData, nNumSample * nChannels * 2);
		CMP3Recorder::GetInstance().PushData(0x11111111, recData);
	}

	int volume = AudioUtil::CalcSoundVolume(pPlayData, nNumSample * nChannels);

	OnShowVolume(volume);

	return true;
}


// void AudioReciver::NoticeVoiceDisplay(unsigned int uid, bool isPlay)
// {
// 	__voiceStateMgr->push_voice_state(uid,isPlay);
// }

void AudioReciver::UpdateVoiceUser()
{
	std::list<HULI> users;
	USERMAP::iterator itUsersID = __voices.begin();
	for(;itUsersID != __voices.end(); itUsersID++)
	{
		users.push_back(itUsersID->first);
	}
	__voiceStateMgr->push_voice_state(users);
	
}

void AudioReciver::decode(protocol::session::XVoiceData* pvd,EDecodeType decodeType)
{	
	for(protocol::session::XVoiceData* it = pvd; it != NULL;it = it->next)
	{
		if(it->seq == -1)
		{
			ReleaseResampler(it->user);
			USERMAP::iterator xx = __voices.find(it->user);
			if(xx != __voices.end())
				__voices.erase(__voices.find(it->user));
		}else{
			__voiceDataBuf.assign(it->payload,it->payloadsize);		

			if(E_DECODE_TYPE_BROADCAST == decodeType)
			{
				SpeexEncoder::Decode(it->user, audio::ACT_SPEEX,16000, 1,__voiceDataBuf);

				if(__audioSender->getChannelCount() == 2)
				{
					audio::IAudioResampler* resampler = AudioCreateResampler(it->user);
					resampler->SetRate(1, 16000, __audioSender->getSampleRate());
					resampler->Resample(__voiceDataBuf.data(), __voiceDataBuf.size(), __tmp);
					AudioUtil::Mono2Stereo((short*)__tmp.data(),__tmp.size()/2, __voiceDataBuf);
				}

				LocalStringA& vvv = __voices[it->user];
				vvv.append(__voiceDataBuf.c_str(),__voiceDataBuf.size());
			}
			else if(E_DECODE_TYPE_DEFAULT == decodeType)
			{
				if(it->user == __micUser && __lastSeq > 0)
				{
					pushForSyncVideo(it->seq, __voiceDataBuf);
				}else{
					SpeexEncoder::Decode(it->user, __audioSender->getSampleType(),__audioSender->getSampleRate(), __audioSender->getChannelCount(),__voiceDataBuf);
					LocalStringA& vvv = __voices[it->user];
					vvv.append(__voiceDataBuf.c_str(),__voiceDataBuf.size());
				}
			}
			else if(E_DECODE_TYPE_ANDROID == decodeType)
			{
				SpeexEncoder::Decode(it->user, audio::ACT_SPEEX,8000, 1,__voiceDataBuf);

				audio::IAudioResampler* resampler = AudioCreateResampler(it->user);
				resampler->SetRate(1, 8000, __audioSender->getSampleRate());
				resampler->Resample(__voiceDataBuf.data(), __voiceDataBuf.size(), __tmp);
				if(__audioSender->getChannelCount() == 2)
					AudioUtil::Mono2Stereo((short*)__tmp.data(),__tmp.size()/2, __voiceDataBuf);
				else
					__voiceDataBuf = __tmp;

				LocalStringA& vvv = __voices[it->user];
				vvv.append(__voiceDataBuf.c_str(),__voiceDataBuf.size());
			}
			else
			{
				assert(0);
			}
		}
	
	}
}

