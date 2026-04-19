#ifndef IWATCHER_COMMON_H
#define IWATCHER_COMMON_H
#include "common/unistr.h"
namespace protocol{
namespace uinfo{

struct IUInfoWatcher{
	//virtual void onChange() = 0;
	virtual void onRefresh() = 0;
};
}



}

#endif /*IWATCHER_*/
