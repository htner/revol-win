#ifndef JINGLE_JPRESENCE_H_
#define JINGLE_JPRESENCE_H_

#include "jingle/jdata.h"

#include <string>

namespace buzz
{
	class JPresence : public JData
	{
	public:
		enum Priority
		{
			PRIORITY_MIN = -128,
			PRIORITY_DEFAULT = 0,
			PRIORITY_MAX = 127
		};

		enum PresenceType
		{
			PRESENCE_AVAILABLE,
			PRESENCE_UNAVAILABLE,
			PRESENCE_SUBSCRIBE,
			PRESENCE_SUBSCRIBED,
			PRESENCE_UNSUBSCRIBE,
			PRESENCE_UNSUBSCRIBED,
			PRESENCE_PROBE,
			PRESENCE_ERROR
		};

		enum ShowType
		{
			SHOW_NORMAL,
			SHOW_AWAY,
			SHOW_CHAT,
			SHOW_DND,
			SHOW_XA
		};

	public:
		JPresence();
		JPresence(int type, const std::string &jid);
		JPresence(const std::string &type, const std::string &jid);
		JPresence(std::string data);
		~JPresence();

		int GetPresenceType() const;
		void SetPresenceType(int type);

		std::string GetPresence() const;
		void SetPresence(const std::string &type);

		std::string GetJid() const;
		void SetJid(const std::string &jid);

		int GetShowType() const;
		void SetShowType(int show);

		std::string GetShow() const;
		void SetShow(const std::string &show);

		std::string GetStatus() const;
		void SetStatus(const std::string &status);

		int GetPriority() const;
		void SetPriority(int priority);

		bool IsAvailable() const;

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		int type_;
		std::string jid_;
		std::string status_;
		int show_;
		int priority_;
	};
}

#endif //JINGLE_JPRESENCE_H_
