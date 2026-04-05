#include "common/core/request.h"
#include "common/core/ibase.h"
#include "common/res_code.h"
//#include "../core/sox/logger.h"
static std::string zero_place("");

using namespace core;
Request::Request(const Request &r) :
	up(NULL, 0), pb(), pk(pb) {
	length = r.length;
	uri = r.uri;
	resCode = r.resCode;
	sid = r.sid;
	tag = r.tag;
	key = r.key;
	handler = r.handler;
	cmd = r.cmd;
	ctx = r.ctx;
}
Request::Request(const char *data, uint32_t sz) :
	up(data, sz), pb(), pk(pb),  handler(NULL), cmd(NULL), ctx(NULL) {
	length = up.pop_uint32();

	uri = up.pop_uint32();
	sid = up.pop_uint16();
	resCode = up.pop_uint16();

	tag = up.pop_uint8();

	/*
	 sessId = p.pop_uint32();
	 int keylen = p.pop_uint8();
	 key = p.pop_fetch(keylen);*/
}

Request::Request(URI_TYPE uri, sox::Marshallable &m, core::IContext *ctx) :
	up(NULL, 0), pb(), pk(pb), length(0), resCode(RES_SUCCESS), sid(0), tag(0), handler(NULL),
			cmd(NULL) {
	this->uri = uri;
	this->ctx = ctx;

//	sox::log(Debug, "construct request: ", uri);
	basePack();
	
	m.marshal(pk);
}

Request::~Request() {
	if (handler && cmd) {
		handler->destroyForm(cmd);
	}
}
bool Request::setFormHandler(IFormHandle *h) {
	if (h) {
		if(resCode == RES_SUCCESS){
			handler = h;
			cmd = h->handlePacket(up);
		}
		return true;
	}else{
		return false;
	}
}

void Request::forceHandler(IFormHandle *h){
	if (h) {
		handler = h;
		cmd = h->handlePacket(up);
	}else{
		handler = NULL;
	}
}

void Request::basePack() {
	pk.push_uint32(length);

	pk.push_uint32(uri);
	pk.push_uint16(sid);
	pk.push_uint16(resCode);

	pk.push_uint8(tag);
}

Request::ResponseType Request::forward(URI_TYPE uri, 	sox::Marshallable *obj, core::IContext *ctx) {
	this->uri = uri;
	this->ctx = ctx;
	resCode = RES_SUCCESS;
	
	//sox::log(Debug, "forward request: ", uri);

	basePack();
	
	obj->marshal(pk);

	return FORWARD;
}

Request::ResponseType Request::directForward(core::IContext *ctx) {
	this->ctx = ctx;
	//sox::log(Debug, "direct forward request: ", uri);
	basePack();
	pk.push(up.data(), up.size());
	return FORWARD;
}

Request::ResponseType Request::directAnswer(){
	//sox::log(Debug, "direct answer request: ", uri);
	basePack();
	pk.push(up.data(), up.size());
	return ANSWER;
}

Request::ResponseType Request::answer(URI_TYPE uri, uint16_t res,
		sox::Marshallable *obj) {
	this->uri = uri;
	resCode = res;
	basePack();
	if(obj)
		obj->marshal(pk);

	return ANSWER;
}

void Request::endPack(uint16_t sid) {
	if(!key){
		pk << key ;
	}
	
	pk.replace_uint32(0, pb.size());
	pk.replace_uint16(8, sid);
}

void Request::endPack(uint16_t sid, uint32_t uid) {
	pk << uid;
	pk.replace_uint32(0, pb.size());
	pk.replace_uint16(8, sid);
}

const char *Request::data() {
	return pk.data();
}

uint32_t Request::size() {
	return pk.size();
}

uint32_t Request::peeklen(const void * d) {
	uint32_t i = *((uint32_t*)d);
	return XHTONL(i);
}

uint32_t Request::cookieSize() {
	//length uint16, null string 
	return 6;
}

uint32_t Request::getKey(){
	if(key == 0){
		while(!up.empty()){
			up >>  key;
		}
	}
	return key;
}

Request::ResponseType Request::end(){
	return END;
}

bool Request::isSuccess() const{
	return resCode == RES_SUCCESS;
}

void Request::fetchKey(){
	while(!up.empty()){
		up >> key;
	}
}

bool Request::ifSuccess(uint32_t res)
{
	return res == RES_SUCCESS;
}
