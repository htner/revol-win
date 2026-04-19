#ifndef _PZIPBASE_H
#define _PZIPBASE_H

//#include <string>
#include "common/packet.h"
#include "common/iproperty.h"
#include "protocol/const.h"
#include <common/compresser.h>

//#ifndef WIN32
//#include "core/sox/logger.h"

//#else
//#include <common/nsox/nulog.h>

//#endif

namespace protocol {
namespace session {

#define LENGTH_ZIP 51200
template<typename PCommand>
class PZipBase :  public PCommand
{
	
public:
		bool 		_bzip;
		PZipBase(){
		}
		virtual void marshal(sox::Pack &p) const {
        			sox::PackBuffer bf;
        			sox::Pack pk(bf);
        			PCommand::marshal(pk);
				std::string compress;
				if (pk.size() >= LENGTH_ZIP && compresser::compress(compress, std::string(pk.data(), pk.size())) )
				{
//#ifndef WIN32
//					sox::log(Info,"begin and after compress, the data size is: ", pk.size()," & " ,compress.size());
//#endif
					p << true;
        				
				} else {
//#ifndef WIN32
//					sox::log(Info, "zipbase not wants to compress, the data size is: ", pk.size());
///#endif
					p << false;
					compress = std::string(pk.data(), pk.size());
				}
				p.push_varstr32(compress.data(), compress.size());
        	}


		virtual void unmarshal(const sox::Unpack &p) {
				p >> _bzip;
				std::string _string;
				_string = p.pop_varstr32();
//#ifndef WIN32
//				sox::log(Info,"network data size is", _string.size());
//#endif
				if (_bzip == true) {
					std::string uncompress;
					if (compresser::uncompress(uncompress, _string)) {
						sox::Unpack up(uncompress.data(), uncompress.size());
						PCommand::unmarshal(up);
//#ifndef WIN32
//						sox::log(Info,"begin and after uncompress, the data size is", _string.size(), uncompress.size());
//#endif
					} else {
//#ifndef WIN32
//						sox::log(Info,"network data uncompress error!");
//#endif
						return;
					}
	
				} else {
//#ifndef WIN32
//					sox::log(Info,"network data is not need uncompress!");
//#endif
					sox::Unpack up(_string.data(), _string .size());
					PCommand::unmarshal(up);
				}
			
		}

};

}
}

#endif
