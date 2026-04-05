#include "configimp.h"
#include "IConfigManager.h"
#include "helper/utf8to.h"
using namespace xproto;

ConfigImp::ConfigImp()
{
}

ConfigImp::~ConfigImp(void)
{
}
void	ConfigImp::SetUid(const std::string& u)
{
		uid = u;
}
void	ConfigImp::SetManager(IInnerConfigManager* m)
{
		manager = m;
}


sox::Pack & operator << (sox::Pack &p, int i){
	return p.push_uint32((uint32_t)i);
}


const sox::Unpack & operator >> (const sox::Unpack &p, long &i){
	i = (long)p.pop_uint32();
	return p;
}

sox::Pack &operator <<(sox::Pack &p,const RECT &rect){
	return p << rect.top << rect.left << rect.bottom << rect.right;
}

sox::Pack &operator << (sox::Pack &p, const POINT &pt){
	return p << pt.x << pt.y;
}

const sox::Unpack &operator >>(const sox::Unpack &p, RECT &rect){
	return p >> rect.top >> rect.left >> rect.bottom >> rect.right;
}

const sox::Unpack &operator >>(const sox::Unpack &p, POINT &pt){
	return p >> pt.x >> pt.y;
}

int ConfigImp::saveConfig(){
	xstring mid;
	manager->saveConfig(net2app(uid, mid), config);
	forEachWatcher0(&IConfigEvent::onConfigChanged);
	return 0;
}

int ConfigImp::getInt(int tag, int _default){
	return sox::properties::getint(config, tag, _default);
}

void ConfigImp::setInt(int tag, int value){
	sox::properties::setint(config, tag, value);
}

void ConfigImp::getStr(int tag, xstring & out){
	net2app(sox::properties::get(config, tag), out);
}
void ConfigImp::setStr(int tag, const xstring &str){
	sox::properties::replace(config, tag, app2net(str.data()), true);
}

void ConfigImp::getRect(int tag, RECT & rect){
	std::string str = sox::properties::get(config, tag);
	if(str.length() == 0)
		return ;
	sox::Unpack up(str.data(), str.size());
	up >> rect;
}
void ConfigImp::setRect(int tag, const RECT & rect){
	sox::PackBuffer pb;
	sox::Pack pk(pb);
	pk << rect;
	sox::properties::replace(config, tag, std::string(pk.data(), pk.size()));
}

void ConfigImp::getPoint(int tag, POINT & point){
	std::string str = sox::properties::get(config, tag);
	if(str.length() == 0)
		return ;
	sox::Unpack up(str.data(), str.size());
	up >> point;
}

void ConfigImp::setPoint(int tag, const POINT & point){
	sox::PackBuffer pb;
	sox::Pack pk(pb);
	pk << point;
	sox::properties::replace(config, tag, std::string(pk.data(), pk.size()));
}

std::string ConfigImp::getBin(int tag, const std::string & _default){
	std::string str = sox::properties::get(config, tag);
	if(str.empty())
		str = _default;
	return str;
}

void ConfigImp::setBin(int tag, const std::string & bin){
	sox::properties::replace(config, tag, bin);
}

bool ConfigImp::getList(int tag, std::vector<xstring>& strlist)
{
	std::string str = getBin(tag, "");

	if(!str.empty())
	{
		try{
			sox::Unpack upk(str.data(), str.size());;
			sox::unmarshal_container(upk, std::back_inserter(strlist));
		}catch(sox::UnpackError& err){
			return false;
		}			
	}
	return true;
}
bool ConfigImp::setList(int tag, const std::vector<xstring>& strlist)
{
	//if(strlist.empty()) return true;

	try{
		sox::PackBuffer pb;
		sox::Pack pk(pb);
		sox::marshal_container(pk, strlist);
		sox::properties::replace(config, tag, std::string(pk.data(), pk.size()));
		return true;
	}catch(sox::PackError& e)
	{
		return false;
	}	
}

bool ConfigImp::getMap(int tag, std::map<xstring,std::string>& map)
{
	std::string str = getBin(tag, "");
	if(!str.empty())
	{
		try{
			sox::Unpack upk(str.data(), str.size());;
			sox::unmarshal_container(upk, sox::properties::replace_iterator(map));
		}catch(sox::UnpackError& err){
			return false;
		}			
	}
	return true;
}
bool ConfigImp::setMap(int tag, std::map<xstring,std::string>& map)
{
	//if(map.empty()) return true;

	try{
		sox::PackBuffer pb;
		sox::Pack pk(pb);
		sox::marshal_container(pk, map);
		sox::properties::replace(config, tag, std::string(pk.data(), pk.size()));
		return true;
	}catch(sox::PackError& e)
	{
		return false;
	}	
}

void ConfigImp::release(){
	manager->releaseConfig(this);
}

void ConfigImp::replaceProperty(const sox::Properties &p){
	sox::properties::replace(config, p);
}