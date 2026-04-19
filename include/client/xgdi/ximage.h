#pragma once
#include "xgdi.h"
#include "xgdidef.h"
#include "xrgbhsl.h"

#include <GdiPlus.h>
#include <vector>
#include <helper/ufile.h>

NAMESPACE_BEGIN(xgdi)

using namespace Gdiplus;



class CBitmapDC : public CDC
{
public:
	// Data members
	HDC m_hDCOriginal;
	SIZE m_szPaint;
	CBitmap m_bmp;
	HBITMAP m_hBmpOld;

	// Constructor/destructor

	CBitmapDC(HDC hDC, HBITMAP hBitMap) :  m_hDCOriginal(hDC), m_hBmpOld(NULL){
		
		m_bmp.Attach(hBitMap);
		m_bmp.GetSize(m_szPaint);

		CreateCompatibleDC(m_hDCOriginal);
		ATLASSERT(m_hDC != NULL);

		m_hBmpOld = CDC::SelectBitmap(m_bmp);
	}
	CBitmapDC(HDC hDC, SIZE& szPaint,BOOL bCompatible = TRUE) : m_hDCOriginal(hDC), m_hBmpOld(NULL)
	{
		m_szPaint = szPaint;
		
		if(bCompatible) 
			m_bmp.CreateCompatibleBitmap(m_hDCOriginal, m_szPaint.cx, m_szPaint.cy);	
		else
		{
			BITMAPINFO lpbmi = {0};
			BYTE* pBackgrounData=NULL;

			lpbmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
			lpbmi.bmiHeader.biPlanes = 1;   
			lpbmi.bmiHeader.biCompression = BI_RGB;   
			lpbmi.bmiHeader.biClrImportant = 0;   
			lpbmi.bmiHeader.biSizeImage = 0;   
			lpbmi.bmiHeader.biClrUsed = 0;   
			lpbmi.bmiHeader.biBitCount = 32;   
			lpbmi.bmiHeader.biWidth = m_szPaint.cx;   
			lpbmi.bmiHeader.biHeight = m_szPaint.cy;   

			m_bmp = CreateDIBSection(NULL, &lpbmi, DIB_RGB_COLORS, (void**)pBackgrounData, NULL, 0);
		}
		ATLASSERT(m_bmp.m_hBitmap != NULL);

		CreateCompatibleDC(m_hDCOriginal);
		ATLASSERT(m_hDC != NULL);

		m_hBmpOld = CDC::SelectBitmap(m_bmp);
	}
	HBITMAP Detach()
	{
		CDC::SelectBitmap(m_hBmpOld);
		m_hBmpOld = NULL;
		return m_bmp.Detach();
	}
	~CBitmapDC()
	{
		if(m_hBmpOld){
			CDC::SelectBitmap(m_hBmpOld);
		}		
	}
};



