#ifndef JINGLE_JLIST_H_
#define JINGLE_JLIST_H_

#include "jingle/jlist.h"

#include <list>

namespace buzz
{
	template <typename DataT>
	class JList : public JData
	{
	public:
		JList()
		{
		}

		JList(std::string data)
		{
			Deserialize(data);
		}

		virtual ~JList()
		{
		}

		void Add(const DataT &data)
		{
			dataList_.push_back(data);
		}

		bool HasNext() const
		{
			return !dataList_.empty();
		}

		DataT GetNext()
		{
			DataT data = dataList_.back();
			dataList_.pop_back();
			return data;
		}

	protected:
		virtual int DataType() const
		{
			return 0;
		}

		virtual std::string Serialize() const
		{
			std::string data;
			PushList(dataList_, data);
			return data;
		}

		virtual bool Deserialize(std::string &data)
		{
			PopList(dataList_, data);
			return true;
		}

	protected:
		typedef std::list<DataT> DataList;

	private:
		DataList dataList_;
	};
}

#endif //JINGLE_JLIST_H_