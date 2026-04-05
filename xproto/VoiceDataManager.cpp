#include "SessionImp.h"
#include "voicedatamanager.h"
#include "common/nsox/nulog.h"
#include "atlstr.h"
#include "Mmsystem.h"
#include "audio/samples.h"
#include <math.h>
#include "memMisc.h"
#include "location/default.h"
#include "helper/ufile.h"
#include "report.h"

/*
 * max lost gap must sync to server,  or too much resendvoice paceket to server.
 */
#define  MAX_LOST_GAP 5 
#define MAX_NO_REPORT_TIME (30 * 60 * 1000)

using namespace protocol::session;


/*XVoiceData* _chat2voice_(PExpandInfoVoice* cv)
{	
	XVoiceData* vd = (XVoiceData*)_temp_alloc_(sizeof(XVoiceData));
	vd->payloadsize = cv->chat.size();
	if( vd->payloadsize > 0){
		vd->payload =  (char*)_temp_alloc_(vd->payloadsize) ;
		memcpy(vd->payload,cv->chat.data(),vd->payloadsize);			
	}else
		vd->payload = NULL;
	vd->seq		= cv->seq;
	vd->user	= cv->from;
	vd->vt	= cv->vt;
	vd->qc	= cv->qc;
	vd->next = NULL;
	return vd;
}
*/

void _free_voicedata_(XVoiceData* head);
// must free outside, xxx,  modify by client
XVoiceData* CopyBaseToNewXVoice(protocol::session::PBaseSessVoice* voice) {
			XVoiceData* xdata;
#ifdef  WIN32
			xdata = (XVoiceData*)_temp_alloc_(sizeof(XVoiceData));
#else
			xdata = new XVoiceData();
#endif
			assert(xdata);
			xdata->payloadsize = voice->chat.size();
			if( xdata->payloadsize > 0){
#ifdef  WIN32
				xdata->payload =  (char*)_temp_alloc_(xdata->payloadsize) ;
#else
				xdata->payload =  (char*)malloc(xdata->payloadsize) ;
#endif
				assert(xdata->payload);
				if (xdata->payload) {
					memcpy(xdata->payload,voice->chat.data(),xdata->payloadsize);			
				} else {
					xdata->payload = NULL;
					xdata->payloadsize  = 0;
				}
			}else {
				xdata->payload = NULL;
			}
			xdata->seq	= voice->seq;
			xdata->user	= voice->from;
			xdata->vt	= voice->vt;
			xdata->next = NULL;
			return xdata;
}

XVoiceData* CopyBaseToNewXVoice(int seq, int payloadSize, char* payload, UINT from, VoiceType vt) 
{
	XVoiceData* xdata;
#ifdef  WIN32
	xdata = (XVoiceData*)_temp_alloc_(sizeof(XVoiceData));
#else
	xdata = new XVoiceData();
#endif
	assert(xdata);
	xdata->payloadsize = payloadSize;
	if( xdata->payloadsize > 0){
#ifdef  WIN32
		xdata->payload =  (char*)_temp_alloc_(xdata->payloadsize) ;
#else
		xdata->payload =  (char*)malloc(xdata->payloadsize) ;
#endif
		assert(xdata->payload);
		if (xdata->payload) {
			memcpy(xdata->payload,payload ,xdata->payloadsize);			
		} else {
			xdata->payload = NULL;
			xdata->payloadsize  = 0;
		}
	}else {
		xdata->payload = NULL;
	}
	xdata->seq	= seq;
	xdata->user	= from;
	xdata->vt	= vt;
	xdata->next = NULL;
	return xdata;
}