template<bool t_bManaged>
class CImageT
	: implement IImage
{
public:
	CImageT(HBITMAP hBitmap = NULL)
	{
		Init();
		if(hBitmap)
		{           
			BITMAP bm = { 0 };
			if(::GetObject(hBitmap, sizeof(BITMAP),&bm) == sizeof(BITMAP)){
				m_vtFrames.push_back(hBitmap);
				m_sizeImage.cx = bm.bmWidth;
				m_sizeImage.cy = bm.bmHeight;
			}	
		}
	}

	CImageT(LPCTSTR lpszFile)
	{
		Init();
		LoadImage(lpszFile);
	}

	void Init()
	{
		m_nFrameElapse  = 0;
		m_nCurrentFrame = 0;
		m_sizeImage.cx	= 0;
		m_sizeImage.cy	= 0;
		m_QualityBitmap = NULL;
		m_bColorize = FALSE;
		m_bChangeHsl = FALSE;
		m_crColorize = 0;
		m_fImagefac = 0.50;
		m_crClip = RGB(255,0,255);
	}

	CImageT(HDC hDC, const CRect& rcCapture)
	{
		m_nFrameElapse  = 0;
		m_nCurrentFrame = 0;
		m_sizeImage.cx	= rcCapture.Width();
		m_sizeImage.cy	= rcCapture.Height();

		m_bColorize = FALSE;
		m_bChangeHsl = FALSE;
		m_crColorize = 0;
		m_fImagefac = 0.50;
		m_crClip = RGB(255,0,255);

		if(HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, rcCapture.Width(), rcCapture.Height()))
		{
			HDC hMemDC = ::CreateCompatibleDC(hDC);
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, (HGDIOBJ)hBitmap);
			::BitBlt(
				hMemDC, 0, 0, rcCapture.Width(), rcCapture.Height(),
				hDC, rcCapture.left, rcCapture.top, SRCCOPY);

			hBitmap = (HBITMAP)::SelectObject(hMemDC, (HGDIOBJ)hOldBitmap);
			::DeleteDC(hMemDC);

			m_vtFrames.push_back(hBitmap);
		}
	}

	~CImageT()
	{
		if(t_bManaged)
		{
			DeleteObject();
		}
	}

	IImage* Scale(int cx, int cy){

		SIZE szNew = {cx,cy};

		HDC hDC = ::GetWindowDC(::GetDesktopWindow());

		CBitmapDC oldImg(hDC,this->GetUIHandle());
		CBitmapDC newImg(hDC,szNew,FALSE);

#if 1
		BLENDFUNCTION blend;
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.AlphaFormat = AC_SRC_ALPHA;
		blend.SourceConstantAlpha = 255;
		BOOL bRet = ::AlphaBlend(newImg, 0, 0, cx, cy, 
			oldImg, 0, 0, m_sizeImage.cx, m_sizeImage.cy, 
			blend);
#else
		SetStretchBltMode(newImg, STRETCH_HALFTONE); 
		::StretchBlt(
			newImg, 0, 0, cx, cy,
			oldImg, 0, 0, m_sizeImage.cx, m_sizeImage.cy,
			SRCCOPY);
#endif

		oldImg.Detach();

		::ReleaseDC(::GetDesktopWindow(),hDC);
		return new CImageT<TRUE>(newImg.Detach());
	}

	CImageT<t_bManaged> operator =(HBITMAP hBitmap)
	{
		Attach(hBitmap);
		return *this;
	}

	operator HBITMAP() const
	{
		return (m_nCurrentFrame < m_vtFrames.size()) ? m_vtFrames[m_nCurrentFrame] : NULL;
	}

	BOOL DeleteObject()
	{
		if(t_bManaged)
		{
			std::vector<HBITMAP>::iterator iter = m_vtFrames.begin();
			for(; iter != m_vtFrames.end(); iter++)
			{
				::DeleteObject((HGDIOBJ)*iter);
			}
			m_vtFrames.clear();
			m_nFrameElapse  = 0;
			m_nCurrentFrame = 0;
			if(m_QualityBitmap)
			{
				delete m_QualityBitmap;
				m_QualityBitmap = NULL;
			}
		}

		return TRUE;
	}

	BOOL Attach(const UI_HANDLE& hBitmap)
	{
		if(t_bManaged && !m_vtFrames.empty() && m_vtFrames[m_nCurrentFrame] != NULL && m_vtFrames[m_nCurrentFrame] != hBitmap)
			::DeleteObject(m_vtFrames[m_nCurrentFrame]);

		if(!m_vtFrames.empty())
		{
			m_vtFrames[m_nCurrentFrame] = hBitmap;
		}
		else
		{
			m_vtFrames.push_back(hBitmap);
		}

		return (m_vtFrames[m_nCurrentFrame] == NULL) ? FALSE : TRUE;
	}

	UI_HANDLE Detach()
	{
		DeleteObject();
		Init();
		return NULL;
	}

	UINT GetFrameCount() const
	{
		return m_vtFrames.size();
	}

	UINT GetFrameElapse() const
	{
		return m_nFrameElapse;
	}

	const SIZE& GetSize() const
	{
		return m_sizeImage;
	}

	UI_HANDLE GetUIHandle() const
	{
		return (m_nCurrentFrame < m_vtFrames.size()) ? m_vtFrames[m_nCurrentFrame] : NULL;
	}

	bool IsNull() const
	{
		return m_vtFrames.empty();
	}

	std::wstring EnsureFullpath(LPCTSTR lpszFile)
	{
		std::wstring strPath;
		if( lpszFile && lpszFile[0] )
		{

			if(lpszFile[1] == ':')
				strPath = lpszFile;
			else
			{
				static std::wstring strAppPath;
				if (strAppPath.empty())
				{
					TCHAR tmp[1024] = {0};
					GetModuleFileName(NULL, tmp, sizeof(tmp));
					TCHAR* lpPtr = (TCHAR*)_tcsrchr(tmp,'\\');
					lpPtr[1] = 0;
					strAppPath = tmp;
				}
				strPath = strAppPath;				
				strPath.append(lpszFile);
			}
		}
		return strPath;
	}

	BOOL SaveFile(LPCTSTR pFileName)
	{
		BITMAP bm;
		if(::GetObject(m_vtFrames[m_nCurrentFrame], sizeof(BITMAP), &bm) != sizeof(BITMAP))
			return FALSE;

		if( bm.bmBitsPixel != 32 && bm.bmBitsPixel != 24 )
			return FALSE;


		int nBits = bm.bmBitsPixel;

		PixelFormat pf = PixelFormatUndefined;
		if( nBits == 32 )
			pf = PixelFormat32bppARGB;
		else if( nBits == 24 )
			pf = PixelFormat24bppRGB;
		else 
			return FALSE;


		BitmapData data;
		Bitmap img(m_sizeImage.cx,m_sizeImage.cy,pf);		
		Rect r(0, 0, m_sizeImage.cx, m_sizeImage.cy);

		if( 0 != img.LockBits(&r, ImageLockModeWrite, pf, &data) )
			return FALSE;

		int nLine = bm.bmWidth * bm.bmBitsPixel / 8;

		char* pSrc = (char*)bm.bmBits;
		char* pDes = (char*)data.Scan0 + (bm.bmHeight - 1) * nLine;

		for (int i = 0; i < bm.bmHeight; i++)
		{
			memcpy(pDes,pSrc,nLine);
			pDes -= nLine;
			pSrc += nLine;
		}

		img.UnlockBits(&data);

		CLSID encoderClsid;
		std::wstring strFile = pFileName;
		if ( strFile.find(TEXT(".bmp")) != std::wstring::npos)
			GetEncoderClsid(L"image/bmp", &encoderClsid);
		else if (strFile.find(TEXT(".jpg")) != std::wstring::npos 
			|| strFile.find(TEXT(".jpeg")) != std::wstring::npos)
			GetEncoderClsid(L"image/jpeg", &encoderClsid);
		else
			GetEncoderClsid(L"image/png", &encoderClsid);


		ULONG quality = 50;
		EncoderParameters encoderParameters;				
		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = EncoderQuality;
		encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;		
		encoderParameters.Parameter[0].Value = &quality;
		Status status = img.Save(pFileName,&encoderClsid,&encoderParameters);
		return (status == Ok);
	}

	IImage* LoadImage(LPCTSTR lpszFile, const SIZE& size = CSize(0,0),BOOL bHighQulity = FALSE)
	{		
		DeleteObject();

		std::wstring strPath = EnsureFullpath(lpszFile);

		if (!::PathFileExists(strPath.c_str()))
		{
			//assert(FALSE);
			return NULL;
		}
		BOOL bSuc = FALSE;
		if( bHighQulity )
		{
			bSuc = LoadAsGdiPlusPicture(strPath.c_str(),size);			
		}else
		{
			bSuc = LoadAsGdiPicture(strPath.c_str(),size);		
		}
		return (bSuc ? this : NULL);
	}

	IImage* LoadImage(const UI_HANDLE& hBitmap, const SIZE& size)
	{
		if(Attach(hBitmap)) m_sizeImage = size;        

		return (m_vtFrames.empty() ? NULL : this);
	}
	LPVOID GetGdiPlusHandle()
	{
		return static_cast<Gdiplus::Image*>(m_QualityBitmap);
	}

	UINT CurFrame(){
		return m_nCurrentFrame;
	}

	int NextFrame(UINT nFrame = -1)
	{
		if(nFrame != -1)
		{
			m_nCurrentFrame = nFrame;
		}
		else
		{
			m_nCurrentFrame ++;            
		}
		m_nCurrentFrame %= m_vtFrames.size();
		return m_nCurrentFrame;
	}

	void PrevFrame(UINT nFrame = -1)
	{
		if(nFrame != -1)
		{
			m_nCurrentFrame = nFrame;
		}
		else
		{
			m_nCurrentFrame --;            			
		}
		m_nCurrentFrame %= m_vtFrames.size();
	}


	virtual void Release()
	{
		delete this;
	}

	virtual void InitColorize(__in BOOL bColorize,__in float factory, __in COLORREF crColorize, __in COLORREF crClip)
	{
		if (bColorize)
		{
			m_fImagefac = factory;
			m_crColorize = crColorize;
			m_crClip = crClip;
		}
		m_bColorize = bColorize;
	}

	virtual void		InitImageHsl(__in BOOL bWidthHSL, __in HSL hsl)
	{
		if(bWidthHSL)
			m_hsl = hsl;
		m_bChangeHsl = bWidthHSL;
	}

	static int GetEncoderClsid(LPCTSTR format, CLSID* pClsid)
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		ImageCodecInfo* pImageCodecInfo = NULL;

		GetImageEncodersSize(&num, &size);
		if(size == 0)
			return -1;  // Failure

		pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
		if(pImageCodecInfo == NULL)
			return -1;  // Failure

		GetImageEncoders(num, size, pImageCodecInfo);

		for(UINT j = 0; j < num; ++j)
		{
			if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}    
		}

		free(pImageCodecInfo);
		return -1;  // Failure
	}
	static BOOL BitmapToFile(HBITMAP hBitmap,LPCTSTR path)
	{
		Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);
		if(pBitmap)
		{
			CLSID Clsid; 
			GetEncoderClsid(L"image/png", &Clsid);				
			Status  st = pBitmap->Save(path, &Clsid, NULL); 
			delete pBitmap;
			return (st == Ok);
		}
		return FALSE;
	}

