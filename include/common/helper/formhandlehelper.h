#ifndef FORMHANDLEHELPER_H_
#define FORMHANDLEHELPER_H_
#include "common/core/ibase.h"
namespace misc {
class VoidFormHandle : public core::IFormHandle {
public:
	virtual void *handlePacket(sox::Unpack &up) {
		return NULL;
	}
	virtual void destroyForm(void *cmd) {
	}
};

template<class T> class FormHandleT : public core::IFormHandle {
public:
	virtual void *handlePacket(sox::Unpack &up) {
		T *obj = new T();
		up >> *obj;
		return obj;
	}
	virtual void destroyForm(void *cmd) {
		T *obj = (T *)cmd;
		if(obj)
			delete obj;
	}

};
}
#endif /*FORMHANDLEHELPER_H_*/
