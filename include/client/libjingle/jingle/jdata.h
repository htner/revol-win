#ifndef JINGLE_JDATA_H_
#define JINGLE_JDATA_H_

#include <string>
#include <list>

namespace buzz
{
	class JData
	{
	public:
		JData();
		virtual ~JData();

		virtual int DataType() const = 0;

		virtual std::string Serialize() const = 0;
		virtual bool Deserialize(std::string &data) = 0;

	protected:
		static void Push(int bytes, const char *value, std::string &data);
		static bool Pop(int bytes, char *value, std::string &data);

		template <typename ValueType>
		static void Push(const ValueType &value, std::string &data)
		{
			Push(sizeof(ValueType), (const char *) &value, data);
		}

		template <typename ValueType>
		static bool Pop(ValueType &value, std::string &data)
		{
			return Pop(sizeof(ValueType), (char *) &value, data);
		}

		template <>
		static void Push<std::string>(const std::string &value, std::string &data)
		{
			Push(value.size(), data);
			Push(value.size(), value.data(), data);
		}

		template<>
		static bool Pop<std::string>(std::string &value, std::string &data)
		{
			size_t size;
			if (!Pop(sizeof(size_t), (char *) &size, data)) {
				return false;
			} else {
				char *pack = new char[size];
				bool succeed = Pop(size, pack, data);
				if (succeed) {
					value = std::string(pack, size);
				}
				delete[] pack;
				return succeed;
			}
		}

		template <>
		static void Push<JData>(const JData &value, std::string &data)
		{
			data.append(value.Serialize());
		}

		template<>
		static bool Pop<JData>(JData &value, std::string &data)
		{
			return value.Deserialize(data);
		}

		template <typename ValueT>
		static void PushList(const std::list<ValueT> &value, std::string &data)
		{
			Push(value.size(), data);
			for (std::list<ValueT>::const_iterator i = value.begin(); i != value.end(); ++i) {
				const ValueT &item = *i;
				if (dynamic_cast<const JData *>(&item) != NULL) {
					Push((JData&) item, data);
				} else {
					Push(item, data);
				}
			}
		}

		template <typename ValueT>
		static void PopList(std::list<ValueT> &value, std::string &data)
		{
			int size = 0;
			Pop(size, data);
			value.clear();
			for (int i = 0; i < size; ++i) {
				ValueT item;
				if (dynamic_cast<JData *>(&item) != NULL) {
					Pop((JData&) item, data);
				} else {
					Pop(item, data);
				}
				value.push_back(item);
			}
		}
	};
}

#endif //JINGLE_JDATA_H_