XVoiceData* CopyToNewXVoice(PBaseSessVoice*  voice)
{
	XVoiceData* xdata = NULL;
	if (voice) {
		switch (voice->getUri()) {
		case PChatVoice::uri : {
			xdata =  CopyBaseToNewXVoice((PChatVoice*) voice);
			break;
		 }
		case PExpandTypeVoice::uri: {
			xdata = CopyBaseToNewXVoice((PExpandTypeVoice*) voice);
			if (xdata) {
				xdata->type= ((PExpandTypeVoice*) voice)->type;
			}
			break;
		}
		case PExpandQcVoice::uri : {
			xdata =  CopyBaseToNewXVoice((PExpandQcVoice*) voice);
			if (xdata) {
				xdata->qc= ((PExpandQcVoice*)voice)->qc;
			}
			break;
		}
		case PBroadcastVoice::uri : {
			xdata =  CopyBaseToNewXVoice((PBroadcastVoice*) voice);
			//if (xdata) {
			//	xdata->qc= ((PBroadcastVoice*)voice)->qc;
			//}
			break;
		}
		}
	}
	return xdata;
}

const int MAX_LIMIT = 5;
bool		__is_voices_fatal_log_on = false;





bool vpacket_compare(const XVoiceData* first, const XVoiceData* second)
{
	return first->seq < second->seq;
}

/*
void VoiceData::flush(IVoicePumper* pumper)
{
		std::vector<PChatVoice>::iterator itr = voices.begin();
		for(; itr != voices.end(); ++ itr){
			pumper->on_pump_data(*itr);
		}
		voices.clear();
}
*/

void VoiceData::set_voice_quality(IVoiceQuality* quality)
{
	__voice_quality = quality;
}

VoiceType VoiceData::getVoiceType()
{
	return voiceType;
}

void VoiceData::setJitter(uint32_t jit)
{
	__jitter = jit;
}

void VoiceData::parsePacket(XVoiceData* data)
{
	int seq = 0;
	UINT from = 0;

	if(data != NULL)
	{
		char* payload = data->payload;
		seq = data->seq; from = data->user;
		int left = data->payloadsize;

		if(data->payloadsize > 0 && payload != NULL)
		{
			int packSize = *((int *) payload);
			if(packSize > 6 || packSize <= 0) 
			{
				//assert(0);
				return;
			}

			_packetCount = packSize;
			payload += 4;left -= 4;
			
			for(int i = 0; i < _packetCount; i++)
			{
				if(left < 2)
				{
					assert(0);
					return;
				}

				short len = *((short*) payload);
				payload += 2;left -= 2;

				if(len > left)
				{
					assert(0);
					return;
				}
				_packets.push_back(CopyBaseToNewXVoice(seq, len, payload, from, voiceType));
				left -= len;payload +=len;
				
			}
			return;
		}
	}

	for(int i = 0; i < _packetCount; i++)
		_packets.push_back(CopyBaseToNewXVoice(seq, 0, NULL, from, voiceType));
}

