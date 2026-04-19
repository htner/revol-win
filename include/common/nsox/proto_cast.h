#ifndef __SESSCACHED_PROTO_H
#define __SESSCACHED_PROTO_H

#include "nu_request.h"
#include "packet.h"


namespace nproto
{
				inline bool write_request(nproto::connection*c, request* req, int uri)
				{
					if(req == NULL || c == NULL){
						nsox::xlog(NsoxError, "try to  write null request");
						return false;
					}
					try{
						sox::PackBuffer pb;
						sox::Pack pk(pb);

						req->URI = uri;
						req->pack(pk);
						pk.replace_uint32(0, pk.size());

						nsox::xlog(NsoxDebug, "send request, URI:%d", req->URI);
						return c->write_data(pb.data(),pb.size());
					}catch(sox::PackError& e){
						nsox::xlog(NsoxError, "pack error : %s, uri:%d", e.what(), uri);
						return false;
					}

					
				}
				template<class TReq>
				bool write_request(nproto::connection*c, TReq* req)
				{
						return write_request(c,req,TReq::uri);
				}

				template<class TReq>
						bool write_request(nproto::connection*c, TReq& req)
				{					
						return write_request(c, &req);
				}

				
				template<class TRep>
						bool write_response(nproto::connection*c, TRep& rep)
				{
						return  write_request(c,rep);
				}


				template<class T>
				struct proto_cast : public nproto::request
								  , public T
				{
						proto_cast()
						{
								resCode = 200;
								sxid	= 0;
								URI		= 0;
						}
						proto_cast(T& x){
								resCode = 200;
								sxid	= 0;
								URI		= 0;
								copyRequest(x);
						}
						uint16_t resCode, sxid; 
						uint8_t tag;
						std::string key;
						uint32_t sessId;

						virtual void pack(sox::Pack& pk){
								nproto::request::pack(pk);
								pk << sxid << resCode;
								pk.push_uint8(tag);
								T::marshal(pk);
						}
						virtual void unpack(sox::Unpack& upk){
								nproto::request::unpack(upk);
								upk >> sxid >> resCode;
								tag = upk.pop_uint8();
								T::unmarshal(upk);
						}
						void	copyRequest(T& x){
								*(T*)this = x;
						}
				};						

}
#define XCast nproto::proto_cast

#endif
