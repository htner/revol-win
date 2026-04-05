#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <common/xstring.h>

enum InputDevice
{	
		NONE = 0,
		IN_MICROPHONE,
		IN_WAVEOUT,
		IN_HEADPHONE,
		OUT_MICROPHONE,
		OUT_SPEAKER
};

class CMixer
{
public:
		CMixer(void);
		~CMixer(void);
public:
		/*
		* 函数名称：CMixer::Create
		* 函数描述：创建mixer
		* 输入参数：dev, 设备的名称
		* 输出参数：void
		* 返回值  ：创建成功返回true;否则返回false
		*/
		virtual bool Create(InputDevice dev, int nMixerID);

		/*
		* 函数名称：CMixer::Close
		* 函数描述：关闭mixer
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：viod
		*/
		virtual void Close(void);

		/*
		* 函数名称：CMixer::SetVolume
		* 函数描述：设置mixer的音量
		* 输入参数：vol,音量的问题
		* 输出参数：void
		* 返回值  ：成功返回true;否则返回false
		*/
		bool SetVolume(unsigned vol);

		/*
		* 函数名称：CMixer::GetVolume
		* 函数描述：获以mixer的音量
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：返回mixer的音量
		*/
		int GetVolume(void);

		/*
		* 函数名称：CMixer::SetMute
		* 函数描述：设置静音
		* 输入参数：mute,是否静音
		* 输出参数：void
		* 返回值  ：成功返回true;否则返回false
		*/
		bool SetMute(bool mute);
		bool SetMute2(bool mute);

		/*
		* 函数名称：CMixer::GetMute
		* 函数描述：获取是否静音
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：返回是否成功
		*/
		bool GetMute(void);
		bool GetMute2();
protected:
		bool GetMuteData(MIXERCONTROLDETAILS& mxcd, MIXERCONTROLDETAILS_BOOLEAN* val);

		bool DeviceToType(InputDevice dev, DWORD& compoment, DWORD& mixer);

		bool GetVolumeData(MIXERCONTROLDETAILS& mxcd, MIXERCONTROLDETAILS_UNSIGNED* mxcd_u);

		MIXERLINE GetLineStru(void)
		{
				return m_sub_line;
		}
private:
		bool IsMonoMixerByName(const xstring& mixername);
		bool IsStereoMixerByName(const  xstring& mixername);
protected:
		bool IsMicByName(const xstring& mixername);
		HMIXER			m_hMixer;

		InputDevice		m_InputDevice;

		MIXERLINE		m_line;

		MIXERLINE		m_sub_line;

		MIXERLINE		m_monoline;
};

class CMixer_Input : public CMixer
{
public:
		CMixer_Input(void);
		~CMixer_Input(void);
public:
		/*
		* 函数名称：CMixer_Input::Create
		* 函数描述：创建mixer
		* 输入参数：dev, 设备的名称
		* 输出参数：void
		* 返回值  ：创建成功返回true;否则返回false
		*/
		virtual bool Create(InputDevice dev,int nMixerID);
public:
		/*
		* 函数名称：CMixer_Input::GetVolume
		* 函数描述：获以mixer的音量
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：返回mixer的音量
		*/
		bool Select(void)
		{
				return CheckMutex();
		}

		bool CanSelect(void)
		{
			MIXERCONTROL					mxc;
			MIXERCONTROLDETAILS				mxcd;
			MIXERCONTROLDETAILS_BOOLEAN*	pmutex = GetMutexData(mxc, mxcd);
			if(NULL == pmutex || m_mutex_index == -1)
				return false;

			return true;
		}
		/*
		* 函数名称：CMixer::GetVolume
		* 函数描述：获以mixer的音量
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：返回mixer的音量
		*/
		bool GetSelected(void)
		{
				return GetMutex();
		}
protected:
		/*
		* 函数名称：CMixer::CheckMutex
		* 函数描述：选中互斥量
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：成功返回true，操作有效;否则返回false,操作无效
		*/
		bool CheckMutex(void);

		/*
		* 函数名称：CMixer::GetMutex
		* 函数描述：检查互斥量
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：返回互斥量
		*/
		bool GetMutex(void);
private:
		MIXERCONTROLDETAILS_BOOLEAN* GetMutexData(MIXERCONTROL& mxc, MIXERCONTROLDETAILS& mxcd);
protected:
		int				m_mutex_index;
};

class CMicrophoneEnhance
{
public:
		CMicrophoneEnhance();
		~CMicrophoneEnhance();
public:
		/*
		* 函数名称：CMicrophoneEnhance::SetEnhance
		* 函数描述：设置麦增强
		* 输入参数：enhance,是否启用麦加强
		* 输出参数：void
		* 返回值  ：设置成功返回true;否则返回false
		*/
		bool SetEnhance(bool enhance);
};

class CMicrophone : public CMixer_Input
{
public:
		CMicrophone();
		~CMicrophone();
public:
		/*
		* 函数名称：CMicrophone::Create
		* 函数描述：创建麦克风
		* 输入参数：void
		* 输出参数：void
		* 返回值  ：设置成功返回true;否则返回false
		*/
		virtual bool Create(int nMixerID);
		virtual	bool IsClosed();
		virtual void Close(void);
private:
	//	bool innerCreate();
		BOOL m_bClosed;
};