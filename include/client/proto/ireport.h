#ifndef IReport_Protocol_H
#define IReport_Protocol_H
#include "common/unistr.h"
#include "common/int_types.h"
namespace protocol
{
	namespace report
	{
		struct IReport
		{
			virtual void reportLostPack(const UniString& uid, const UniString& sid, uint32_t lostp, uint32_t totalp) = 0;
			virtual void reportSpy1(const std::string &mac, const std::string &localip, bool hasIS, bool hasUT) = 0;

		};
	}
}

#endif