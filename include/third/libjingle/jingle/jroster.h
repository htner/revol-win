#ifndef JINGLE_JROSTER_H_
#define JINGLE_JROSTER_H_

#include "jingle/jdata.h"

#include <string>
#include <list>

#include "talk/xmpp/jid.h"

#include "jingle/jstring.h"
#include "jingle/jgrouplist.h"

namespace buzz
{
	class JRoster : public JData
	{
	public:
		enum SubscriptionType
		{
			SUBSCRIPTION_NONE,
			SUBSCRIPTION_TO,
			SUBSCRIPTION_FROM,
			SUBSCRIPTION_BOTH,
			SUBSCRIPTION_REMOVE
		};

		enum AskType
		{
			ASK_NONE,
			ASK_SUBSCRIBE,
			ASK_UNSUBSCRIBE
		};

	public:
		JRoster();
		JRoster(const std::string &jid, int subscription);
		JRoster(const std::string &jid, const std::string &subscription);
		JRoster(std::string data);
		virtual ~JRoster();

		std::string GetJid() const;
		void SetJid(const std::string &jid);

		std::string GetName() const;
		void SetName(const std::string &name);

		int GetSubsType() const;
		void SetSubsType(int subscription);

		std::string GetSubscription() const;
		void SetSubscription(const std::string &name);

		JGroupList GetGroups() const;
		void SetGroups(const JGroupList &groups);

		int GetAskType() const;
		void SetAskType(int ask);

		std::string GetAsk() const;
		void SetAsk(const std::string &ask);

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		int subscription_;
		int ask_;
		std::string jid_;
		std::string name_;
		JGroupList groups_;
	};
}

#endif //JINGLE_JROSTER_H_
