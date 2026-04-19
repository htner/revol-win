#ifndef JINGLE_JTRANSFER_H_
#define JINGLE_JTRANSFER_H_

#include "jingle/jdata.h"

#include <string>

namespace buzz
{
	class JTransfer : public JData
	{
	public:
		enum TransferType
		{
			TYPE_FILE_TRANSFER
		};

		enum TransferStatus
		{
			TRANSFER_OFFER,
			TRANSFER_START,
			TRANSFER_COMPLETE,
			TRANSFER_PRGRESS,
			TRANSFER_LOCAL_CANCEL,
			TRANSFER_REMOTE_CANCEL,
			TRANSFER_FAILURE,
			TRANSFER_FINISH
		};

	public:
		JTransfer();
		JTransfer(int type, int id);
		JTransfer(int type, int id, const std::string &name, int status);
		JTransfer(std::string data);
		virtual ~JTransfer();

		int GetId() const;
		void SetId(int id);

		int GetType() const;
		void SetType(int type);

		size_t GetProgress() const;
		void SetProgress(size_t progress);

		size_t GetTotal() const;
		void SetTotal(size_t total);

		int GetStatus() const;
		void SetStatus(int status);
		
		int GetOfferId() const;
		void SetOfferId(int offerId);

		std::string GetJid() const;
		void SetJid(const std::string &jid);

		std::string GetName() const;
		void SetName(const std::string &name);

	protected:
		int DataType() const;
		std::string Serialize() const;
		bool Deserialize(std::string &data);

	private:
		int id_;
		int type_;
		int progress_;
		int total_;
		int status_;
		int offerId_;
		std::string jid_;
		std::string name_;
	};
}

#endif //JINGLE_JTRANSFER_H_
