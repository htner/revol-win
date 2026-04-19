#ifndef JINGLE_JROSTERLIST_H_
#define JINGLE_JROSTERLIST_H_

#include "jingle/jdata.h"

#include <string>

#include "jingle/jroster.h"
#include "jingle/jlist.h"

namespace buzz
{
	class JRosterList : public JList<JRoster>
	{
	public:
		JRosterList();
		JRosterList(std::string data);
		virtual ~JRosterList();
	};
}

#endif //JINGLE_JROSTERLIST_H_
