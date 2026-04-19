#pragma	  once

typedef uint32_t UID;

struct IJitterData
{
	virtual int					Seq()		= 0;
	virtual UID					From()		= 0;
	virtual IJitterData*		Clone()		= 0;
	virtual void				Destroy()	= 0;
};

struct IJitterPumper {	
	virtual void on_pump_data(IJitterData* voice)			= 0;
	virtual void on_empty_data(const UID& from)				= 0;
	virtual void on_stop_data(const UID& from)				= 0;
};

struct  IJitterNotify
{
	virtual void on_packet_loss(const std::string& user, int seq) = 0;
};
struct IJitterBuffer
{
	virtual void SetNotify(IJitterNotify* notify)  =0;
	virtual void SetPumper(IJitterPumper* pumper)  =0;
	virtual void Push(IJitterData* jdata)		   = 0;
	virtual void Stop(const UID& uid)			   = 0;
	virtual void Clear()						   = 0;
	virtual void Destroy()						   = 0;
	virtual void SetJitterGap(int time)			   = 0;
};

extern "C"
{
		__declspec(dllexport) IJitterBuffer* CoCreateJitterBuf();
}	