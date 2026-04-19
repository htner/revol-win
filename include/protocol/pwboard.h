#ifndef PWbObjectList_H_
#define PWbObjectList_H_

#include "common/packet.h"
#include "common/iproperty.h"
#include "protocol/const.h"
#include "core/base_svid.h"

namespace protocol{
	namespace wb{
//add by neou,support WbObjectList

typedef std::pair<std::string,std::string>  WBObject;

struct WbObjectList : public sox::Marshallable
					 , public std::vector<WBObject>
{
		virtual void marshal(sox::Pack &p) const {
			sox::marshal_container(p, *this);
		}
		virtual void unmarshal(const sox::Unpack &up) {
			sox::unmarshal_container(up, std::back_inserter(*this));
		}

};

struct POpenWBoard : public nproto::request{
	enum{uri = (protocol::WB_SVID << 16 | 10)};
	virtual ~POpenWBoard(){}


	std::string uid;
	uint32_t	sid;
	uint32_t	ssid;

	
	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << uid << sid << ssid;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> uid >> sid >> ssid;
	}
};

struct POpenWBoardRes : public nproto::request{
	enum{uri = (protocol::WB_SVID << 16 | 12)};
	virtual ~POpenWBoardRes(){}

	WbObjectList  wb;

	
	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << wb;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> wb;
	}
};


struct PUpdateWbObject : public nproto::request
{
	enum{uri = (protocol::WB_SVID << 16 | 14)};
	virtual ~PUpdateWbObject(){}

	WbObjectList  wb;
	uint32_t   role;
		
	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << wb << role;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> wb >> role;
	}
};

struct POnWbObjectUpdated : public nproto::request
{
	enum{uri = (protocol::WB_SVID << 16 | 16)};
	virtual ~POnWbObjectUpdated(){}


	WbObjectList  wb;	
	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << wb;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> wb;
	}
};

struct PDeleteWbObject :  public PUpdateWbObject
{
	enum{uri = (protocol::WB_SVID << 16 | 18)};
	PDeleteWbObject(){clear=0;}
	virtual ~PDeleteWbObject(){}
	uint16_t clear;
	uint32_t role;

	virtual void pack(sox::Pack& pk){
			PUpdateWbObject::pack(pk);
			pk << clear << role;
	}
	virtual void unpack(sox::Unpack& upk){
			PUpdateWbObject::unpack(upk);
			upk >> clear >> role;
	}
};

struct POnWbObjectDeleted :  public POnWbObjectUpdated
{
	enum{uri = (protocol::WB_SVID << 16 | 20)};
	virtual ~POnWbObjectDeleted(){}


	uint16_t clear;

	virtual void pack(sox::Pack& pk){
			POnWbObjectUpdated::pack(pk);
			pk << clear;
	}
	virtual void unpack(sox::Unpack& upk){
			POnWbObjectUpdated::unpack(upk);
			upk >> clear;
	}
};


struct PCloseWBoard : public nproto::request
{
	enum{uri = (protocol::WB_SVID << 16 | 22)};
	virtual ~PCloseWBoard(){}

};


struct PLockWBoard : public nproto::request
{
	enum{uri = (protocol::WB_SVID << 16 | 24)};
	virtual ~PLockWBoard(){}

	uint32_t   role;

	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << role;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> role;
	}
};

struct PUnLockWBoard : public nproto::request
{
	enum{uri = (protocol::WB_SVID << 16 | 26)};
	virtual ~PUnLockWBoard(){}

	uint32_t   role;

	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << role;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> role;
	}
};

struct POnWBoardLocked : public nproto::request
{
	enum{uri = (protocol::WB_SVID << 16 | 28)};
	virtual ~POnWBoardLocked(){}
	std::string uid;

	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << uid;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> uid;
	}
};

struct POnWBoardUnLocked : public nproto::request
{
	enum{uri = (protocol::WB_SVID << 16 | 30)};
	virtual ~POnWBoardUnLocked(){}
	std::string uid;

	virtual void pack(sox::Pack& pk){
			nproto::request::pack(pk);
			pk << uid;
	}
	virtual void unpack(sox::Unpack& upk){
			nproto::request::unpack(upk);
			upk >> uid;
	}
};

}}

#endif //PWbObjectList_H_