#ifndef TSERVICEMANAGER_H_
#define TSERVICEMANAGER_H_
#include <map>
#include "helper/RefObjectBase.h"
#include "helper/utf8to.h"

template<class I> struct TServiceFactory{
	virtual I *create(const xstring &uid) = 0;
	virtual void destroy(I *) = 0;
};

template <class I> class TServiceManager{
protected:
	typedef nsox::nu_auto_ptr<I>	type_ptr;
	typedef std::map<std::string, type_ptr> service_t;
	service_t services;
	TServiceFactory<I> *factory;
public:
	~TServiceManager(){
		for(service_t::iterator it = services.begin(); it != services.end(); ++it){
			//assert(false);
			factory->destroy(it->second);
		}
		services.clear();
	}
	I *getService(const xstring &uid){
		std::string au = app2net(uid.data()); 
		service_t::iterator it = services.find(au);
		if(it != services.end()){
			return it->second;
		}else{
			type_ptr obj = factory->create(uid);
			services[au] = obj;
			return obj;
		}
	}
	
	void setFactory(TServiceFactory<I> *f){
		factory = f;
	}
	
	void releaseService(I *inter){
		for(service_t::iterator it = services.begin(); it != services.end(); ++it){
			if(it->second == inter){
				inter->release();
				if(inter == NULL){
					services.erase(it);
				}
				return;
			}
		}
	}
};

#endif /*TSERVICEMANAGER_H_*/
