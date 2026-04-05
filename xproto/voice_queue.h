#ifndef __RTALK_VOICE_QUEUE_H__
#define __RTALK_VOICE_QUEUE_H__
#include <string>
//#include "protocol/prtalk.h"
#include "proto/iproto.h"
#include "common/nsox/nu_request.h"
#include "helper/helper.h"

namespace protocol {
namespace session {

struct PNormalVoiceData : public nproto::request {
    uint32_t seq;
    std::string data;
    virtual void pack(sox::Pack& pk) {
		nproto::request::pack(pk);
        pk.push_uint16(0);  //sid
		pk.push_uint16(200);   // rescode
		pk.push_uint8(0);   // tag
        pk.push(data.data(),data.size());
    }
	virtual void unpack(sox::Unpack& upk) {
		assert(0);
    }
};

struct OutputVoiceQueue {
    public:
        typedef std::list<PNormalVoiceData> QueueVoice;
        OutputVoiceQueue();
        ~OutputVoiceQueue();
        int Push(const char* voice, int size, uint32_t seq, uint32_t uri);
        PNormalVoiceData* Get(int seq);
        void End();
    private:
        QueueVoice queue_voice_;
		CRITICAL_SECTION critical_section_;
};

} // end namespace rtalk
}
#endif //__DATA_QUEUE_H__
