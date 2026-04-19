#ifndef JINGLE_JFILEOFFER_H_
#define JINGLE_JFILEOFFER_H_

#include "jingle/jdata.h"
#include "jingle/jhandler.h"

namespace buzz
{
	class JFileOffer : public JData
	{
	public:
		JFileOffer();
		JFileOffer(const std::string &jid, const std::string &file, int offerId);
		JFileOffer(std::string data);
		virtual ~JFileOffer();

		std::string GetFile() const;
		std::string GetJid() const;
		int GetOfferId() const;

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		std::string jid_;
		std::string file_;
		int size_;
		int offerId_;
	};
}

#endif //JINGLE_JFILEOFFER_H_
