#pragma once
#include "helper/utf8to.h"
#include "boostlite/format.hpp"
class GoogleAnalyticsUrl
{
public:
	enum HIT_TYPE{HIT_PAGEVIEW = 0,HIT_EVENT,HIT_TIMING};
public:
	GoogleAnalyticsUrl()
	{		
		_timingValue = -1;
		_nPageLoadTime = -1;
		_nPageDownloadTime = -1;
	}
	void setHitType(HIT_TYPE hitType)
	{
		//Example value: pageview
		//Example usage: t=pageview
		//The type of hit. Must be one of 'pageview', 'screenview', 'event', 'transaction', 'item', 'social', 'exception', 'timing'.
		_hitType = hitType;

	}
	void SetTrackingID(LPCTSTR lpTrackingID)
	{
		//Example value: UA-XXXX-Y
		//Example usage: tid=UA-XXXX-Y
		_trackID = lpTrackingID;
	}
	void SetClientID(LPCTSTR lpClientID)
	{
		//Example value: 35009a79-1a05-49d7-b876-2b884d0f825b
		//Example usage: cid=35009a79-1a05-49d7-b876-2b884d0f825b
		_clientID = lpClientID;
	}
	void SetDocLocationUrl(LPCTSTR url)
	{
		_docLocationUrl = url;
		//dl = url;
		//dh = hostname;
		//dp = host path;
	}
	void SetDocumentTitle(LPCTSTR title)
	{
		_title = title;
		//Example usage: dt=Settings
	}
	void SetPageLoadTime(int slaps)
	{
		_nPageLoadTime = slaps;
		//plt
	}
	void SetPageDownloadTime(int slaps)
	{
		_nPageDownloadTime = slaps;
		//pdt
	}
	void SetCampaignName(LPCTSTR campaignName){
		_campaignName = campaignName;
	}
	void SetCampaignSource(LPCTSTR campaignSource){
		_campaignSource = campaignSource;
	}
	void SetCampaignMedium(LPCTSTR campaignMedium){
		_campaignMedium = campaignMedium;
	}
	void SetCampaignKeyword(LPCTSTR campaignKeyword){
		_campaignKeyword = campaignKeyword;
	}
	void SetTimingCategory(LPCWSTR tCategory){
		_timingCat = tCategory;
	}
	void SetTimingVar(LPCWSTR tVar){
		_timingVar = tVar;
	}
	void SetTimingVlaue(int t){
		_timingValue = t;
	}
	void SetTimingLabel(LPCWSTR tLabel){
		_timingLable = tLabel;
	}
	void SetEventParam(LPCWSTR category,LPCTSTR action,LPCWSTR label = NULL,LPCWSTR lpValue = NULL)
	{
		//Example usage: ec=Category
		//Example usage: ea=Action
		//Example usage: el=Label
		//Example usage: ev=55
		
		std::wstring ec = helper::urlencode(category) ;
		std::wstring ea = helper::urlencode(action);
		xstring param = format(L"ec=%s&ea=%s", ec.c_str(), ea.c_str());
		if(label)
		{
			param += L"&el=";
			param += helper::urlencode(label);
		}
		if(lpValue)
		{
			param += L"&ev=";
			param += helper::urlencode(lpValue);
		}
		_eventParam = param;
	}
	std::wstring GetGAUrl()
	{
		//http://www.google-analytics.com/collect?
		//v=1
		//ul=en-us
		//sr=800x600
		//ua=Opera%2F9.80%20%28Windows%20NT%206.0%29%20Presto%2F2.12.388%20Version%2F12.14
		//z=289372387623
		static std::wstring g_langid;
		if(g_langid.empty())
		{
			WCHAR szName[100]; 
			LCID lcid = GetSystemDefaultLCID();		
			if ( 0 != GetLocaleInfo( lcid , LOCALE_SENGLANGUAGE, szName, 100 )) 
				g_langid = szName;
			else
				g_langid = L"unknow";
		}
		
		static std::wstring g_screen;
		if(g_screen.empty())
		{
			int cx = GetSystemMetrics(SM_CXSCREEN);
			int cy = GetSystemMetrics(SM_CYSCREEN);
			g_screen = format(L"%dx%d", cx ,cy);
		}

		std::wstring hitType;		
		switch(_hitType)
		{
		case HIT_PAGEVIEW:	hitType = L"pageview";	break;
		case HIT_EVENT:		hitType = L"event";		break;
		case HIT_TIMING:	hitType = L"timing";	break;
		default:
			return L"";
		}

		std::wstring dl = helper::urlencode(_docLocationUrl);
		std::wstring ul = helper::urlencode(g_langid);
		std::wstring url = L"http://www.google-analytics.com/collect?";
		std::wstring param = format(L"v=1&t=%s&tid=%s&cid=%s&dl=%s&ul=%s&sr=%s",
														hitType.c_str() , _trackID.c_str() , _clientID.c_str(),
														dl.c_str(), 
														ul.c_str(), g_screen.c_str());
				
		if(!_campaignName.empty())
		{
			xstring t = helper::net2app(helper::urlencode(helper::app2net(_campaignName)));
			param += format(L"&cn=%s", t.c_str());
		}
		if(!_campaignSource.empty())
		{
			xstring t = helper::net2app(helper::urlencode(helper::app2net(_campaignSource)));
			param += format(L"&cs=%s", t.c_str());
		}
		if(!_campaignMedium.empty())
		{
			xstring t = helper::net2app(helper::urlencode(helper::app2net(_campaignMedium)));
			param += format(L"&cm=%s", t.c_str());
		}
		if(!_campaignKeyword.empty())
		{
			xstring t = helper::net2app(helper::urlencode(helper::app2net(_campaignKeyword)));
			param += format(L"&ck=%s", t.c_str());
		}
		if(_nPageLoadTime >= 0)
		{
			param += format(L"&plt=%d", _nPageLoadTime);
		}
		if(_nPageDownloadTime >= 0){
			param += format(L"&pdt=%d", _nPageDownloadTime);
		}
		if(!_timingCat.empty()){
			xstring t = helper::net2app(helper::urlencode(helper::app2net(_timingCat)));
			param += format(L"&utc=%s", t.c_str());
		}
		if(!_timingVar.empty()){
			xstring t = helper::net2app(helper::urlencode(helper::app2net(_timingVar)));
			param += format(L"&utv=%s", t.c_str());
		}
		if(!_timingLable.empty()){
			xstring t = helper::net2app(helper::urlencode(helper::app2net(_timingLable)));
			param += format(L"&utl=%s", t.c_str());
		}
		if(_timingValue > 0){
			param += format(L"&utt=%d", _timingValue);
		}
		if(!_eventParam.empty())
		{
			param += format(L"&%s",_eventParam.c_str());	
		}else
		{
			if(!_title.empty()){
				xstring t = helper::net2app(helper::urlencode(helper::app2net(_title)));
				param += format(L"&dt=%s",t.c_str());	
			}
		}

		int randNum = rand();
		param += format(L"&z=%d" , randNum);

		std::wstring f = url + param;
		return f;
	}
protected:
	std::wstring format(LPCWSTR fmt,...)
	{
		WCHAR* szBuffer = new WCHAR[1024];

		va_list argp;
		va_start(argp,fmt);				
		wvsprintf(szBuffer,fmt,argp);
		va_end(argp);

		std::wstring s = szBuffer;
		delete[] szBuffer;
		return s;
	}
protected:
	HIT_TYPE	  _hitType;
	int			  _nPageLoadTime,_nPageDownloadTime;
	std::wstring  _trackID;
	std::wstring  _clientID;
	std::wstring  _docLocationUrl;
	std::wstring  _eventParam;
	std::wstring  _title;
	std::wstring  _campaignName,_campaignSource,_campaignMedium,_campaignKeyword;
	std::wstring  _timingCat,_timingVar,_timingLable;
	int			  _timingValue;
};