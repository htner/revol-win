#include "voice_queue.h"
#include "common/nsox/proto_cast.h"
#include "proto/isession.h"
#define MAX_VOICECHAT_BYTE_SIZE 900  // < 1024
#define QUEUE_SIZE 60




namespace protocol {
namespace session {



OutputVoiceQueue::OutputVoiceQueue() 
{
	InitializeCriticalSection(&critical_section_);
}

OutputVoiceQueue::~OutputVoiceQueue(){
	DeleteCriticalSection(&critical_section_);
}
int OutputVoiceQueue::Push(const char* voice, int size, uint32_t seq, uint32_t uri)
{
	CAutoLock auto_lock(&critical_section_);
    if (size > MAX_VOICECHAT_BYTE_SIZE){
		return protocol::session::ISession::VOICE_OVERFLOW;
    }
    PNormalVoiceData voice_data;
    voice_data.URI = uri;
    voice_data.seq = seq;
    voice_data.data.assign(voice, size);
    if(queue_voice_.size() >= QUEUE_SIZE) {			
        queue_voice_.pop_front();						
    }	
    queue_voice_.push_back(voice_data);
    return protocol::session::ISession::SENDED;
}

PNormalVoiceData* OutputVoiceQueue::Get(int seq) {
	CAutoLock auto_lock(&critical_section_);
	QueueVoice::iterator it = queue_voice_.begin();
    for(; it != queue_voice_.end(); ++it) {
        if(it->seq > (uint32_t) seq){
            return NULL;
        } else if (it->seq == (uint32_t)seq) {
            return &(*(it));
        }
	}
	return NULL;
}

void OutputVoiceQueue::End(){
	CAutoLock auto_lock(&critical_section_);
	queue_voice_.clear();
}

}//end namespacep rtalk

}