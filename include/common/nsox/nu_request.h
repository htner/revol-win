#pragma	   once

#ifndef	__nrequest_h
#define __nrequest_h

#include "int_types.h"
#include "nu_types.h"
#include "packet.h"
#include "selector.h"
#include "nulog.h"

namespace nproto
{
                struct request
                {
                                uint32_t		len;
                                uint32_t		URI; // identify this request

                                virtual ~request(){}

                                static uint32_t peek_len(const char *data){
                                                uint32_t len = *(uint32_t*)data;
                                                return XNTOHL(len);
                                }
                                static uint32_t	 peek_uri(const char* data){
                                                uint32_t URI = *(uint32_t*)(data + sizeof(uint32_t));
                                                return XNTOHL(URI);
                                }
                                virtual void pack(sox::Pack& pack){
                                                pack << len << URI;
                                }
                                virtual void unpack(sox::Unpack& unpack){
                                                unpack >> len >> URI;
                                }
                };

                struct response : public request {
                                uint16_t   rescode;

                                virtual ~response(){}
                                virtual void pack(sox::Pack& pk){
                                                request::pack(pk);
                                                pk << rescode;
                                }
                                virtual void unpack(sox::Unpack& upk){
                                                request::unpack(upk);
                                                upk >> rescode;
                                }
                };

                struct connection : virtual public nsox::IAutoPtr
                {
                                virtual ~connection(){}
                                virtual bool write_data(const char* p, nsox::nu_size_t len) = 0;
                                virtual void close_conn() = 0;
                                virtual bool is_conn_close() = 0;
                                virtual bool is_conn_alive() = 0;
                };

                struct proto_handler
                {
                                virtual ~proto_handler(){}
                                virtual int  ondata(const char* data, nsox::nu_size_t len, connection* c) = 0;
                                virtual void onclose(nsox::error_t err, connection* c) = 0;
                                virtual void oncreate(connection* c) = 0;
                };

}

#define DECLARE_REQUEST_MAP \
                virtual bool   process_request(uint32_t uri, const char* data, nsox::nu_size_t len, nproto::connection* c);

#define  BEGIN_REQUEST_MAP(theClass)\
                virtual bool   process_request(uint32_t uri, const char* data, nsox::nu_size_t len, nproto::connection* c)\
                 {

#define  BEGIN_REQUEST_MAP_EXT(theClass)\
                bool  theClass::process_request(uint32_t uri, const char* data, nsox::nu_size_t len, nproto::connection* c)\
                {

#define  REQUEST_HANDLER(Proto, func) \
                if(uri == Proto::uri){\
                                Proto pro; sox::Unpack upk(data,len);	\
                                try{pro.unpack(upk);}catch(sox::UnpackError& err){ \
                                                nsox::xlog(NsoxDebug, "unpack request error , uri:%d", uri);\
                                }\
                                func(pro, c); return true;\
                }

#define  END_REQUEST_MAP()\
                                return false; \
                }

#define  CHAIN_REQUEST_MAP_MEMBER(theChainMember) \
                if(theChainMember.process_request(uri,data,len,c)) return true;

#define  CHAIN_REQUEST_MAP_PTR_MEMBER(theChainMember) \
                if(theChainMember->process_request(uri,data,len,c)) return true;
#endif

