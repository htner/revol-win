#ifndef JINGLE_JGROUPLIST_H_
#define JINGLE_JGROUPLIST_H_

#include "jingle/jdata.h"

#include <string>

#include "jingle/jstring.h"
#include "jingle/jlist.h"

namespace buzz
{
	class JGroupList : public JList<JString>
	{
	public:
		JGroupList();
		JGroupList(std::string data);
		virtual ~JGroupList();
	};
}

#endif //JINGLE_JGROUPLIST_H_
