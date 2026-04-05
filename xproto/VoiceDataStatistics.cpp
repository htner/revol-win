#include "common/nsox/nulog.h"
#include "VoiceDataStatistics.h"


VoiceDataStatistics::VoiceDataStatistics(void)
{
	reset();	
}

VoiceDataStatistics::~VoiceDataStatistics(void)
{
}


void VoiceDataStatistics::emptyVoice(uint32_t seq)
{
	int pos = getPos(seq);
	__voiceStates[pos].__emptyPakSeq = seq;
}

bool VoiceDataStatistics::isNeedResend(uint32_t seq)
{
	int pos = getPos(seq);
	if (__voiceStates[pos].__emptyPakSeq == seq) {
		nsox::xlog(NsoxFatal, " seq = %d has empty data, do not need resend", seq);
		return false;
	}
	return true;
}

uint32_t VoiceDataStatistics::getPos(uint32_t seq)
{
	return (seq / PACKET_SEQ_INCREMENT) % MAX_STAT_VECTOR;
}

uint32_t VoiceDataStatistics::getNextPos(uint32_t pos)
{
	pos++;
	return pos % MAX_STAT_VECTOR;
}

void VoiceDataStatistics::empty(uint32_t seq)
{
	int pos = getPos(seq);
	if (__voiceStates[pos].__seq > seq) {
		return;
	} else if(__voiceStates[pos].__seq == seq) {
		__voiceStates[pos].__emptyTtimes++;
		return;
	} 
	//assert(0);  
	flush(seq, e_s_loss);
	__voiceStates[pos].__emptyTtimes = 1;
}

void VoiceDataStatistics::flush(uint32_t seq, bool isLoss, bool isResend)
{
	PACKET_STATE s = e_s_succ;
	if (isResend) {
		if (isLoss) {
			s = e_s_resend_late;
		} else {
			s = e_s_resend_succ;
		}
	} else if (isLoss){
			s = e_s_real_loss;
	}
	flush(seq, s);
}



void VoiceDataStatistics::flush(uint32_t seq, PACKET_STATE s)
{
	int pos = getPos(seq);
	int real_seq = seq;

	uint32_t lastpos = -1;
	
	if (__lastseq != 0) {
		lastpos = getPos(__lastseq);
		if (__voiceStates[lastpos].__seq != __lastseq) {
			//assert(0);
			reset();		//數據錯亂了
			return;
		}
	}
	
	if (lastpos != -1) {
		int diff = real_seq > __lastseq ? (real_seq - __lastseq) : (__lastseq - real_seq);
		if (diff > MAX_JUMP_VECTOR) {
				// 來的包有問題
				nsox::xlog(NsoxFatal, "VoiceDataStatistics reset, _lastseq => %d _inseq => %d, ", __lastseq,  real_seq);
				reset();
				return;
		}
	} 
	uint32_t oldseq = __voiceStates[pos].__seq;

	if (oldseq == real_seq) {
		PACKET_STATE oldstate = __voiceStates[pos].__states;
		if (oldstate != e_s_resend_succ && oldstate != e_s_succ) {
			//__voiceStates[pos].is_udp_ = protocol::session::g_is_udp;
			__voiceStates[pos].__states = s;
		} else if (oldstate == e_s_succ && s == e_s_resend_late) {			
			__voiceStates[pos].__states = e_s_resend_excess;
		}
		return;
	}

	if (__lastseq < real_seq || lastpos == -1) {
		if (lastpos != -1) {
			uint32_t p_seq = __lastseq;
			for (int i = getNextPos(lastpos); i != pos; ) {
				p_seq += PACKET_SEQ_INCREMENT;
				__voiceStates[i].__states = e_s_loss;
				__voiceStates[i].__seq = p_seq;
				__voiceStates[i].__emptyTtimes = 0;  //刷新狀態
				//__voiceStates[i].is_udp_ = protocol::session::g_is_udp;
				i = getNextPos(i);
			}

		}
		__lastseq = real_seq;
	}
	__voiceStates[pos].__states = s;
	__voiceStates[pos].__seq = real_seq;
	__voiceStates[pos].__emptyTtimes = 0;  //刷新狀態
	//__voiceStates[pos].is_udp_ = protocol::session::g_is_udp;
}

