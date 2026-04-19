#ifndef JINGLE_JNOTIFY_H_
#define JINGLE_JNOTIFY_H_

#include "talk/base/messagequeue.h"

#include <string>

namespace buzz
{
	class JData;
}

namespace buzz
{
	class JNotify : public talk_base::MessageData
	{
	public:
		JNotify(int type, const JData &data);
		virtual ~JNotify();

		int GetType() const;

		template <typename DataType>
		DataType GetJingleData() const
		{
			//return (DataType &) data_;
			return DataType(data_);
		}

	private:
		int type_;
		//const JData &data_;
		std::string data_;
	};
}

#endif //JINGLE_JNOTIFY_H_