//extern XVoiceData* _chat2voice_(PExpandInfoVoice* cv);
bool VoiceData::fetch(XVoiceData** out)
{		
		if(!_packets.empty())
		{
			*out = *_packets.begin();
			_packets.erase(_packets.begin());
			return true;
		}

		if(bstop && voices.empty()){
				__emptyVoice.seq = -1;//special one for empty

				*(out) = CopyToNewXVoice(&__emptyVoice);
				return false;
		}
		DWORD now = timeGetTime();
		total ++;	
		if(firstPopTime==0){
				firstPopTime = now;
		}
		bool remove = false;
		
		XVoiceData* voice = NULL;// &__emptyVoice;
		if(currentSeq == NULL) // first play
		{
			voice = (voices.front());	
			currentSeq = voice->seq;
			remove = true;
		}else{							
				UINT seq  =  0;
				if(!voices.empty()){
					emptyTimes = 0;
					seq = voices.front()->seq;
					assert(seq > currentSeq);
				}else{
					emptyTimes ++;
					if(__is_voices_fatal_log_on){
						nsox::xlog(NsoxInfo," %d voice queue empty %d times, waiting... , %d <===> %d",
									__emptyVoice.from, emptyTimes, maxGitterTime,totalWait);
					}
				}
				if(voices.empty() || (seq != currentSeq + PACKET_SEQ_INCREMENT)) //补空包, 丢包
				{ 
					//voice = _chat2voice_(&__emptyVoice);//&__emptyVoice;  //补空包
					voice = CopyToNewXVoice(&__emptyVoice);    //补空包
					if(maxGitterTime > totalWait){
						totalWait += GetPackTime();
						if(__is_voices_fatal_log_on){
							nsox::xlog(NsoxInfo, "gitter time max than  wait time, waiting...,maxGitterTime = %d, totalWait = %d", 
								maxGitterTime, totalWait);
						}
					}else{
						totalLoss ++;
						currentSeq += PACKET_SEQ_INCREMENT;		

						float rate = (float)totalLoss*100 / (float)total;	
						if(__is_voices_fatal_log_on){
							nsox::xlog(NsoxInfo,  "packet really loss seq : %d, total loss:%d, total packet:%d, loss rate: %f,last seq :%d",
												   currentSeq , totalLoss, (lastSeq-firstSeq)/PACKET_SEQ_INCREMENT, rate, lastSeq);

							DWORD time = now - firstFeedTime;
							time -=  (currentSeq - firstSeq) * (GetPackTime()/PACKET_SEQ_INCREMENT); // 80/2
							nsox::xlog(NsoxInfo,"%d totalWait: %d, offset:%d, maxGitterTime:%d", __emptyVoice.from, totalWait, time, maxGitterTime);
						}																																		
					}
#ifndef	NO_VOICE_STAT
					__statistics.empty(currentSeq);//通知统计补空包
#endif
				}else{
						remove = true;
						voice = (voices.front());
						currentSeq += PACKET_SEQ_INCREMENT; 								
				}							
		}
		if(emptyTimes > 10){
				nsox::xlog(NsoxInfo,"%d empty for 10 times, stop.....", __emptyVoice.from);
				voice->seq = -1;//speical seq for stop voice
		}
		//*out = voice ? voice : _chat2voice_(&__emptyVoice);
// 		if (voice) {
// 			*out = voice;
// 		} else {
// 			*(out) = CopyToNewXVoice(&__emptyVoice);
// 		}
		parsePacket(voice);
		//assert(!_packets.empty());
		if(!_packets.empty())
		{
			*out = *_packets.begin();
			_packets.erase(_packets.begin());
		}

		if(remove){
			voices.erase(voices.begin());
		}

		_free_voicedata_(voice);
		if(__is_voices_fatal_log_on)
		{
				DWORD time = now - firstFeedTime;
				time -=  (currentSeq - firstSeq) * (GetPackTime()/PACKET_SEQ_INCREMENT); // 80/2

				float avrPopTime = (float)(now - firstPopTime)/total;

				nsox::xlog(NsoxInfo,"%d voice queue size:%d ms, currentSeq:%d,  lastSeq:%d, totalWait:%d<====>%dms, avrPopTime:%f, total:%d",  
					__emptyVoice.from, int(voices.size() * GetPackTime()), currentSeq,  lastSeq, totalWait,  (int)time,  avrPopTime, total);
		}
		return emptyTimes <= 50;
}

bool	VoiceData::insert(PBaseSessVoice* vpak)
{
	if(vpak->seq <= currentSeq) {
		return false;
	}
	static XVoiceData _dummy_;
	_dummy_.seq = vpak->seq;
	VoiceDataContainer::iterator itr = std::lower_bound(voices.begin(),
		voices.end(),
		&_dummy_,
		vpacket_compare);
	if(itr != voices.end() && (*itr)->seq == vpak->seq) //已经存在
	{
		return false;
	}
	XVoiceData* vd ;
	vd = CopyToNewXVoice(vpak);
	assert(vd);
	if (vd ) {
		//XVoiceData* vd = _chat2voice_(vpak);
		voices.insert(itr,vd);
		return true;
	}
	return false;
}

