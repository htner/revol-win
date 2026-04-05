#pragma once
#include <map>
#include <vector>
#include "protocol/psession.h"
#include "proto/isession.h"
#include "memMisc.h"
#include "helper/helper.h"
#define jitter_buffer_size 8

#define SHOW_S_TIME 60 * 1000

//#define NO_VOICE_STAT

#ifndef NO_VOICE_STAT
#include "VoiceDataStatistics.h"
#endif

#define PING_LOG_NUM 15


struct ChatVoiceData
{
	uint32_t seq;	
	uint32_t from;
	uint32_t sid;
	std::string chat;
};

namespace protocol
{
		namespace session{



				struct IVoicePumper ;
				class  VoiceDataManager;

				struct VoiceData {
					VoiceData(){reset(0);_packetCount = 1;__jitter = 0;}
					~VoiceData(){
						clearVoice();
					}
					bool _isUdp;
					UINT firstSeq;
					UINT currentSeq;
					UINT lastSeq;
					UINT firstFeedTime;
					UINT total;
					UINT totalLoss;
					UINT totalWait;
					UINT emptyTimes;
					UINT maxGitterTime;
					BOOL bstop;
					UINT firstPopTime;
					
					typedef std::vector<XVoiceData*,TempAllocator(XVoiceData*) > VoiceDataContainer;
					VoiceDataContainer voices;
					//OBJECT_LEAK_CHECK(VoiceData);

					VoiceDataContainer _packets;
					int  _packetCount;
					void setJitter(uint32_t jit);
					void parsePacket(XVoiceData* data);

					void reset(UINT fseq);
					void clearVoice();

					bool			fetch(XVoiceData** out);
					void			flush(IVoicePumper* pumper);
					UINT			push(PBaseSessVoice* vpak, bool isResend);
					bool			insert(PBaseSessVoice* vpak);
					bool			stop();

					double			GetPackTime();
					PChatVoice		__emptyVoice;
					bool			isUdp() {return _isUdp;}
					void			setUdp(bool b) {_isUdp = b;}
					VoiceType		getVoiceType();

					void set_voice_quality(IVoiceQuality* quality);
					IVoiceQuality* __voice_quality;
					uint32_t       __jitter;
					////////////////////////////////////////////////////////
#ifndef NO_VOICE_STAT
					bool			isNeedResend(uint32_t seq);
					void			log(bool);
					void			on_pack_loss(uint32_t seq);
					void			resend_error(uint32_t seq);
					void			pushEmpty(PEmptyChatVoice* vemptypak);
					VoiceType		voiceType;
					VoiceDataStatistics __statistics;
#endif
				};
			

				class VoiceDataManager
				{
					
				public:
					DEFINE_NEW_DELETE_OPERATOR();
					OBJECT_LEAK_CHECK(VoiceDataManager);					
						VoiceDataManager(void);
						~VoiceDataManager(void);

						void set_packet_notify(IPacketNotifyer* packetNotifyer);
						void set_voice_pumper(IVoicePumper* pumper);
						void set_voice_quality(IVoiceQuality* quality);

						void push(PBaseSessVoice* vpacket);
						void pushEmpty(PEmptyChatVoice* vemptypak);
						void stop(uint32_t uid);
						void clear();

						XVoiceData* fetch(int nTimeStamp, VoiceType vt);
						void releaseVoice(XVoiceData*);
						XVoiceData* fetchBroadcast(int nTimeStamp);						

						void pause();
						void start();
						void statistics();
						
						void getImmediateStatus(std::list<VoiceStatisticsData>* status);

						void  initJitter();
						void  calcJitter();
						virtual uint32_t getJitter();
						void SetRTT(uint32_t rtt);
						void SetSid(uint32_t sid);
						void StopAll();
				protected:
						void on_packet_loss(const UID_T& user, int fromseq, int toseq);
						XVoiceData* fetchVoice(/*voice_map& voices,*/int nTimeStamp, VoiceType vt);

				protected:
						uint32_t sid_;
						typedef std::map<uint32_t, VoiceData,std::less<uint32_t>, PairAllocator(uint32_t, VoiceData) > voice_map;
						voice_map		 __user_voices;		
						//voice_map		 __broastcast_voices;
						IPacketNotifyer* __packet_notify;
						IVoicePumper*    __voice_pumper;
						IVoiceQuality*	 __voice_quality;
						CRITICAL_SECTION __cs;
						bool			 __running;


						TimerHandler<VoiceDataManager>	__statisticsTimer;

						uint32_t					__lastPing_index;
						uint32_t					__lastPings[PING_LOG_NUM];
						uint32_t					ping_average_;
						uint32_t					__ping_counts;
						uint32_t					__jitter;				// 最近的抖動值
						DWORD						__last_report;			// 最后的匯報時間
						DWORD						__check_reportvoice;
						DWORD						__reportvoice_interval;
						std::list<int>				__last_valid_gets_percentage;
				};

		}
}

