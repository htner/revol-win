#ifndef JINGLE_JMESSAGE_H_
#define JINGLE_JMESSAGE_H_

#include "jingle/jdata.h"

#include <string>

namespace buzz
{
	class JMessage : public JData
	{
	public:
		enum MessageType
		{
			MESSAGE_NORMAL,
			MESSAGE_CHAT,
			MESSAGE_ERROR,
			MESSAGE_GROUPCHAT,
			MESSAGE_HEADLINE
		};

	public:
		JMessage();
		JMessage(int type, const std::string &body, const std::string &jid);
		JMessage(const std::string &type, const std::string &body, const std::string &jid);
		JMessage(std::string data);
		virtual ~JMessage();

		int GetType() const;
		void SetType(int type);

		std::string GetMessageType() const;
		void SetMessageType(const std::string &type);

		std::string GetBody() const;
		void SetBody(const std::string &body);

		std::string GetSubject() const;
		void SetSubject(const std::string &subject);

		std::string GetId() const;
		void SetId(const std::string &id);

		std::string GetJid() const;
		void SetJid(const std::string &jid);

		std::string GetTimestamp() const;
		void SetTimestamp(const std::string &timestamp);

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		int type_;
		std::string body_;
		std::string subject_;
		std::string id_;
		std::string jid_;
		std::string timestamp_;
	};
}

#endif //JINGLE_JMESSAGE_H_
