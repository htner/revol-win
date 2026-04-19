#pragma once

#include <psapi.h>



class CProcessMetrics
{
protected:
	HANDLE process_;
	int    processor_count_;
	INT64  last_system_time_,last_time_;
public:
	CProcessMetrics()
	{
		process_ = NULL;
		last_system_time_ = last_time_ = 0;

		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		processor_count_ = system_info.dwNumberOfProcessors;
	}
	void AttachProcess(HANDLE handle = NULL){
		if( handle ){
			process_ = handle;
		}else
		{
			process_ = GetCurrentProcess();
		}
	}

public:
	int GetCPUUsage()
	{
		if(process_ == NULL)
			return 0;

		FILETIME now;
		FILETIME creation_time, exit_time;
		FILETIME kernel_time, user_time;

		GetSystemTimeAsFileTime(&now);

		if (!GetProcessTimes(process_, &creation_time, &exit_time,
			&kernel_time, &user_time)) {
				return 0;
		}
		INT64 system_time = (FileTimeToUTC(kernel_time) + FileTimeToUTC(user_time)) /
			processor_count_;
		INT64 time = FileTimeToUTC(now);

		if ((last_system_time_ == 0) || (last_time_ == 0)) {			
			last_system_time_ = system_time;
			last_time_ = time;
			return 0;
		}

		INT64 system_time_delta = system_time - last_system_time_;
		INT64 time_delta = time - last_time_;
		
		if (time_delta == 0)
			return 0;
		
		int cpu = static_cast<int>((system_time_delta * 100 + time_delta / 2) /
			time_delta);

		last_system_time_ = system_time;
		last_time_ = time;

		return cpu;
	}
	int GetWorkingSetSize() const {
		if(process_)
		{
			PROCESS_MEMORY_COUNTERS pmc;
			if (GetProcessMemoryInfo(process_, &pmc, sizeof(pmc))) {
				return pmc.WorkingSetSize;
			}
		}
		return 0;
	}
public:
	static INT64 FileTimeToUTC(const FILETIME& ftime) {
		LARGE_INTEGER li;
		li.LowPart = ftime.dwLowDateTime;
		li.HighPart = ftime.dwHighDateTime;
		return li.QuadPart;
	}

};