void VoiceData::clearVoice()
{
	extern void _free_voicedata_(XVoiceData* head);
	VoiceDataContainer::iterator it = voices.begin();
	VoiceDataContainer::iterator itEnd = voices.end();
	for (; it != itEnd; it++)
	{
		_free_voicedata_(*it);
	}

	it = _packets.begin();
	for(; it != _packets.end(); it++)
	{
		_free_voicedata_(*it);
	}

	_packets.clear();
	voices.clear();
}
void VoiceData::reset(UINT fseq)
{
	emptyTimes = 0;
	firstSeq = currentSeq = lastSeq = NULL;
	total = totalLoss = totalWait = 0;
	maxGitterTime = 0;
	bstop	= FALSE;
	clearVoice();
	firstPopTime = 0;
	__emptyVoice.seq = 0;
	if(fseq){
		int jitterPacket = __jitter/GetPackTime();
		if(jitterPacket <= 0)
			jitterPacket = 1;
		else if(jitterPacket > 15)
			jitterPacket = 15;

		nsox::xlog(NsoxInfo, "insert %d gitter packet at header",jitterPacket);
		firstFeedTime = timeGetTime() - jitterPacket*GetPackTime();
		firstSeq = fseq - (jitterPacket*PACKET_SEQ_INCREMENT);		
		//for anti gitter, there has buffer, using empty_voice do not care real type
		PChatVoice gitterVoice = __emptyVoice; 
		for(unsigned int i = jitterPacket; i>0;  -- i){
				gitterVoice.seq = fseq - i*PACKET_SEQ_INCREMENT;
				insert(&gitterVoice);
		}
		lastSeq	 = fseq;
	}else{
		firstFeedTime = timeGetTime();
	}
}

bool VoiceData::stop()
{	
	if (bstop == false) {
		bstop = true;
		return true;
	} else {
		return false;
	}
}


