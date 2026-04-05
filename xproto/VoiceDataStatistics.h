#pragma once

#include "common/nsox/int_types.h"
#include "protocol/const.h"
#include "client/proto/isession.h"


#define MAX_STAT_VECTOR 500
#define MAX_JUMP_VECTOR 400
#define SHOW_PRE	20
#define SHOW_COUNT	128

class VoiceDataStatistics
{
public:

	enum PACKET_STATE{
		e_s_init,		// 初始化
		e_s_loss,		// 丢失
		e_s_resend,		// 重传中
		e_s_succ,		// 一次成功
		e_s_resend_succ,	// 重传成功
		e_s_resend_excess,	// 额外的重传
		e_s_resend_late,	// 重传太迟
		e_s_resend_error,	// 重传失败
		e_s_real_loss,		// 丢失
	};

	struct voiceState {
		bool		is_udp_;
		PACKET_STATE __states;
		uint32_t __seq;					//當前的seq
		uint32_t __emptyTtimes;			//為等待當前包,語音隊列補空包次數
		uint32_t __emptyPakSeq;			//記錄服務器通知無語音
		voiceState() {
			reset();
		};
		void reset() {
			is_udp_ = false;
			__states = e_s_init;
			__seq = 0;
			__emptyTtimes = 0;			
			__emptyPakSeq = 0;				
		};
	};

	VoiceDataStatistics();
	~VoiceDataStatistics();

	void			flush(uint32_t seq, bool isLoss, bool isResend = false);
	void			flush(uint32_t seq, PACKET_STATE s);
	void			empty(uint32_t seq);
	void			log(bool tolog, uint32_t currect_seq);
	uint32_t		getPos(uint32_t seq);
	uint32_t		getNextPos(uint32_t pos);
	uint32_t			perc(uint32_t i1, uint32_t i2);
	void			reset();
	void			emptyVoice(uint32_t seq);
	bool			isNeedResend(uint32_t seq);
	bool			isWork();

	const protocol::session::VoiceStatisticsData& getData();

private:
	uint32_t					__lastseq;
	voiceState					__voiceStates[MAX_STAT_VECTOR];
	protocol::session::VoiceStatisticsData			__lastData;	
};