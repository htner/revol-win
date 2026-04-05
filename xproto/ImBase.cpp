#include "ImBase.h"
#include "Login.h"

#include "xconnpoint.h"
#include "common/nsox/proto_cast.h"
#include "proto/proto_obj_name.h"

#include "protocol/pforward.h"
#include "protocol/pim.h"

#include "ProtoLink.h"

using namespace protocol::im;
using namespace protocol::link;

namespace xproto {
CImBase::CImBase() {
}

void CImBase::setProtoLink(CProtoLink *link) {
	protoLink_ = link;
}

LPCSTR CImBase::getObjectName() {
	return IM_OBJ;
}

void CImBase::send(const std::string &data) {
	XCast<protocol::im::PZImRequest> imRequest;
	imRequest.data = data;
	XCast<PForward> forward;
	forward.cmd;
	sox::PackBuffer pb;
	sox::Pack pk(pb);

	imRequest.URI = imRequest.uri;

	imRequest.pack(pk);
	forward.cmd.assign(pb.data(), pb.size());
	protoLink_->writeRequest(&forward, forward.uri);
}

void CImBase::onImResponse(const PZImResponse &response) {
	XConnPoint<IImEvent>::forEachWatcher1(&IImEvent::onImResponse, response.data);
}
}

