#pragma once
#include <string>
#include "timer/extratimer.h"
#include "proto/jitterbuf.h"

struct EmptyVoice : public IJitterData
{
public:
	virtual int			Seq(){
		return seq;
	}
	virtual UID			From(){
		return "";
	}
	virtual IJitterData* Clone()
	{
		EmptyVoice* xx = new EmptyVoice;
		*xx = *this;
		return xx;
	}
	virtual void		Destroy()
	{
		 delete this;
	}
	int seq;
};
class JitterBuffer;

class VoiceTimeWindow : public ITimerSenseObj
{
public:
		VoiceTimeWindow(JitterBuffer * imp);
		virtual ~VoiceTimeWindow();					
		void    start(int timegap);
		virtual void TimeArrive();
		JitterBuffer* handler;

		UINT			 __offset; //  ±÷”∆Ø“∆
		UINT			 __lasttime;
		int				 __timeGap;
};

struct JitterData {
	   JitterData(){reset();}
	   UINT firstSeq;
	   UINT currentSeq;
	   UINT lastSeq;
	   UINT firstTime;
	   UINT total;
	   UINT totalLoss;
	   UINT totalWait;
	   UINT waitTimes;
	   UINT maxGitterTime;
	   BOOL	 bstop;

	   typedef std::vector<IJitterData*> JDataArray;

	   JDataArray voices;

	   void reset();

	   bool			pop(IJitterPumper* pumper);
	   void			flush(IJitterPumper* pumper);
	   UINT			push(IJitterData* vpak);
	   bool			insert(IJitterData* vpak);
	   void			stop();

	   EmptyVoice	emptyVoice;
	   UID			uFrom;
	   int			timeGap;
};
		
class JitterBuffer : public IJitterBuffer
{
public:
		JitterBuffer(void);
		virtual ~JitterBuffer(void);

		void onTimer();
	
		virtual void SetNotify(IJitterNotify* notify);
		virtual void SetPumper(IJitterPumper* pumper);
		virtual void Push(IJitterData* jdata);
		virtual void Stop(const UID& uid);
		virtual void SetJitterGap(int time);
		virtual void Clear();
		virtual void Destroy();
protected:
		void on_packet_loss(const UID& user, int fromseq, int toseq);
protected:
		typedef std::map<UID, JitterData> voice_map;
		voice_map		 __user_voices;		
		IJitterNotify*	 __packet_notify;
		IJitterPumper*   __voice_pumper;
		VoiceTimeWindow  __timeWnd;		
		int				 __jitterGap;
};