void VoiceDataStatistics::log(bool tolog, uint32_t currect_seq)
{
	if (__lastseq == 0) {
		return;
	}

	__lastData.total			= SHOW_COUNT;
	__lastData.no_init		= 0;
	__lastData.loss			= 0;
	__lastData.error			= 0;
	__lastData.first			= 0;
	__lastData.resend_succ	= 0;
	__lastData.resend_late	= 0;
	__lastData.resend_error	= 0;
	__lastData.resend_excess	= 0;
	__lastData.succ			= 0;

	uint32_t lastpos	= getPos(__lastseq);

	int f_seq = __lastseq;
	int t_seq = 0;
	__lastData.emptys = 0;
	__lastData.speak_loss_maybe  = 0;

	for (int i = 0; i < SHOW_COUNT; i++)
	{
		int pos = getPos(currect_seq - i*2);
		PACKET_STATE state = __voiceStates[pos].__states;
		int seq = __voiceStates[pos].__seq;
		if (state != e_s_init) {
				if(seq > t_seq) {
					t_seq = seq;
				}
				if (seq < f_seq && seq != 0) {
					f_seq = seq;
				}
				if (__lastseq < seq) {
					//assert(0);
				}
		} else {
			__lastData.total--;
			__lastData.no_init++;
			continue;
		}

		if (state == e_s_real_loss || state == e_s_loss) {
				if ( __voiceStates[pos].__emptyPakSeq == seq) {
					__lastData.speak_loss_maybe++;
				}
		}

		__lastData.emptys +=  __voiceStates[pos].__emptyTtimes;
		switch(state) 
		{				
		case  e_s_init:	
					break;
		case  e_s_loss:
					__lastData.loss++;
					break;
		case  e_s_succ:
					__lastData.first++;
					__lastData.succ++;
					break;
		case  e_s_resend_succ:
					__lastData.succ++;
					__lastData.resend_succ++;
					break;
		case e_s_resend_excess:
					__lastData.first++;
					__lastData.succ++;
					__lastData.resend_excess++;
					break;
		case  e_s_resend_late:
					__lastData.resend_late++;
					__lastData.loss++;
					break;
		case  e_s_resend_error:
					__lastData.resend_error++;
					__lastData.loss++;
					break;
		case  e_s_real_loss:
					__lastData.loss++;
					break;
		case  e_s_resend:
					__lastData.loss++;
					__lastData.resend_error++;
					break;
					
		}
	}

	__lastData.gets = __lastData.speak_loss_maybe + __lastData.succ;
	__lastData.total_percentage = 100;
	__lastData.no_init_percentage = 0;
	__lastData.loss_percentage = perc(__lastData.total, __lastData.loss);
	__lastData.error_percentage = perc(__lastData.total, __lastData.error);
	__lastData.first_percentage = perc(__lastData.total,__lastData.first);
	__lastData.resend_succ_percentage =  perc(__lastData.total, __lastData.resend_succ);
	__lastData.resend_late_percentage = perc(__lastData.total, __lastData.resend_late);
	__lastData.resend_error_percentage = perc(__lastData.total, __lastData.resend_error);
	__lastData.resend_excess_percentage = perc(__lastData.total, __lastData.resend_excess);
	__lastData.succ_percentage = perc(__lastData.total, __lastData.succ);
	__lastData.gets_percentage = perc(__lastData.total, __lastData.gets);

	if (tolog) {
		nsox::xlog(NsoxFatal, "//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
 		nsox::xlog(NsoxFatal, "//  voice data Statistics(seq %d=>%d): total=%d, success=%d, loss=%d, error=%d, insert empty =%d, maybe speaker loss=%d", 
							f_seq, t_seq, __lastData.total, __lastData.succ, __lastData.loss, __lastData.error, __lastData.emptys, __lastData.speak_loss_maybe);
 		nsox::xlog(NsoxFatal, "//  ....: initial success=%d, resend success=%d, resend comes late=%d, resend error=%d, resend excess=%d", 
							__lastData.first, __lastData.resend_succ, __lastData.resend_late, __lastData.resend_error, __lastData.resend_excess);
	 
 		nsox::xlog(NsoxFatal, "//  voice data Statistics Percentage (1000%%):  success=%d%%, loss=%d%%, error=%d%%.", 
							__lastData.succ_percentage, __lastData.loss_percentage,__lastData.error_percentage);
 		nsox::xlog(NsoxFatal, "//  ....: initial success=%d%%, resend success=%d%%, resend comes late=%d%% resend error=%d%%", 
							__lastData.first_percentage,  __lastData.resend_succ_percentage,  __lastData.resend_late_percentage, __lastData.resend_error_percentage);;
		nsox::xlog(NsoxFatal, "//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
	}
}

uint32_t VoiceDataStatistics::perc(uint32_t i1, uint32_t i2)
{
	if (i1 == 0)
		return 0;
	return (i2 * 1000) / i1;
}

void VoiceDataStatistics::reset() 
{
	nsox::xlog(NsoxFatal, "voice data Statistics reset");
	__lastseq = 0;
	for (int i = 0; i < MAX_STAT_VECTOR; i++) {
		__voiceStates[i].reset();
	}
}

const protocol::session::VoiceStatisticsData& VoiceDataStatistics::getData()
{
	return __lastData;
}

bool VoiceDataStatistics::isWork() {
	return __lastseq != 0;
}