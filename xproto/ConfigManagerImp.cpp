#include "configmanagerimp.h"
#include <shlwapi.h>
#include <Objbase.h>
#include <windows.h>
#include "ConfigImp.h"
#include "atlbase.h"
#include "common/nsox/nuautoptr.h"
#include "helper/ufile.h"

using namespace xproto;

ConfigManagerImp::ConfigManagerImp(void)
{
	setFactory(this);
}

ConfigManagerImp::~ConfigManagerImp(void)
{
}


struct IStorageOper{
	virtual void onError() = 0;
	virtual bool onOpenStorage(IStorage *storage) = 0;
	virtual bool onOpenStream(IStorage *a, IStream *s) = 0;
	virtual bool createStorageOnFileNotFound() = 0;
	virtual bool createStreamOnFileNotFound() = 0;
};

#define HR_ERROR(hr, oper) if(hr < 0){oper.onError(); return;}

void stroageHelper(const xstring &uid, IStorageOper& oper, const xstring &path = _T("")){
	xstring file;
	if (path == _T("")) {
		file = getRexConfPath().append(L"config.cfg");
	} else {
		file = path + _T("config.cfg");
	}

	CComPtr<IStorage> pStorage;

	HRESULT hr = StgOpenStorage(file.data(), NULL,STGM_READWRITE|STGM_SHARE_EXCLUSIVE,NULL, 0, &pStorage);

	if (hr == STG_E_FILENOTFOUND){
		hr = StgCreateDocfile(file.data(),STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,0, &pStorage);
	}
	HR_ERROR(hr, oper);

	if(oper.onOpenStorage(pStorage)){
		pStorage->Commit(STGC_DEFAULT);
	}

	CComPtr<IStorage> sub;
	hr = pStorage->OpenStorage(uid.data(), NULL, STGM_READWRITE|STGM_SHARE_EXCLUSIVE, NULL, 0, &sub); //STGM_CREATE
	if(STG_E_FILENOTFOUND == hr && oper.createStorageOnFileNotFound()){
		hr = pStorage->CreateStorage(uid.data(),STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,0,0,&sub);
	}

	HR_ERROR(hr, oper);

	CComPtr<IStream> pStream;

	hr = sub->OpenStream(L"config",NULL, STGM_READWRITE|STGM_SHARE_EXCLUSIVE, 0, &pStream);//STGM_CREATE
	if(STG_E_FILENOTFOUND == hr && oper.createStreamOnFileNotFound()){
		hr = sub->CreateStream(L"config",STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,0,0,&pStream);
	}

	HR_ERROR(hr, oper);

	oper.onOpenStream(sub, pStream);
	pStorage->Commit(STGC_DEFAULT);
}


class CreateOper: public IStorageOper{
	IInnerConfigManager *manager;
	std::string uid;
public:
	ConfigImp* m_spConfig;

	CreateOper(IInnerConfigManager *manager, const xstring& id){
		this->manager = manager;
		uid = app2net(id.data());

		sox::Properties prop;
		m_spConfig = nsox::nu_create_object<ConfigImp>::create_inst();
		m_spConfig->SetUid(app2net(id));
		m_spConfig->SetManager(manager);
	}
	virtual void onError(){

	}
	virtual bool onOpenStorage(IStorage *storage){
		return false;
	}
	virtual bool onOpenStream(IStorage *a, IStream *s){
		try{
				char buf[1024];
				std::string data;
				ULONG l = 1024;
				while(l == 1024){
					if(s->Read(buf, ULONG(1024), &l) < 0)	return false;
					data.append(buf, size_t(l));
				}
				sox::Properties prop;
				if(data.length() > 0){
					prop.unmarshal(sox::Unpack(data.data(), data.size()));
					m_spConfig->replaceProperty(prop);
				}
		}catch(sox::UnpackError& e)
		{
			sox::Properties prop;
			m_spConfig->replaceProperty(prop);
			return false;
		}	
		return true;
	}
	virtual bool createStorageOnFileNotFound(){
		return false;
	}
	virtual bool createStreamOnFileNotFound(){
		return false;
	}
};

IConfig *ConfigManagerImp::create(const xstring &uid){
	CreateOper op(this, uid);
	stroageHelper(uid, op, __path);
	return op.m_spConfig;
}


IConfig *ConfigManagerImp::getConfig(const xstring &uid)
{
	return getService(uid);
}

class RemoveOper: public IStorageOper{
	xstring uid;
public:
	RemoveOper(const xstring& u): uid(u){}
	virtual void onError(){
		return;
	}
	virtual bool onOpenStorage(IStorage *storage){
		storage->DestroyElement(uid.data());
		return true;
	}
	virtual bool onOpenStream(IStorage *a, IStream *s){
		return false;
	}
	virtual bool createStorageOnFileNotFound(){
		return false;
	}
	virtual bool createStreamOnFileNotFound(){
		return false;
	}
};
int ConfigManagerImp::removeConfig(const xstring &uid){
	std::string id = app2net(uid.data());
	service_t::iterator it = services.find(id);
	if(it != services.end()){
		ConfigImp *imp = (ConfigImp *)(IConfig *)(it->second);
		imp->replaceProperty(sox::Properties());
		return 1;
	}else{
		RemoveOper op( uid);
		stroageHelper(uid, op, __path);
		return 0;
	}
}
void ConfigManagerImp::releaseConfig(IConfig *config){
	releaseService(config);
}

class SaveOper: public IStorageOper{
public:
	const sox::Properties &prop;
	int res;
	SaveOper(const sox::Properties& p): prop(p){res = 0;}
	virtual void onError(){
		res = -1;
		return;
	}
	virtual bool onOpenStorage(IStorage *storage){
		return false;
	}
	virtual bool onOpenStream(IStorage *a, IStream *s){
		try{
			sox::PackBuffer pb;
			sox::Pack pk(pb);
			prop.marshal(pk);
			ULONG l = 0;
			s->Write(pk.data(), pk.size(), &l);
		}catch(sox::PackError& err){
			return false;
		}			
		return true;
	}
	virtual bool createStorageOnFileNotFound(){
		return true;
	}
	virtual bool createStreamOnFileNotFound(){
		return true;
	}
};

int ConfigManagerImp::saveConfig(const xstring &uid, const sox::Properties& config){
	SaveOper save(config);
	stroageHelper(uid, save, __path);
	return save.res;
}
void ConfigManagerImp::destroy(IConfig *config){
}

void ConfigManagerImp::delConfig(const xstring& uid)
{
		removeConfig(uid);
}

void ConfigManagerImp::setPath(const xstring& path)
{
		__path = path;
}