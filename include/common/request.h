#ifndef IREQUEST_H_
#define IREQUEST_H_
#include "common/packet.h"
#include "common/int_types.h"
#include "common/socketinc.h"

namespace core {
	struct IFormHandle;
	struct IContext;
	class Request{
		sox::Unpack up;
		sox::PackBuffer pb;
		sox::Pack pk;
	public:
		uint32_t length;
		URI_TYPE uri;
		uint16_t resCode, sid;
		uint8_t tag;

		uint32_t key;	
				
		IFormHandle *handler;
		void *cmd;
		
		IContext *ctx;

		enum ResponseType {
				ANSWER,
				FORWARD,
				REDIRECT,
				END,
				BROADCAST,
				STOP
			};
			
		int connectType;	
	public:
		static uint32_t peeklen(const void * d);

		Request(const Request&);

		Request(const char *data, uint32_t sz);
		
		Request(URI_TYPE uri, sox::Marshallable &m, IContext *ctx);

		Request(URI_TYPE uri, int resCode, sox::Marshallable &m);
		

		~Request();
		
		bool setFormHandler(IFormHandle *h);
		
		void forceHandler(IFormHandle *h);

		URI_TYPE getUri() const {
			return uri;
		}
		void setUri(URI_TYPE u) {
			uri = u;
		}

		uint16_t getResCode() const {
			return resCode;
		}

		bool isSuccess() const;

		void setResCode(uint16_t r) {
			resCode = r;
		}

		uint16_t getSid() const {
			return sid;
		}
		void setSid(uint16_t s) {
			sid = s;
		}
		void setKey(uint32_t uid) {
			key = uid;
		}
		uint32_t getKey();

		void *getCmd() const{
			return cmd;
		}
		void setCmd(void *c){
			cmd = c;
		}
		IContext *getCtx() const{
			return ctx;
		}

		void setCtx(IContext *c){
			ctx = c;
		}

		ResponseType forward(URI_TYPE uri, sox::Marshallable *, IContext *ctx);
				
		ResponseType answer(URI_TYPE uri, uint16_t res, sox::Marshallable *obj);
		//for linkd
		ResponseType directForward(core::IContext *);
		ResponseType directAnswer();
		
		void setConnType(int tp){
			connectType = tp;
		}

		int getConnType() const{
			return connectType;
		}

		static ResponseType end();
		
		void endPack(uint16_t s);
				
		void endPack(uint16_t s, uint32_t uid);
		
		const char *data();
		
		uint32_t size();

		void fetchKey();

		static bool ifSuccess(uint32_t);
	protected:
		uint32_t cookieSize();
		
		void basePack();
	};
}
#endif /*IREQUEST_H_*/
