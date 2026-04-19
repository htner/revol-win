#pragma once


#include <vector>
#include <string>
#include <map>

#pragma comment( lib, "shlwapi" )

namespace helper
{
	class  ufile
	{
	public:
		static bool ReadFileData(const std::wstring& fileName,std::string& data);
		static bool	WriteFileData(const std::wstring& fileName,const std::string& data);
		static std::wstring GetModulePathDir();
		static std::wstring GetFileDirFromPath(LPCTSTR strFilePath);
		static bool IsPicture(const std::string& data);
	};
}

#include "ufile.hpp"