UINT	VoiceData::push(PBaseSessVoice* vpak, bool isResend)//插入一个语音包
{
	voiceType = vpak->vt;
	if(bstop){			
			if(__is_voices_fatal_log_on){	
				nsox::xlog(NsoxWarn, "user voice has stoped, %d", __emptyVoice.from);
			}
			return vpak->seq;
	}
	UINT tempSeq = lastSeq;
	if(firstSeq == NULL) // first push
	{
		reset(vpak->seq);		
		tempSeq = firstSeq;
	}else
	{
		if(vpak->seq  >=  SEQPAGE + lastSeq){ //next SEQPAGE				
			reset(vpak->seq);//warning, voice queue not empty
			if(__is_voices_fatal_log_on){	
				nsox::xlog(NsoxWarn, "user queue been cleard by large seq : %d", __emptyVoice.from);
			}
			tempSeq = firstSeq;
		}else{									
			DWORD now = timeGetTime();
			DWORD time =  now - firstFeedTime - (vpak->seq - firstSeq) * (GetPackTime() / 2); // 
			if(lastSeq >= vpak->seq){
				if(__is_voices_fatal_log_on){
					nsox::xlog(NsoxInfo, " packet received loss seq : %d", vpak->seq);
				}								
			} else{
				if((int)time > (int)maxGitterTime){
						maxGitterTime = min(time, 560); //写死了，woow
				}
				lastSeq = vpak->seq;		
			}	
			if(__is_voices_fatal_log_on){	
				if(lastSeq==vpak->seq && lastSeq!=firstSeq){
						float avrFeedTime = (float)(now-firstFeedTime)*2/(lastSeq-firstSeq);
						nsox::xlog(NsoxInfo, " %d packet latency : %d, seq:%d, time:%d , avrFeedTime:%f, total:%d", 
										vpak->from,  (int)time, vpak->seq, now,avrFeedTime,(lastSeq-firstSeq)/PACKET_SEQ_INCREMENT);

				}else{
						nsox::xlog(NsoxInfo, " %d packet latency : %d, seq:%d, time:%d ", 
												vpak->from,  (int)time, vpak->seq, now);
				}
			}
	
		}		
	}			


	
#ifndef	NO_VOICE_STAT
	bool succ = insert(vpak);
	__statistics.flush(vpak->seq, !succ, isResend);  //通知统计插入情况
#else
	insert(vpak);
#endif

	return tempSeq;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

double VoiceData::GetPackTime()
{
	double t = 80.0;

	if(__voice_quality)
	{
		UINT32 quality = __voice_quality->GetVoiceQuality();

		if((quality>>24) == 9)
		{
			t = 1024.0*2*__voice_quality->GetSegmentInPack() / 48.0;
		} else {
			t = __voice_quality->GetDefaultPackTime();
		}
	}

	return t;
}


#ifndef NO_VOICE_STAT
void VoiceData::log(bool b)
{
	__statistics.log(b, currentSeq);
}

bool VoiceData::isNeedResend(uint32_t seq)
{
	return __statistics.isNeedResend(seq);
}


void VoiceData::on_pack_loss(uint32_t seq)
{
	__statistics.flush(seq, VoiceDataStatistics::e_s_resend);
	
}
void VoiceData::resend_error(uint32_t seq)
{
	__statistics.flush(seq, VoiceDataStatistics::e_s_resend_error);
}

void VoiceData::pushEmpty(PEmptyChatVoice* vemptypak)
{
	__statistics.emptyVoice(vemptypak->seq);
}
#endif

/************************************************************************/
/*                                                                      */
/************************************************************************/


void VoiceDataManager::clear()
{
	CAutoLock lock(&__cs);
	//PBaseSessVoice endvoice;
	/*
	voice_map::iterator itr = __user_voices.begin();
	for(; itr != __user_voices.end(); ++ itr)
	{
			endvoice.from = itr->first;
			endvoice.seq  = -1;
			if(__voice_pumper){
				__voice_pumper->on_pump_data(endvoice);
			}
	}
	*/
	__user_voices.clear();
}


VoiceDataManager::VoiceDataManager(void)
{
	__statisticsTimer.init(this, &VoiceDataManager::statistics);
	__statisticsTimer.start(SHOW_S_TIME);

	__last_report = 0;
	ping_average_ = 0;
	__running = false;
	__voice_pumper = NULL;
	__packet_notify = NULL; 
	InitializeCriticalSection(&__cs);

	xstring path = GetCurrentPathDir();


	xstring xx = getAppPath();
	xx += _T("\\rcEnv.ini");
	__is_voices_fatal_log_on = GetPrivateProfileIntW(L"Log",	L"voiceLog", 0 ,xx.c_str());

	 __check_reportvoice = GetPrivateProfileIntW(L"xproto",      L"check_reportvoice", 10, xx.c_str());
	 __reportvoice_interval = GetPrivateProfileIntW(L"xproto",      L"reportvoice_interval", 600, xx.c_str());

	if(__is_voices_fatal_log_on)
	{
		nsox::xlog(NsoxInfo, "=+++++++ packet loss log on ++++++++=");
	}

	initJitter();
}

VoiceDataManager::~VoiceDataManager(void)
{
	StopAll();
	DeleteCriticalSection(&__cs);
}

void _free_voicedata_(XVoiceData* head)
{
	XVoiceData* item = head;
	while( item ){
		XVoiceData* p = item;
		item = item->next;
		p->payload ? _temp_free_(p->payload) : NULL;
		_temp_free_(p);
	}
}

void VoiceDataManager::releaseVoice(XVoiceData* head)
{
	_free_voicedata_(head);
}

XVoiceData* VoiceDataManager::fetchVoice(/*voice_map& voices,*/int nTimeStamp, VoiceType vt)
{
	if  (!__running) {
		return NULL;
	}
	CAutoLock lock(&__cs);
	XVoiceData* tail = NULL;
	XVoiceData* head = NULL;

	voice_map::iterator itr = __user_voices.begin();
	for(; itr != __user_voices.end(); ){
		if (itr->second.getVoiceType() != vt) {
			++itr;
			continue;
		}
		XVoiceData* vd = NULL;		
		if(!itr->second.fetch(&vd)){
			if(__is_voices_fatal_log_on){
				nsox::xlog(NsoxInfo, "user voice stop, %d", itr->first);
			}
			vd ? releaseVoice(vd) : NULL;
			itr = __user_voices.erase( itr );
		}else{		

			if( tail == NULL ){
				tail = vd;		
				head = tail;
			}else{
				tail->next = vd;
				tail = vd;				
			}
			
			if(tail)
				tail->next = NULL;
			++itr;
		}
	}

	return head;
}

XVoiceData* VoiceDataManager::fetch(int nTimeStamp, VoiceType vt)
{
	return fetchVoice(/*__user_voices,*/nTimeStamp, vt);
}


XVoiceData* VoiceDataManager::fetchBroadcast(int nTimeStamp)
{
	return fetchVoice(/*__user_voices,*/ nTimeStamp, using_broadcast);	
}

void VoiceDataManager::push(PBaseSessVoice* vpak)
{
	if  (!__running) 
		return;
	if(vpak->from == 0) return;
	bool bresend = vpak->isResend;

	CAutoLock lock(&__cs);

	voice_map::iterator itr = __user_voices.find(vpak->from);
	if(itr != __user_voices.end()){	
		itr->second.setJitter(getJitter());
		UINT lastseq = itr->second.push(vpak, bresend);
		on_packet_loss(vpak->from, lastseq, vpak->seq);									
	}else{ // no such user 
		VoiceData& vd = __user_voices[vpak->from];
		vd.setJitter(getJitter());
		vd.set_voice_quality(__voice_quality);
		vd.__emptyVoice.from = vpak->from;
		vd.push(vpak, bresend);				
	}	
	/*if (isbroadcast == true) {
			voice_map::iterator itr = __broastcast_voices.find(vpak->from);
			if(itr != __broastcast_voices.end()){	
				UINT lastseq = itr->second.push(vpak, bresend);
				on_packet_loss(vpak->from, lastseq, vpak->seq);									
			}else{ // no such user 
				VoiceData& vd = __broastcast_voices[vpak->from];
				vd.set_voice_quality(NULL);
				vd.__emptyVoice.from = vpak->from;
				vd.push(vpak, bresend);				
			}	
	} else {
			voice_map::iterator itr = __user_voices.find(vpak->from);
			if(itr != __user_voices.end()){	
				UINT lastseq = itr->second.push(vpak, bresend);
				on_packet_loss(vpak->from, lastseq, vpak->seq);									
			}else{ // no such user 
				VoiceData& vd = __user_voices[vpak->from];
				vd.set_voice_quality(__voice_quality);
				vd.__emptyVoice.from = vpak->from;
				vd.push(vpak, bresend);				
			}			
	//}*/
}

void VoiceDataManager::on_packet_loss(const UID_T &user, int fromseq, int toseq)
{
	if(fromseq >= toseq) return;

	int losscount = (toseq - fromseq)/PACKET_SEQ_INCREMENT;
	if(losscount < MAX_LOST_GAP){
		for(int seq =fromseq+ PACKET_SEQ_INCREMENT ; seq < toseq; seq += PACKET_SEQ_INCREMENT)
		{
			if(__is_voices_fatal_log_on){
				nsox::xlog(NsoxInfo, "client packet loss seq : %d, last seq: %d", seq, toseq);
			}
			voice_map::iterator itr = __user_voices.find(user);
			
#ifndef	NO_VOICE_STAT
			if(itr != __user_voices.end()){	
				if (itr->second.isNeedResend(seq)) {
					__packet_notify->on_packet_loss(user, seq); // 数据包丢失
				}
				itr->second.on_pack_loss(seq);
			}
#else
			if(itr != __user_voices.end()){	
				__packet_notify->on_packet_loss(user, seq); // 数据包丢失
			}
#endif
		} 
	}else{
		if(__is_voices_fatal_log_on){
			nsox::xlog(NsoxInfo, "client packet loss too much from: %d, to: %d", fromseq, toseq);
		}
	}
}

void VoiceDataManager::set_packet_notify(IPacketNotifyer* packetNotifyer)
{
	__packet_notify = packetNotifyer;
}

void VoiceDataManager::set_voice_pumper(IVoicePumper* pumper)
{
	__voice_pumper = pumper;
}

void VoiceDataManager::StopAll()
{
	CAutoLock lock(&__cs);
	voice_map::iterator it = __user_voices.begin();
	while(it != __user_voices.end()) {
		if (it->second.stop() == true) {
			if (it->second.total > 100) {
				ReportManager::GetInst()->Report(4, 0, "f=t;%d;%d;%d;%d", it->first, it->second.totalLoss * 100 / it->second.total,it->second.total,it->second.totalLoss);
			}
		}
		++it;
	}
}

void VoiceDataManager::stop(uint32_t uid)
{
	CAutoLock lock(&__cs);
	nsox::xlog(NsoxInfo,"voice data manager stop:%d",uid);		
	//__user_voices[uid].stop();
	voice_map::iterator it = __user_voices.find(uid);
	if (it != __user_voices.end()) {
		if (it->second.stop() == true) {
			if (it->second.total > 100) {
				ReportManager::GetInst()->Report(4, 0, "f=t;%d;%d;%d;%d", it->first, it->second.totalLoss * 100 / it->second.total, it->second.total, it->second.totalLoss);
			}
		}
	}
}

void  VoiceDataManager::pause()
{
	nsox::xlog(NsoxInfo,"voice data manager pause");
	__running = false;
}


void VoiceDataManager:: start()
{
	nsox::xlog(NsoxInfo,"voice data manager start");
	__running = true;
}

void VoiceDataManager::statistics()
{
#ifndef	NO_VOICE_STAT
	//if(__is_voices_fatal_log_on )
	voice_map::iterator itr = __user_voices.begin();
	for(; itr != __user_voices.end(); ++itr){
		//nsox::xlog(NsoxFatal, "voice data statistics uid: %d", itr->first);
		itr->second.log(__is_voices_fatal_log_on);
	}
#endif
	//return;
	CAutoLock lock(&__cs); // TOTEST

	XCast<PUserVoiceStatus> userVoiceStatus;
	if (__ping_counts < 5 ) {
	//	return;
	}

	bool must = false;

	userVoiceStatus.jitter = __jitter;
	userVoiceStatus.ping = __lastPings[__lastPing_index];

	PVoiceStatus voiceStatus;
	voice_map::iterator itrx = __user_voices.begin();

	if (__last_valid_gets_percentage.size() >= 5) {
		__last_valid_gets_percentage.pop_front();
	}

	int gets_percentage_total = 0;
	int valid = 0;

	int succ_total = 0;
	int resend_success = 0;

	for(; itrx != __user_voices.end(); ++itrx){
		if (itrx->second.__statistics.isWork()) {
			voiceStatus.uid = itrx->first;
			const protocol::session::VoiceStatisticsData& vsdata = itrx->second.__statistics.getData();
			voiceStatus.error_percentage = vsdata.error_percentage;
			voiceStatus.succ_percentage = vsdata.succ_percentage;
			voiceStatus.loss_percentage = vsdata.loss_percentage;
			voiceStatus.first_percentage = vsdata.first_percentage;
			voiceStatus.resend_succ_percentage = vsdata.resend_succ_percentage;
			voiceStatus.resend_late_percentage = vsdata.resend_late_percentage;
			voiceStatus.resend_error_percentage = vsdata.resend_error_percentage;
			//voiceStatus.gets_percentage = vsdata.gets_percentage;
			userVoiceStatus.voices.push_back(voiceStatus);
			//must = must && (voiceStatus.first_percentage == 1000); 

			if (vsdata.total > 20 && vsdata.gets_percentage > 0 && vsdata.gets_percentage <= 100) {
				gets_percentage_total += vsdata.gets_percentage;
				succ_total += vsdata.succ_percentage;
				resend_success += vsdata.resend_succ_percentage;
				
				valid++;

				nsox::xlog(NsoxFatal,"gets:%u, succ:%u, resend:%u", vsdata.gets_percentage, vsdata.succ_percentage, vsdata.resend_succ_percentage);
			}
		}
	}

	if (valid) {
		__last_valid_gets_percentage.push_back(gets_percentage_total / valid);
		int pre_succ = succ_total / valid;

		if (__last_valid_gets_percentage.size() >= 3) {
			gets_percentage_total = 0;
			int time = 0;
			int recent;
			for(std::list<int>::reverse_iterator it = __last_valid_gets_percentage.rbegin(); it != __last_valid_gets_percentage.rend(); ++it) {
				if (time == 0) {
					recent = *it;
				}
				gets_percentage_total += *it;
				time++;
			}
			int per_gets = gets_percentage_total / __last_valid_gets_percentage.size();
			
			if (per_gets < 85) { // 平均丢包率超过15%
				__packet_notify->OnPacketLossTooMuch();
			} else if (recent < per_gets && recent < (100 - 6) ) { // 最近一次丢包率变差，而且丢包率超过6%
				__packet_notify->OnPacketLossChangeBad();	
			} else if (recent < (100 - 10)) { // 最近一次丢包率丢包率超过10%
				__packet_notify->OnPacketLossChangeBad();	
			} else {
			}
		}
	}
	//__voice_pumper->OnUserVoiceStatus(userVoiceStatus);
}

void VoiceDataManager::set_voice_quality(IVoiceQuality* quality)
{
	__voice_quality = quality;
}


void VoiceDataManager::pushEmpty(PEmptyChatVoice* vemptypak)
{
#ifndef	NO_VOICE_STAT
	if  (!__running) 
		return;

	if(vemptypak->from == 0) return;

	CAutoLock lock(&__cs);

	voice_map::iterator itr = __user_voices.find(vemptypak->from);
	if(itr != __user_voices.end()){	
		itr->second.pushEmpty(vemptypak);						
	}
#endif
}

void  VoiceDataManager::getImmediateStatus(std::list<VoiceStatisticsData>* status)
{
	CAutoLock lock(&__cs);
	voice_map::iterator itr = __user_voices.begin();
	for(; itr != __user_voices.end(); ++itr){
		status->push_back(itr->second.__statistics.getData());
	}
}


void VoiceDataManager::SetRTT(uint32_t rtt)
{
	//assert(rtt < 1000);
	__lastPing_index = (__lastPing_index + 1) % PING_LOG_NUM;
	__lastPings[__lastPing_index] = rtt;
	__ping_counts++;
	calcJitter();

	if (__ping_counts == PING_LOG_NUM || __ping_counts % (120*2) == 0) 
		ReportManager::GetInst()->Report(3, 0, "f=ns;sid=%d;p=%d;a=%d;j=%d;c=%d", sid_, rtt, ping_average_, __jitter, __ping_counts);
	//getJitter();
}

void  VoiceDataManager::initJitter() {
	for(int i = 0; i < PING_LOG_NUM; ++i) {
		__lastPings[i] = 0;
	}
	__lastPing_index = 0;
	__jitter = 0;
	__ping_counts = 0;
}
void  VoiceDataManager::calcJitter() {
	int max = 0;
	int min = 0;
	ping_average_ = 0;
	int time = 0;
	for(int i = 0; i < PING_LOG_NUM; ++i) {
		int index = (__lastPing_index + PING_LOG_NUM - i) % PING_LOG_NUM;
		int ping = __lastPings[index];
		if (ping == 0) {
			continue;
		}
		max = max(max, ping);
		min = min == 0 ? ping : min(min, ping);
		ping_average_ += ping; 
		time++;
	}
	if (time)
		ping_average_ = ping_average_ / time;
	__jitter = max - min;
}

uint32_t VoiceDataManager::getJitter() 
{
	//nsox::xlog(NsoxDebug, "getJitter() = %d ....", __jitter);

	//char arrLog[128] = {0};
	//sprintf(arrLog, "getJitter() = %d ....\n", __jitter);
	//::OutputDebugStringA(arrLog);

	return __jitter;
}

void VoiceDataManager::SetSid(uint32_t sid) {
	sid_ = sid;
}