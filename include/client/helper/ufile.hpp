#include "stdafx.h"

#include "mfile.h"
#include <shlwapi.h>



namespace helper
{
	inline std::wstring ufile::GetFileDirFromPath(LPCTSTR filepath)
	{
		unsigned int orgLen = _tcslen(filepath);

		TCHAR end = filepath[ orgLen - 1];

		if(end !=_T('\\') && end != _T(':'))
		{
			LPCTSTR FN = PathFindFileName(filepath);

			return std::wstring(filepath, orgLen -  _tcslen(FN));
		} 
		else
		{
			return std::wstring(filepath);
		}
	}

	inline std::wstring ufile::GetModulePathDir()
	{
		TCHAR	buffer[MAX_PATH];
		ZeroMemory(buffer, sizeof(TCHAR) * MAX_PATH);
		::GetModuleFileName(NULL, buffer, MAX_PATH);

		return GetFileDirFromPath(buffer);
	}

	inline bool ufile::ReadFileData(const std::wstring& fileName,std::string& data)
	{
		dspeak::util::FileMapping fm;

		if(!fm.open(fileName.c_str())) return false;

		data.clear();

		data.assign((char*)fm.data(),fm.size());

		return TRUE;
	}

	inline bool ufile::WriteFileData(const std::wstring& fileName,const std::string& data)
	{
		FILE *fp;
		if((fp = _tfopen(fileName.c_str(),_TEXT("wb+"))) != NULL){
			fwrite(data.data(), sizeof(std::string::traits_type::char_type), data.size(), fp);
			fclose(fp);
			return TRUE;
		}else 
			return FALSE;
	}

	inline bool ufile::IsPicture(const std::string& data)
	{
		unsigned char h[6];
		memcpy(h, data.c_str(),  5);
		h[5] = '\0';
		//GIF8
		if (h[0] == 71 && h[1] == 73 && h[2] == 70 && h[3] == 56)
			return TRUE;

		//89 PNG
		if (h[0] == 137 && h[1] == 80 && h[2] == 78 && h[3] == 71)
			return TRUE;

		//FFD8
		if (h[0] == 255 && h[1] == 216)
			return TRUE;

		return FALSE;

	}
}