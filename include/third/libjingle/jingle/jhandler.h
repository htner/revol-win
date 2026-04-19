#ifndef JINGLE_JHANDLER_H_
#define JINGLE_JHANDLER_H_

namespace buzz
{
	class JNotify;
}

namespace buzz
{
	class JHandler
	{
	public:
		JHandler();
		virtual ~JHandler();

		virtual void OnNotify(const JNotify &notify) = 0;
	};
}

#endif //JINGLE_JHANDLER_H_
