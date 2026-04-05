#include "infobase.h"
#include "helper/utf8to.h"

using namespace xproto;

InfoBase::InfoBase(void)
{
}
InfoBase::InfoBase(sox::Properties & getter, sox::Properties & setter)
{

}

InfoBase::~InfoBase(void)
{
}
std::string	InfoBase::getBString(uint16_t tag) const
{
	return sox::properties::get(__getter, tag);
}


xstring InfoBase::getString(uint16_t tag) const{
	return net2app(sox::properties::get(__getter, tag));
}

void InfoBase::setString(uint16_t tag,const xstring& str)
{
	if(str !=  getString(tag)){
		sox::properties::replace(__setter, tag, app2net(str));
	}
}
void InfoBase::setBString(uint16_t tag, const std::string& str,bool bForceWrite )
{
	if( bForceWrite ||
		str !=  getBString(tag)){
		sox::properties::replace(__setter, tag, str);
	}
}
int InfoBase::getInt(uint16_t tag) const{
	if (const std::string * val = sox::properties::find(__getter, tag))
	{
		return atoi(val->data());
	}
	return 0;
}
void InfoBase::setInt(uint16_t tag, int i){
	if(getInt(tag) != i){
		char buf[20];
		memset(buf, 0, 20);
		sox::properties::replace(__setter, tag, itoa(i, buf, 10));
	}
}
bool InfoBase::getBool(uint16_t tag) const{
	return sox::properties::getint(__getter, tag) != 0;
}
void InfoBase::setBool(uint16_t tag, bool b){
	if(getBool(tag) != b){
		sox::properties::setint(__setter, tag, b);
	}
}
uint64_t InfoBase::getInt64(uint16_t tag) const 
{
	if (const std::string * val = sox::properties::find(__getter, tag))
		return _atoi64(val->data());
	return 0;
}