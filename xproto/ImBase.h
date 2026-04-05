#ifndef PROTOIM_H_
#define PROTOIM_H_

#include "proto/iproto.h"

#include "protocol/pim.h"

namespace xproto {
class CProtoLink;

class CImBase: public IImBase, public XConnPoint<IImEvent> {
public:
	CImBase();

	void setProtoLink(CProtoLink *link);
	virtual LPCSTR getObjectName();

	virtual void send(const std::string &data);
	virtual void onImResponse(const protocol::im::PZImResponse &response);

private:
	CProtoLink *protoLink_;
};
}

#endif

