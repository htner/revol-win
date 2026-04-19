#ifndef	__LXTHREAD_H
#define	__LXTHREAD_H



class LXThread 
{
public:
	LXThread() {
		__quiting = false;
		_handle = CreateThread(0, 0, ThreadProc, this, CREATE_SUSPENDED, &_tid); 		
	}

	LXThread(LPTHREAD_START_ROUTINE pProc, void *arg) {
		__quiting = false;
		_handle = CreateThread(0, 0, pProc, arg, CREATE_SUSPENDED, &_tid); 		
	}

	virtual ~LXThread() {		
		CloseHandle(_handle);
	}

	void Close(DWORD timeout = 5000) {
		if(!__quiting)
		{
				__quiting = true;
				WaitForSingleObject(_handle,timeout);
		}
	}

	bool IsQuiting() {
		return __quiting;
	}

	DWORD Resume() {
		return ResumeThread(_handle);
	}

	DWORD ID() {
		return _tid;
	}

	HANDLE Handle() {
		return _handle;
	}

	virtual void Run() {}

	static DWORD WINAPI ThreadProc(LPVOID lpArg) {
		LXThread *th = static_cast<LXThread *>(lpArg);

		th->Run();
		th->Close(0);
		return 0;
	}
protected:
	void quit()
	{
			__quiting = true;
	}

private:
	HANDLE _handle;
	DWORD _tid;
	bool __quiting;
};

class LXCS 
{
public:
	LXCS()
	{ InitializeCriticalSection(&_CS); }
	~LXCS()
	{ DeleteCriticalSection(&_CS); }

	void Lock()
	{ /*TRACE("LXCS lock %d\n", ::GetCurrentThreadId());*/EnterCriticalSection(&_CS); }
	void UnLock()
	{ /*TRACE("LXCS unlock %d\n", ::GetCurrentThreadId());*/LeaveCriticalSection(&_CS); }

private:
	CRITICAL_SECTION _CS;
};

class LXAutoCS
{
public:
	LXAutoCS(LXCS *cs)
	{
		_cs = cs;
		_cs->Lock();
	}

	~LXAutoCS()
	{_cs->UnLock();}

private:
	LXCS	*_cs;
};

#endif	//ifndef __LXTHREAD_H