private:
	void CreateBitmapFrame(Bitmap* pBitmap,const SIZE& size)
	{
		if (m_bColorize)
			ProcessImage(pBitmap, TRUE);
		else if (m_bChangeHsl)
			ProcessImage(pBitmap, FALSE);
		Color color;
		HBITMAP hBitmap;
		pBitmap->GetHBITMAP(color, &hBitmap);
		if(SplitHBitmap(hBitmap, size, m_sizeImage, &m_vtFrames))
		{
			if(size.cx) m_sizeImage.cx = size.cx;
			if(size.cy) m_sizeImage.cy = size.cy;
			::DeleteObject(hBitmap);
		}
		else
		{
			m_vtFrames.push_back(hBitmap);
		}
	}
	void ProcessImage(Bitmap* pBitMap, BOOL bColorize)
	{
		BitmapData data;
		Rect r(0, 0, pBitMap->GetWidth(), pBitMap->GetHeight());
		PixelFormat pf = pBitMap->GetPixelFormat();
		if ( pf != PixelFormat24bppRGB && pf != PixelFormat32bppARGB)
			return;
		
		if( 0 != pBitMap->LockBits(&r, ImageLockModeRead | ImageLockModeWrite, pf, &data) )
			return;

		int nBits = ( pf == PixelFormat24bppRGB ? 3 : 4);
		try
		{
			int offset = data.Stride - data.Width * nBits;
			unsigned char *p = NULL;
			p = (unsigned char*)data.Scan0;
			for (int y = 0; y < data.Height; y ++, p += offset)
				for (int x = 0; x < data.Width && p; x ++, p += nBits)// p += 3
				{
					if (bColorize)
						SetRgbHSL(p[2], p[1], p[0], m_fImagefac, m_crColorize, m_crClip);
					else
						AdjustHSL(p[2], p[1], p[0], m_hsl.H, m_hsl.S, m_hsl.L, m_crClip);
				}
		}
		catch(...)
		{

		}
		pBitMap->UnlockBits(&data);
	}

	IStream* IStreamFromImage(Image* pImage, LPCTSTR lpFormatType)
	{
		if (!lpFormatType)
			return NULL;
		CLSID bmpClsid;
		GetEncoderClsid(lpFormatType, &bmpClsid);

		if(HGLOBAL     hMem = ::GlobalAlloc(GMEM_MOVEABLE, 0))
		{
			IStream*    pStream = NULL;
			::CreateStreamOnHGlobal(hMem, TRUE, &pStream);

			pImage->Save(pStream, &bmpClsid, NULL);
			return pStream;
		}
		return NULL;
		
	}

	BOOL SplitHBitmap(HBITMAP hBmp, const SIZE& sizeDes, const SIZE& sizeSrc, std::vector<HBITMAP>* pvtBitmaps)
	{
		if (hBmp == NULL || sizeDes.cx == 0 || sizeDes.cy == 0)
			return FALSE;

		if(sizeDes.cx == sizeSrc.cx || sizeDes.cy == sizeSrc.cy) // needn't do anything
			return FALSE;

		Bitmap ImageSrc(hBmp, NULL);
		
		HDC hDC = GetDC(NULL);
		HDC hMemDC = CreateCompatibleDC(hDC);		
		HBITMAP hBitmap = CreateCompatibleBitmap(hDC,sizeDes.cx, sizeDes.cy);
		HGDIOBJ old = SelectObject(hMemDC,hBitmap);
		Graphics grap(hMemDC);
		
		grap.SetSmoothingMode(SmoothingModeHighQuality);
		grap.SetCompositingQuality(CompositingQualityHighQuality);
		grap.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		grap.DrawImage(&ImageSrc, 
			Rect(0, 0, sizeDes.cx, sizeDes.cy), 
			0, 
			0,
			sizeSrc.cx,
			sizeSrc.cy,
			UnitPixel);
		
		pvtBitmaps->push_back(hBitmap);

		SelectObject(hMemDC,old );
		DeleteDC(hMemDC);
		ReleaseDC(NULL,hDC);

		return TRUE;
	}

	BOOL LoadAsGdiPlusPicture(LPCTSTR lpPath,SIZE size)
	{
		ATLASSERT(m_QualityBitmap == NULL);
		m_QualityBitmap		= LoadFromFile(lpPath);
		m_sizeImage.cx      = m_QualityBitmap->GetWidth();
		m_sizeImage.cy      = m_QualityBitmap->GetHeight();
		return (m_QualityBitmap != NULL);
	}

	Bitmap *LoadFromFile(LPCTSTR lpPath)
	{
		IStream* pStream = NULL;
		CreateStreamOnHGlobal(NULL, TRUE, &pStream);//在堆中创建流对象     
		std::string data;
		helper::ufile::ReadFileData(lpPath, data);
		pStream->Write(data.c_str(), data.length() + 1, NULL);
		Bitmap *pImage = new Bitmap(pStream);
		pStream->Release();
		return pImage;
	}
	
	BOOL LoadAsGdiPicture(LPCTSTR lpPath,SIZE size)
	{
		BOOL bSingleFrame = TRUE;		
		Bitmap image(lpPath, FALSE);		
		m_sizeImage.cx      = image.GetWidth();
		m_sizeImage.cy      = image.GetHeight();
		if(m_sizeImage.cx > 0 && m_sizeImage.cy > 0)
		{
			BOOL bSingleFrame = TRUE;
			if(UINT nDimensionsCount = image.GetFrameDimensionsCount())
			{
				if(GUID *pDimensionIDs = (GUID *)new GUID[nDimensionsCount])
				{
					image.GetFrameDimensionsList(pDimensionIDs, nDimensionsCount);
					UINT nFrameCount = image.GetFrameCount(&pDimensionIDs[0]);
					if( nFrameCount > 1 )
					{
						bSingleFrame = FALSE;
						for(UINT nIndex = 0; nIndex < nFrameCount; nIndex++)
						{
							image.SelectActiveFrame(&pDimensionIDs[0], nIndex);

							//取出GIF的每一帧的图像数据
							//方法一（用此法会导致一些GIF动画会出现白底）
							// 							if(IStream* pStream = IStreamFromImage(&image, TEXT("image/png")))
							// 							{
							// 								if(Bitmap* pBitmap = Bitmap::FromStream(pStream))
							// 								{
							// 									CreateBitmapFrame(pBitmap,size);
							// 									delete pBitmap;
							// 								}
							// 								pStream->Release();
							// 							}

							//方法二
							BITMAPINFO bmi;        // bitmap header
							ZeroMemory(&bmi, sizeof(BITMAPINFO));
							bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
							bmi.bmiHeader.biWidth = image.GetWidth();
							bmi.bmiHeader.biHeight = image.GetHeight();
							bmi.bmiHeader.biPlanes = 1;
							bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
							bmi.bmiHeader.biCompression = BI_RGB;
							bmi.bmiHeader.biSizeImage = image.GetWidth() * image.GetHeight() * 4;
							BYTE *pvBits;          // pointer to DIB section
							HBITMAP hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

							HDC hMemDC = CreateCompatibleDC(NULL);
							HBITMAP hOriBmp = (HBITMAP)SelectObject(hMemDC, hbitmap);

							Gdiplus::Graphics gh(hMemDC);
							gh.DrawImage(&image,0,0);

							SelectObject(hMemDC, hOriBmp);
							DeleteDC(hMemDC);

							m_vtFrames.push_back(hbitmap);
						}
						if(UINT nLength = image.GetPropertyItemSize(PropertyTagFrameDelay))
						{
							PropertyItem* ppi   = (PropertyItem*)new BYTE[nLength];
							image.GetPropertyItem(PropertyTagFrameDelay, nLength, ppi);
							if(!(m_nFrameElapse = *(UINT*)ppi->value * 10))
							{
								m_nFrameElapse  = 100;
							}
							delete []ppi;
						}
					}
					delete []pDimensionIDs;
				}		
			}
			if(bSingleFrame)
			{
				CreateBitmapFrame(&image,size);
			}
			return (m_vtFrames.empty() ? FALSE : TRUE);
		}
		return FALSE;
	}
private:
	std::vector<HBITMAP>    m_vtFrames;

	UINT                    m_nFrameElapse;
	UINT                    m_nCurrentFrame;
	SIZE					m_sizeImage;
	Bitmap*					m_QualityBitmap;
	float					m_fImagefac;
	COLORREF				m_crColorize;
	COLORREF				m_crClip;
	BOOL					m_bColorize;
	BOOL					m_bChangeHsl;
	HSL						m_hsl;
};

typedef CImageT<true>	CImage;
typedef CImageT<false>	CImageHandle;

NAMESPACE_END(xgdi)