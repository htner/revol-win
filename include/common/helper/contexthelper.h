#ifndef CONTEXTHELPER_H_
#define CONTEXTHELPER_H_
#include "common/core/ibase.h"
namespace misc {
template<class H> class OwnPtrCtx : public core::IContext {
	H * rh;
public:
	OwnPtrCtx(H *handle) :
		rh(handle) {
	}
	~OwnPtrCtx() {
		if (rh != NULL)
			delete rh;
	}

	virtual void* getHandler() {
		return rh;
	}

};
}
#endif /*CONTEXTHELPER_H_*/
