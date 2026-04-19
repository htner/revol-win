#ifndef JINGLE_JLOGDATA_H_
#define JINGLE_JLOGDATA_H_

#include "jingle/jstring.h"

#include <string>

namespace buzz
{
	class JLogData : public JString
	{
	public:
		JLogData();
		JLogData(const char *data);
		JLogData(const char *data, int bytes);
		JLogData(std::string data);
		virtual ~JLogData();

		const std::string &GetLog() const;
	};
}

#endif //JINGLE_JLOGDATA_H_
