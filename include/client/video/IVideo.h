#ifndef __IVIDEO_H__
#define __IVIDEO_H__
#include <xstring>



class IVideo{
public:
	enum VideoQualityType{
		kNormalQuality, //流畅 320x240
		kHightDefine,   //高清 480x360
		kBestQuality    //超清640*360
	};

/************************************************************************/
// 开始视频主播
// @param url              视频上传URL
// @param videoDeviceName  视频设备名
// @param qualityType      视频分辨率设置，具体查看
/************************************************************************/
	virtual bool startVideo(const wchar_t* url,
				const wchar_t* videoDeviceName, 
				int qualityType) = 0;
/************************************************************************/
/* 停止视频主播                                                         */
/************************************************************************/	
	virtual void stopVideo() = 0;
/************************************************************************/
// 传递语音的包序号，用于语音视频同步                                   
// @param seq  传递语音的包序号
/************************************************************************/
	virtual void setAudioSeq(unsigned int seq) = 0;

/************************************************************************/
// 代发送RTMP语音包,语音包为HEAAC编码后数据
// @data  编码后数据指针
// @size  数据长度
/************************************************************************/
	virtual void pushAudioData(const char* data, int size) = 0;
/************************************************************************
// 销毁对象
************************************************************************/
	virtual void destroy() = 0;
};

IVideo* createVideo();


#endif //__IVIDEO_CAP_H__
