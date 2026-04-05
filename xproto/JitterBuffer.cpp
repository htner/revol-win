#include "JitterBuffer.h"

#include "SessionImp.h"
#include "JitterBuffer.h"
#include "common/nsox/nulog.h"
#include "atlstr.h"
#include "Mmsystem.h"
#include "audio/samples.h"

#define	 JDATA_SEQ_INCREMENT	1
#define  max_lost_gap 5
#define  VALID_SEQ(x)  (x !=-1)

extern bool		__is_packet_loss_log_on;
const UINT		GITTER_PACKETS  = 2;



bool vpacket_compare(IJitterData* first, IJitterData* second)
{
		return first->Seq() < second->Seq();
}
void JitterData::flush(IJitterPumper* pumper)
{
		JDataArray::iterator itr = voices.begin();
		for(; itr != voices.end(); ++ itr){
				pumper->on_pump_data(*itr);
				(*itr)->Destroy();
		}
		voices.clear();
}

bool JitterData::pop(IJitterPumper* pumper) 
{
		DWORD now = timeGetTime();

		if(bstop && voices.empty()){
				pumper->on_stop_data(uFrom); // stop
				return false;
		}
		total ++;	
		bool remove = false;
		IJitterData* voice = &emptyVoice;
		if(currentSeq == NULL) // first play
		{
			voice = voices.front();	
			currentSeq = voice->Seq();
			remove = true;
		}else{
			if(!voices.empty())
			{
				waitTimes = 0;
				UINT seq  =  voices.front()->Seq();
				assert(seq > currentSeq);
				if(seq != currentSeq + JDATA_SEQ_INCREMENT) //补空包, 丢包
				{ 
					voice = &emptyVoice;  //补空包

					if(maxGitterTime > totalWait){
						totalWait += timeGap;
						if(__is_packet_loss_log_on){
							nsox::xlog(NsoxInfo, "gitter time max than  wait time, waiting..., %d <===> %d", 
								maxGitterTime, totalWait);
						}
					}else{
						totalLoss ++;
						currentSeq += JDATA_SEQ_INCREMENT;		

						float rate = (float)totalLoss*100 / (float)total;	
						if(__is_packet_loss_log_on){
							/*nsox::xlog(NsoxInfo,  "packet really loss seq : %f", currentSeq , 
								", total loss: ", totalLoss,", total packet:", total, 
								", loss rate: ", rate, ", last seq :", seq);*/
							nsox::xlog(NsoxInfo,  "packet really loss seq : %d, total loss:%d, total packet:%d, loss rate: %f,last seq :%d",
												   currentSeq , totalLoss, total, rate,seq);

							DWORD time = now - firstTime;
							time -=  (currentSeq - firstSeq) * (timeGap >> 1); // 80/2
							nsox::xlog(NsoxInfo,"%s totalWait: %d, offset:%d, maxGitterTime:%d", uFrom.c_str(), totalWait, time, maxGitterTime);
						}																																		
					}								
				}else{
						remove = true;
						voice = voices.front();
						currentSeq += JDATA_SEQ_INCREMENT; 								
				}					
			}else{
				totalWait += timeGap;
				if(__is_packet_loss_log_on){
						nsox::xlog(NsoxInfo," %s voice queue empty %d times, waiting... , %d <===> %d",
									uFrom.c_str(), waitTimes, maxGitterTime,totalWait);
				}
				waitTimes ++; // if continues empty remove it
			}
		}
		if(waitTimes > 10){
				nsox::xlog(NsoxInfo,"%s empty for 10 times, stop.....", uFrom.c_str());
				pumper->on_stop_data(uFrom);// stop data
		}else{
				if(!(voice->From().empty())){
						pumper->on_pump_data(voice);
				}else{
						pumper->on_empty_data(uFrom);
				}
		}
		if(remove){
				voices.front()->Destroy();
				voices.erase(voices.begin());
		}
		if(__is_packet_loss_log_on)
		{
				DWORD time = now - firstTime;
				time -=  (currentSeq - firstSeq) * (timeGap >> 1); // 80/2

				nsox::xlog(NsoxInfo,"%s voice queue size : %d, , currentSeq:%d, totalWait:%d<====>%d,  at:%d",  
					uFrom.c_str(), voices.size() * timeGap, currentSeq,  totalWait,  (int)time,  now);
		}
		return waitTimes <= 10;
}

bool	JitterData::insert(IJitterData* vpak)
{
	if(vpak->Seq() <= currentSeq) return false;

	JDataArray::iterator itr = std::lower_bound(voices.begin(),
												voices.end(),
												vpak,
												vpacket_compare);
	if(itr != voices.end() && (*itr)->Seq() == vpak->Seq()) //已经存在
	{
		return false;
	}

	voices.insert(itr,vpak->Clone());
	return true;
}

void JitterData::reset()
{
	waitTimes = 0;
	firstSeq = currentSeq = lastSeq = NULL;
	total = totalLoss = totalWait = 0;
	maxGitterTime = 0;
	bstop	= FALSE;
	voices.clear();
	firstTime	   = timeGetTime();
	emptyVoice.seq	= 0;
}
void JitterData::stop()
{
	bstop = true;
}

