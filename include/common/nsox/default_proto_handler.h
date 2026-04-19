#pragma once

#ifndef __DEFAULT_PROTO_HANDLER
#define __DEFAULT_PROTO_HANDLER

#include "nu_request.h"
#include <windows.h>

namespace nproto
{
		class default_proto_handler :
				public proto_handler
		{
		public:

				BEGIN_REQUEST_MAP(default_proto_handler)						
				END_REQUEST_MAP()


				virtual ~default_proto_handler(){}


				int  ondata(const char* data, nsox::nu_size_t len, connection* c)
				{
						int oldlen = len;
						while(true)
						{
								if((len<sizeof(uint32_t)) || (nproto::request::peek_len(data)>len))
								{
										break; //data not enough
								}

								int		plen = nproto::request::peek_len(data);
								uint32_t uri = nproto::request::peek_uri(data);

								// server bug, fix but old version problem
								if (plen == 0) {
									//assert(plen);
									data += sizeof(uint32_t);
									len -= sizeof(uint32_t);
									break;
								}

								if(!process_request(uri, data, plen, c))
								{
									nsox::xlog(NsoxDebug, "this uri has not been processed, uri : %u", uri);
								}

								data += plen;
								len -= plen;
						}						
						return oldlen - len;
				}
				virtual void onclose(int err, connection* c) = 0;
				virtual void oncreate(connection* c) = 0;
		};
}


#endif

