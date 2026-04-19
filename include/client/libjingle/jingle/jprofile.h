#ifndef JINGLE_JPROFILE_H_
#define JINGLE_JPROFILE_H_

#include "jingle/jdata.h"

#include <string>

namespace buzz
{
	class JProfile : public JData
	{
	public:
		JProfile();
		JProfile(std::string data);
		virtual ~JProfile();

		const std::string &GetAccount() const;
		void SetAccount(const std::string &account);

		const std::string &GetNickname() const;
		void SetNickname(const std::string &nickname);

		const std::string &GetSignature() const;
		void SetSignature(const std::string &signature);

		int GetGender() const;
		void SetGender(int gender);

		int GetScore() const;
		void SetScore(int score);

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		std::string account_;
		std::string nickname_;
		std::string signature_;
		int gender_;
		int score_;
	};
}

#endif //JINGLE_JRAWDATA_H_
