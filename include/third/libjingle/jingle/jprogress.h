#ifndef JINGLE_JPROGRESS_H_
#define JINGLE_JPROGRESS_H_

namespace buzz
{
	class JProgress : public JData
	{
	public:
		JProgress(int progress, int total, int id);
		JProgress(const std::string &data);
		virtual ~JProgress();

		int GetProgress() const;
		int GetTotal() const;
		int GetId() const;

	protected:
		virtual int DataType() const;
		virtual std::string Serialize() const;
		virtual bool Deserialize(std::string &data);

	private:
		int progress_;
		int total_;
		int id_;
	};
}

#endif
