#pragma	  once
#include "client/xconnpoint.h"
#include <protocol/const.h>
#include <client/macros.h>
#include <common/nsox/selector.h>
#include <common/xstring.h>
#include "iproto.h"

#include <string>
#include <vector>


namespace protocol{
 
namespace session
{
	enum WbStatus
	{
		WBS_INIT	= 0,
		WBS_OPENING,
		WBS_OPENED,
		WBS_CLOSED 
	};

	
	typedef std::pair<xstring,std::string>	CWBObject;
	typedef std::vector<CWBObject>			CWBObjList;
	typedef std::vector<xstring>			CWBObjNames;

	struct IWBoardEvent : public xproto::IXObject
	{
			virtual void onStatus(WbStatus status)  = 0;
			virtual void onUpdated(const CWBObjList& list) = 0;
			virtual void onDeleted(const CWBObjNames& name) = 0;
			virtual void onCleard()	= 0;
			virtual void onLocked()	= 0;
			virtual void onUnLocked()	= 0;
	};

	

	struct IWBoard : public xproto::IXObject
				   , virtual public IConnPoint<IWBoardEvent>
	{
			//打开当前频道白板
			virtual void Open()							= 0;
			//关闭当前频道白板
			virtual void Close()						= 0;
			//更新白板对象
			virtual void Update(CWBObjList& values)		= 0;
			//删除白板对象
			virtual void Delete(CWBObjNames& names)		= 0;

			virtual void Lock()							= 0;
			virtual void UnLock()						= 0;
			virtual bool Get(const xstring& name, std::string& value) = 0;
			virtual void Refresh()						= 0;
			virtual void Clear()						= 0;
	};	
}}

