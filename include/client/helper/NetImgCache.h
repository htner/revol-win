#pragma once

#include <xgdi/xskin.h>
#include <common/xstring.h>
#include <xgdi/xgdiutil.h>
#include <map>
#include "helper/ushell.h"
#include "helper/ufile.h"
#include "helper/uencrypt.h"

class NetImgCache
{
public:
	static xstring getImg(const xstring& url,const xstring& account)
	{
		//xstring imgPath = helper::ushell::GetTempPath() + _T("\\rex\\");
		xstring imgPath  = helper::ushell::getRexConfPath()+account;
		imgPath += _T("\\");
		imgPath += helper::net2app(helper::encrypt::MakeMD5(helper::app2net(url)));
		if(::PathFileExists(imgPath.c_str())){
			return imgPath;
		}else{
			return _T("");
		}
	}
	static void	setImg(const xstring& url, const xstring& account,
				   const std::string& data)
	{
		//xstring imgPath = helper::ushell::GetTempPath() + _T("\\rex\\");

		xstring imgPath  = helper::ushell::getRexConfPath()+account;
		imgPath += _T("\\");
		helper::ushell::CreateDirectoryEx(imgPath.c_str());

		imgPath += helper::net2app(helper::encrypt::MakeMD5(helper::app2net(url)));
		helper::ufile::WriteFileData(imgPath, data);
	}
};