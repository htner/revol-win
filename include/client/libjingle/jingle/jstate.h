#ifndef JINGLE_JSTATE_H_
#define JINGLE_JSTATE_H_

#include "jingle/jdata.h"

#include <string>

#include "talk/xmpp/xmppengine.h"

namespace buzz
{
	class JState : public JData
	{
	public:
		enum JingleState
		{
			STATE_NONE = XmppEngine::STATE_NONE,
			STATE_START = XmppEngine::STATE_START,
			STATE_OPENING = XmppEngine::STATE_OPENING,
			STATE_OPEN = XmppEngine::STATE_OPEN,
			STATE_CLOSED = XmppEngine::STATE_CLOSED
		};

	public:
		JState();
		JState(int state);
		JState(std::string data);
		virtual ~JState();

		int GetState() const;

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		int state_;
	};
}

#endif //JINGLE_JSTATE_H_
