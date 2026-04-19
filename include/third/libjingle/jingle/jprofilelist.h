#ifndef JINGLE_JPROFILELIST_H_
#define JINGLE_JPROFILELIST_H_

#include "jingle/jdata.h"

#include <string>

#include "jingle/jprofile.h"
#include "jingle/jlist.h"

namespace buzz
{
	class JProfileList : public JList<JProfile>
	{
	public:
		JProfileList();
		JProfileList(std::string data);
		virtual ~JProfileList();
	};
}

#endif //JINGLE_JPROFILELIST_H_