UINT	JitterData::push(IJitterData* vpak)//插入一个语音包
{
	if(bstop){			
			if(__is_packet_loss_log_on){	
				nsox::xlog(NsoxWarn, "user voice has stoped, %s", uFrom.c_str());
			}
			return vpak->Seq();
	}
	UINT tempSeq = lastSeq;
	if(firstSeq == NULL) // first push
	{
		reset();

		// for anti gitter, there has buffer
		firstSeq = vpak->Seq() - GITTER_PACKETS*JDATA_SEQ_INCREMENT;	
		tempSeq  = lastSeq = vpak->Seq();

		EmptyVoice gitterVoice; 
		for(unsigned int i = GITTER_PACKETS; i > 0;  -- i)
		{
				gitterVoice.seq = vpak->Seq() - i*JDATA_SEQ_INCREMENT;
				insert(&gitterVoice);
		}
	}else
	{
		if(vpak->Seq()  >=  SEQPAGE + lastSeq){ //next SEQPAGE				
			reset();//warning, voice queue not empty

			if(__is_packet_loss_log_on){	
				nsox::xlog(NsoxWarn, "user queue been cleard by large seq : %s", uFrom.c_str());
			}
			firstSeq = lastSeq	= vpak->Seq();		
		}else{						
			if(__is_packet_loss_log_on){					
	

				DWORD now = timeGetTime();
				DWORD time =  now - firstTime - (vpak->Seq() - firstSeq) * (timeGap >> 1); // 

				if((int)time > (int)maxGitterTime) maxGitterTime = min(time, 560); //写死了，wowo
				nsox::xlog(NsoxInfo, " %s packet latency : %d, seq:%d, time:%d ", 
									  uFrom.c_str(),  (int)time, vpak->Seq(), now);
			}

			if(lastSeq >= vpak->Seq()){
				if(__is_packet_loss_log_on){
					nsox::xlog(NsoxInfo, " packet received loss seq : %d", vpak->Seq());
				}								
			}
			else{
				lastSeq = vpak->Seq();		
			}
		}		
	}			
	insert(vpak);

	return tempSeq;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

VoiceTimeWindow::VoiceTimeWindow(JitterBuffer * imp):handler(imp)
{
	__offset = __lasttime = NULL;

}
VoiceTimeWindow::~VoiceTimeWindow()
{
	CTimer::Instance()->UnRegisterListener(this);		
}

void VoiceTimeWindow::TimeArrive()
{
	DWORD now  = timeGetTime();
	if(__lasttime != NULL){
		__offset += now - __lasttime;				
	}else{
		__offset = __timeGap;
	}
	__lasttime = now;
	while(__offset >= __timeGap)
	{
		__offset -= __timeGap;
		handler->onTimer();
	}
}
void   VoiceTimeWindow::start(int timeGap)
{
	__timeGap = timeGap;
	CTimer::Instance()->RegisterListener(this, timeGap);
}

void JitterBuffer::Clear()
{
	voice_map::iterator itr = __user_voices.begin();
	for(; itr != __user_voices.end(); ++ itr)
	{
			if(__voice_pumper){
				__voice_pumper->on_stop_data(itr->first);
			}
	}
	__user_voices.clear();
}

void JitterBuffer::onTimer() // 需要做时间校正
{
	std::vector<std::string> removes;

	voice_map::iterator itr = __user_voices.begin();
	for(; itr != __user_voices.end(); ++itr){
		if(!itr->second.pop(__voice_pumper)){
				if(__is_packet_loss_log_on){
						nsox::xlog(NsoxInfo, "user voice stop, %s", itr->first.c_str());
				}
				removes.push_back(itr->first);
		}
	}
	for(int i=0 ; i <removes.size(); ++ i){		
		__user_voices.erase(removes[i]);
	}
}

JitterBuffer::JitterBuffer(void):__timeWnd(this)
{
	__voice_pumper	= NULL;
	__packet_notify = NULL; 
	__jitterGap		= 80;	
}

JitterBuffer::~JitterBuffer(void)
{
}


void JitterBuffer::Push(IJitterData* vpak)
{
	if(vpak->From().empty()) return;

	voice_map::iterator itr = __user_voices.find(vpak->From());
	if(itr != __user_voices.end()){	
		UINT lastseq = itr->second.push(vpak);
		on_packet_loss(vpak->From(), lastseq, vpak->Seq());									
	}else{ // no such user 
		JitterData& vd = __user_voices[vpak->From()];
		vd.uFrom	   = vpak->From();
		vd.timeGap	   = __jitterGap;
		vd.push(vpak);				
	}			
}

void JitterBuffer::on_packet_loss(const std::string& user, int fromseq,  int toseq)
{
	if(fromseq >= toseq) return;

	int losscount = (toseq - fromseq)/JDATA_SEQ_INCREMENT;
	if(losscount < max_lost_gap){

		for(int seq =fromseq+ JDATA_SEQ_INCREMENT ; seq<toseq; seq += JDATA_SEQ_INCREMENT)
		{
			if(__is_packet_loss_log_on){
				nsox::xlog(NsoxInfo, "client packet loss seq : %d, last seq: %d", seq, toseq);
			}										
			__packet_notify->on_packet_loss(user, seq); // 数据包丢失
		} 
	}else{
		if(__is_packet_loss_log_on){
			nsox::xlog(NsoxInfo, "client packet loss too much from: %d, to: %d", fromseq, toseq);
		}
	}
}

void JitterBuffer::SetNotify(IJitterNotify* notifyer)
{
	__packet_notify = notifyer;
}
void JitterBuffer::SetPumper(IJitterPumper* pumper)
{
	__voice_pumper = pumper;
}
void JitterBuffer::Stop(const UID& uid)
{
		if(__is_packet_loss_log_on){
				nsox::xlog(NsoxInfo,"%s begin stop voice", uid.c_str());
		}
		__user_voices[uid].stop();
}
void JitterBuffer::Destroy()
{
		delete this;
}
void JitterBuffer::SetJitterGap(int time)
{
		__jitterGap = time;
		__timeWnd.start(__jitterGap);
}

IJitterBuffer* CoCreateJitterBuf()
{
		return new JitterBuffer();
}