#ifndef JINGLE_JSTRING_H_
#define JINGLE_JSTRING_H_

#include "jingle/jdata.h"

#include <string>

namespace buzz
{
	class JString : public JData
	{
	public:
		JString();
		JString(const char *data);
		JString(const char *data, int bytes);
		JString(std::string data);
		virtual ~JString();

		void SetData(const std::string &data);
		const std::string &GetData() const;

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		std::string data_;
	};
}

#endif //JINGLE_JSTRING_H_
