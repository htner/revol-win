#ifndef JINGLE_JCLIENT_H_
#define JINGLE_JCLIENT_H_

#include "talk/base/messagequeue.h"
#include "talk/base/taskrunner.h"

#include <string>
#include <set>

#include "talk/base/sigslot.h"

#include "talk/xmpp/jid.h"
#include "talk/xmpp/xmppengine.h"

namespace talk_base
{
	class SocketServer;
	class Thread;
	class Task;
}

namespace buzz
{
	class XmppClient;
	class XmppClientSettings;

	class JBlockManager;
	class JData;
	class JFileTransferManager;
	class JHandler;
	class JMessage;
	class JMessageManager;
	class JNotify;
	class JPingManager;
	class JPresence;
	class JPresenceManager;
	class JProfile;
	class JProfileList;
	class JProfileManager;
	class JRoster;
	class JRosterList;
	class JRosterManager;
	class JTransfer;
	class JTransferManager;
	class JTaskManager;
}

namespace buzz
{
	class JClient :
		public sigslot::has_slots<>,
		public talk_base::MessageHandler
	{
	public:
		enum CommandId
		{
			COMMAND_REFRESH_ROSTERS,
			COMMAND_MODIFY_ROSTERS,
			COMMAND_POST_PRESENCE,
			COMMAND_SEND_MESSAGE,
			COMMAND_PING,
			COMMAND_SEND_FILE,
			COMMAND_ACCEPT_FILE,
			COMMAND_REJECT_FILE,
			COMMAND_CANCEL_FILE
		};

		enum NotifyId
		{
			NOTIFY_LOG_INPUT,
			NOTIFY_LOG_OUTPUT,
			NOTIFY_STATE_CHANGE,
			NOTIFY_ROSTER_INIT,
			NOTIFY_ROSTER_UPDATE,
			NOTIFY_PRESENCE,
			NOTIFY_GET_MESSAGE,
			NOTIFY_PROFILE,

			NOTIFY_TRANSFER,
		};

	public:
		JClient(const std::string &jid, const std::string &password, JHandler *handler);
		virtual ~JClient();

		void Exec(const JNotify &notify);
		void Exec(int command, const JData &data);
		void Exec(int command);

		void Do(const JNotify &notify);
		void Do(int command, const JData &data);
		void Do(int command);

		std::string GetJid() const;

	protected:
		void OnMessage(talk_base::Message *message);

		void DoConnect(JNotify *notify);
		void DoDisconnect(JNotify *notify);
		void DoRefreshRosters(JNotify *notify);
		void DoModifyRosters(JNotify *notify);
		void DoPostPresence(JNotify *notify);
		void DoSendMessage(JNotify *notify);
		void DoPing(JNotify *notify);
		void DoSendFile(JNotify *notify);
		void DoAcceptFile(JNotify *notify);
		void DoRejectFile(JNotify *notify);
		void DoCancelFile(JNotify *notify);

		void OnLogInput(const char *data, int bytes);
		void OnLogOutput(const char *data, int bytes);
		void OnStateChange(XmppEngine::State state);
		void OnRosterUpdate(const JRosterList &rosters);
		void OnPresence(const JPresence &roster);
		void OnGetMessage(const JMessage &message);
		void OnTransfer(const JTransfer &transfer);
		void OnProfile(const JProfileList &profiles);

		void Notify(int type, const JData &data);

	private:
		XmppClient *client_;
		std::string jid_;
		std::string password_;
		JHandler *handler_;
		talk_base::Thread *thread_;
		JRosterManager *rosterManager_;
		JPresenceManager *presenceManager_;
		JProfileManager *profileManager_;
		JMessageManager *messageManager_;
		JTransferManager *transferManager_;
		JPingManager *pingManager_;
	};
}

#endif //JINGLE_JCLIENT_H_
