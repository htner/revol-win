#pragma once

#include "xgdi.h"
#include "xgdidef.h"
#include "xstretch.h"
#include <common/xstring.h>
#ifndef _USRDLL
#include <nsox/nulog.h>
#include <helper/utf8to.h>
#endif

NAMESPACE_BEGIN(xgdi)

struct CImgStreRect : public RECT
{
    CImgStreRect(xgdi::IImage* pImage, LPRECT srcRect)
    {
        SIZE sizeImage = pImage->GetSize();
        if(srcRect == NULL)
        {
            left = top = bottom = right = 0;
        }else{
            *(LPRECT)this = *srcRect;
        }

        if(right <= 0)   right	 = sizeImage.cx + right;
        if(bottom <= 0)  bottom  = sizeImage.cy + bottom;		
    }
};
inline BOOL DrawImage(xgdi::ICanvas* pCanvas, int dLeft, int dTop, int dw, int dh, 
                      xgdi::IImage* pImage, int sLeft, int sTop, int sw, int sh, int nFlag, COLORREF clrKey)
{
    CRect destRect(dLeft, dTop, dLeft+dw, dTop+dh);
    CRect srcRect(sLeft, sTop, sLeft+sw, sTop+sh);

    if(destRect.IsRectEmpty() || srcRect.IsRectEmpty()) return FALSE;

    return pCanvas->DrawImage(pImage, destRect, srcRect,nFlag,clrKey);		
}

inline BOOL DrawStretchImage(xgdi::ICanvas* pCanvas, xgdi::IImage* pImage,
                             RECT _rcDest, RECT outSrc, RECT inSrc, int nFlag,  COLORREF clrKey)
{
	BOOL bRet  = TRUE;
	if(nFlag & DIF_PROPORTSTRETCH)//�ȱ�������
	{
		CRect rcDest = _rcDest;
		CRect rcImg = outSrc;
		CRect rcDraw = rcDest;
		if (rcImg.Width() < rcDest.Width() && rcImg.Height() < rcDest.Height())
		{
			rcDraw.right = rcDraw.left + rcImg.Width();
			rcDraw.bottom = rcDraw.top + rcImg.Height();
		}
		else
		{
			if ( ((float)rcImg.Width()/rcDraw.Width()) > ((float)rcImg.Height()/rcDraw.Height()) )
			{
				int y2 = rcImg.Height() * rcDest.Width() / rcImg.Width();
				rcDraw.right = rcDraw.left + rcDest.Width();
				rcDraw.bottom = rcDraw.top + y2;
			} 
			else
			{
				int x2 = rcImg.Width() * rcDest.Height() / rcImg.Height();
				rcDraw.right = rcDraw.left + x2;
				rcDraw.bottom = rcDraw.top + rcDest.Height();
			}
		}
		//λ��ƫ��
		if(nFlag & DIF_ALIGNLEFT)
			rcDraw.MoveToX(rcDest.left);
		else if(nFlag & DIF_ALIGNRIGHT)
			rcDraw.MoveToX(rcDest.right - rcDraw.Width());
		else //center
			rcDraw.MoveToX(rcDest.left + (rcDest.Width()-rcDraw.Width())/2);
		
		if(nFlag & DIF_ALIGNTOP)
			rcDraw.MoveToY(rcDest.top);
		else if(nFlag & DIF_ALIGNBOTTOM)
			rcDraw.MoveToY(rcDest.bottom - rcDraw.Height());
		else //vcenter
			rcDraw.MoveToY(rcDest.top + (rcDest.Height()-rcDraw.Height())/2);

		bRet &= DrawImage(pCanvas, rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(),
			pImage, rcImg.left, rcImg.top, rcImg.Width(), rcImg.Height(), nFlag|DIF_STRETCH,clrKey);
	}
	else
	{
		CRect rectDest		= _rcDest;
		CRect rcOutsrc		= outSrc;

		int leftBorder      = inSrc.left	-	rcOutsrc.left;
		int topBorder       = inSrc.top	-	rcOutsrc.top;
		int rightBorder     = rcOutsrc.right	-	inSrc.right;
		int bottomBorder    = rcOutsrc.bottom	-	inSrc.bottom;

		// left-top right-top left-bottom right-bottom
		bRet &= DrawImage(pCanvas, rectDest.left, rectDest.top, leftBorder, topBorder,
			pImage, rcOutsrc.left, rcOutsrc.top, leftBorder, topBorder, nFlag,clrKey);
		bRet &= DrawImage(pCanvas, rectDest.right - rightBorder, rectDest.top, rightBorder, topBorder,
			pImage, rcOutsrc.right - rightBorder, rcOutsrc.top, rightBorder, topBorder, nFlag,clrKey);
		bRet &= DrawImage(pCanvas, rectDest.left, rectDest.bottom - bottomBorder, leftBorder, bottomBorder,
			pImage, rcOutsrc.left, rcOutsrc.bottom - bottomBorder, leftBorder, bottomBorder, nFlag,clrKey);
		bRet &= DrawImage(pCanvas, rectDest.right - rightBorder, rectDest.bottom - bottomBorder, rightBorder, bottomBorder,
			pImage, rcOutsrc.right - rightBorder, rcOutsrc.bottom - bottomBorder, rightBorder, bottomBorder, nFlag,clrKey);

		// left right top bottom stretch
		bRet &= DrawImage(pCanvas, rectDest.left, rectDest.top + topBorder, leftBorder, rectDest.Height() - topBorder - bottomBorder,
			pImage, rcOutsrc.left, rcOutsrc.top + topBorder, leftBorder, rcOutsrc.Height() - topBorder - bottomBorder, nFlag,clrKey);
		bRet &= DrawImage(pCanvas, rectDest.right - rightBorder, rectDest.top + topBorder, rightBorder, rectDest.Height() - topBorder - bottomBorder,
			pImage, rcOutsrc.right -rightBorder, rcOutsrc.top + topBorder, rightBorder, rcOutsrc.Height() - topBorder - bottomBorder, nFlag,clrKey);
		bRet &= DrawImage(pCanvas, rectDest.left + leftBorder, rectDest.top, rectDest.Width() - leftBorder - rightBorder, topBorder,
			pImage, rcOutsrc.left + leftBorder, rcOutsrc.top, rcOutsrc.Width() - leftBorder - rightBorder, topBorder, nFlag,clrKey);
		bRet &= DrawImage(pCanvas, rectDest.left + leftBorder, rectDest.bottom - bottomBorder, rectDest.Width() - leftBorder - rightBorder, bottomBorder,
			pImage, rcOutsrc.left + leftBorder, rcOutsrc.bottom - bottomBorder, rcOutsrc.Width() - leftBorder - rightBorder, bottomBorder, nFlag,clrKey);

		if(nFlag & DIF_STRE_NO_CENTER){return bRet;}

		rectDest.DeflateRect(leftBorder, topBorder, rightBorder, bottomBorder);
		rcOutsrc.DeflateRect(leftBorder, topBorder, rightBorder, bottomBorder);
		bRet &= DrawImage(pCanvas, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
			pImage, rcOutsrc.left, rcOutsrc.top, rcOutsrc.Width(), rcOutsrc.Height(), nFlag,clrKey);
	}
    return bRet;
}


inline int MeasureText(ICanvas* pCanvas, const xstring& strText, xgdi::IRCFont* pFont, RECT& rcText) //����str�ĸ߿�
{
    CSize size;
    if(pCanvas)
    {
        ::GetTextExtentPoint32(pCanvas->GetUIHandle(), strText.c_str(), strText.length(), &size);
        rcText.right = rcText.left + size.cx;
        rcText.bottom = rcText.top + size.cy;
    }
    
    return size.cx;
}

inline xgdi::IRCFont* GetDefaultFont()
{
    static XGDIObject<xgdi::IRCFont> font = CreateGDI<xgdi::IRCFont, IID_UIFont>();
    font->Attach(AtlGetDefaultGuiFont());
    return font;
}
inline IBrush* CreateSolidBrush(COLORREF clr)
{
    IBrush* brush = CreateGDI<IBrush, IID_UIBrush>();
    if(brush){
        brush->CreateSolidBrush(clr);
    }
    return brush;
}

inline HFONT GetDefaultGuiFont()
{
		return AtlGetDefaultGuiFont();
}
//static BOOL s_isNonclientmetrics = GetRcEnvConfigInt(_T("Rex"), _T("Nonclientmetrics"), 0);
/** You Need Delete Font Object OutSide*/
inline HFONT GetSystemFont()
{
	/*if (s_isNonclientmetrics)
	{
		NONCLIENTMETRICS nm;
		nm.cbSize = sizeof (NONCLIENTMETRICS);
		SystemParametersInfo (SPI_GETNONCLIENTMETRICS,0,&nm,0);
		LOGFONT fl = nm.lfMenuFont;
		WTL::CFont font; 
		font.CreateFontIndirect(&fl);
		return font.Detach();
	}
	else*/
		return GetDefaultGuiFont();

}

inline xstring GetSystemFontName()
{
	WTL::CFont font =  GetSystemFont();
	LOGFONT lf; font.GetLogFont(lf);
#ifndef _USRDLL
	//nsox::xlog(NsoxInfo, "SystemFontName--------: %s", helper::app2net(lf.lfFaceName).c_str());
#endif
	return lf.lfFaceName;
}

NAMESPACE_END(xgdi)


class XGDIUtil
{
public:
	static BITMAPINFO* PrepareRGBBitmapInfo(int nWidth, int nHeight, int	nColorDep)
	{
		BITMAPINFO *pRes = new BITMAPINFO;
		::ZeroMemory(pRes, sizeof(BITMAPINFO));
		pRes->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pRes->bmiHeader.biWidth = nWidth;
		pRes->bmiHeader.biHeight = nHeight;
		pRes->bmiHeader.biPlanes = 1;
		pRes->bmiHeader.biBitCount = nColorDep;

		DWORD	dwRowSize			= ((nColorDep / 8 )* abs(nWidth) + 3 ) & ~3;

		pRes->bmiHeader.biSizeImage= dwRowSize * abs(nHeight);
		return pRes;
	}
	static HBITMAP GetPartBmp(HBITMAP hSrcBmp, int cx, int cy, int x, int y)
	{
		assert(hSrcBmp); 
		if(NULL == hSrcBmp)
			return NULL;

		BITMAP bmp;
		if(::GetObject(hSrcBmp, sizeof(BITMAP), &bmp) == 0 )
			return NULL;

		if(cx + x > bmp.bmWidth ||
			cy	 + y > bmp.bmHeight)
			return NULL;

		// check for valid size

		HDC hDC = ::GetDC(NULL);

		int bitCount =bmp.bmBitsPixel;
		int colorBits = bitCount / 8;
		BITMAPINFO* pbi = PrepareRGBBitmapInfo(bmp.bmWidth, 
			bmp.bmHeight, bitCount);
		BYTE *pData = new BYTE[pbi->bmiHeader.biSizeImage];
		ZeroMemory(pData, pbi->bmiHeader.biSizeImage);
		::GetDIBits(hDC, hSrcBmp, 0, bmp.bmHeight, pData, pbi, DIB_RGB_COLORS);

		BITMAPINFO* pnewbi = PrepareRGBBitmapInfo(cx, cy, bitCount);
		BYTE*		pData3		=NULL;
		HBITMAP	hDestBmp	=CreateDIBSection(
			hDC,                // handle to DC
			pnewbi,				// bitmap data
			DIB_RGB_COLORS,		// data type indicator
			(void**)&pData3,    // bit values
			NULL,				// handle to file mapping object
			0					// offset to bitmap bit values
			);

		int	Step		=(colorBits * bmp.bmWidth + 3) & ~3;
		BYTE*  pDestOper	=pData + Step * (bmp.bmHeight - cy - y);

		int	nRowOffset	=x * colorBits;
		int	nnewRowSize	=(colorBits * cx + 3) & ~3;

		for(int i=0 ; i<cy; i++)
		{
			memcpy(pData3 + i * nnewRowSize, pDestOper + Step * i + nRowOffset, nnewRowSize);
		}

		delete[]	pData;
		pData	=NULL;

		delete	pbi;
		pbi		=NULL;

		delete	pnewbi;
		pnewbi	=NULL;

		::ReleaseDC(NULL, hDC);

		return  hDestBmp;
	}

	static HRGN BitmapToRegion (HBITMAP hBmp, POINT ptStart,
		COLORREF cTransparentColor, COLORREF cTolerance,
		const RECT& rcMaxBounds,DWORD rgnMode = RGN_XOR)
	{
		assert(hBmp);
		if(NULL == hBmp)
			return NULL;
		int	nHitCount				=0;
		int				x=0 , y=0;
		HRGN			wndRgn;		//��ť��RGN


		BITMAP BitmapSize;
		::GetObject(hBmp, sizeof(BITMAP), &BitmapSize);

		const int COLORDEPTH	=32;
		int		  nSQUADSize    =COLORDEPTH / 8;

		BITMAPINFO *pbi = new BITMAPINFO;
		::ZeroMemory(pbi, sizeof(BITMAPINFO));
		pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbi->bmiHeader.biWidth = BitmapSize.bmWidth;
		pbi->bmiHeader.biHeight =  BitmapSize.bmHeight;
		pbi->bmiHeader.biPlanes = 1;
		pbi->bmiHeader.biBitCount = COLORDEPTH;

		WORD	nBytes				=COLORDEPTH / 8;
		pbi->bmiHeader.biSizeImage= 
			((nBytes * abs( BitmapSize.bmWidth) + 3) & ~3) * abs( BitmapSize.bmHeight);

		if(NULL == pbi)
			return NULL;

		BYTE *pData = new BYTE[pbi->bmiHeader.biSizeImage];
		if(NULL == pData)
		{
			SAFE_DELETE(pbi);
			return NULL;
		}

		HDC	hDC	=GetDC(NULL);

		::GetDIBits(hDC, hBmp, 
			0, BitmapSize.bmHeight, pData, pbi, DIB_RGB_COLORS);

		ReleaseDC(NULL, hDC);

		RECT	rcSrcRgn = {ptStart.x, ptStart.y, ptStart.x + BitmapSize.bmWidth, ptStart.y + BitmapSize.bmHeight};
		wndRgn	=::CreateRectRgnIndirect(&rcSrcRgn);

		BYTE	rTrans	=GetRValue(cTransparentColor);
		BYTE	gTrans	=GetGValue(cTransparentColor);
		BYTE	bTrans	=GetBValue(cTransparentColor);

		RECT	rcTempRgn = {0, 0, 0, 0};
		bool bRectZeroSize = true;
		int rgncount = 0;
		for(y=0; y<BitmapSize.bmHeight; y++)
		{
			rcTempRgn = NULL_RECT;
			bRectZeroSize = true;		

			for(x=0; x<BitmapSize.bmWidth; x++)
			{
				POINT point = {ptStart.x + x, ptStart.y + y};

				if(!::IsRectEmpty(&rcMaxBounds) &&
					::PtInRect(&rcMaxBounds,point))
				{
					nHitCount ++;

					if(!::IsRectEmpty(&rcTempRgn))
					{
						HRGN	hTemp	=::CreateRectRgnIndirect(&rcTempRgn);
						::CombineRgn(wndRgn, wndRgn, hTemp, rgnMode);
						SAFE_DELETEGDIOBJECT(hTemp);

						rcTempRgn = NULL_RECT;
					}
					continue;
				}

				BYTE*	pRgbData=x * nSQUADSize + pData + pbi->bmiHeader.biWidth * nSQUADSize * sizeof(BYTE) * (BitmapSize.bmHeight-y-1);
				BYTE	b		=*(pRgbData + 0);
				BYTE	g		=*(pRgbData + 1);
				BYTE	r		=*(pRgbData + 2);

				if(r == rTrans &&
					g == gTrans &&
					b == bTrans)
				{
					if(bRectZeroSize)
					{	//��ʼ��
						rcTempRgn.left	=ptStart.x + x;
						rcTempRgn.right	=rcTempRgn.left + 1;
						rcTempRgn.top	=ptStart.y + y;
						rcTempRgn.bottom=rcTempRgn.top + 1;
						bRectZeroSize = false;
					}
					else
					{
						rcTempRgn.right	++;
					}

				}
				else
				{
					if(!bRectZeroSize)
					{
						HRGN	hTemp	=::CreateRectRgnIndirect(&rcTempRgn);
						::CombineRgn(wndRgn, wndRgn, hTemp, rgnMode);

						SAFE_DELETEGDIOBJECT(hTemp);

						rcTempRgn = NULL_RECT;

						bRectZeroSize = true;
					}				
				}
			}

			//����rgn������rcTempRgn��Ϊ0
			if(!bRectZeroSize)
			{
				HRGN	hTemp	=::CreateRectRgnIndirect(&rcTempRgn);

				::CombineRgn(wndRgn, wndRgn, hTemp, rgnMode);

				SAFE_DELETEGDIOBJECT(hTemp);
			}
		}

		SAFE_DELETE_ARRAY(pData);
		SAFE_DELETE(pbi);
		return wndRgn;
	}



	/******************************************************
	******************************************************/

	static HRGN GetImageRgn(xgdi::IImage* pImage,const geometry::Rect& rect,COLORREF clrKey)
	{
		using namespace geometry;
		if(pImage == NULL) return NULL;
		HBITMAP hBmp = XGDIUtil::GetPartBmp(pImage->GetUIHandle(), rect.w, rect.h, rect.left, rect.top);
		if(hBmp){
			HRGN hTempRgn = ::CreateRectRgn(0,0,rect.w,rect.h) ;

			HRGN hRgn = XGDIUtil::BitmapToRegion(hBmp, NULL_POINT, clrKey, 0, NULL_RECT);

			::CombineRgn(hRgn,hTempRgn,hRgn,RGN_DIFF);

			SAFE_DELETEGDIOBJECT(hBmp);
			SAFE_DELETEGDIOBJECT(hTempRgn);
			return hRgn;
		}
		return NULL;
	}
	static xgdi::StretchRgn CreateStretchRgn(xgdi::StretchImg& stretch, xgdi::IImage* pImage, COLORREF clrKey)
	{
		using namespace geometry;
		xgdi::StretchRgn results;

		results.rgnLT	= GetImageRgn(pImage, stretch.GetPart(xgdi::StretchPart_LT),clrKey);
		results.rgnRT	= GetImageRgn(pImage, stretch.GetPart(xgdi::StretchPart_RT),clrKey);
		results.rgnLB	= GetImageRgn(pImage, stretch.GetPart(xgdi::StretchPart_LB),clrKey);
		results.rgnRB	= GetImageRgn(pImage, stretch.GetPart(xgdi::StretchPart_RB),clrKey);
		return results;
	}
	static HRGN CombineRgn(xgdi::StretchImg& stretch,const xgdi::StretchRgn& corners,const geometry::Rect& rcBounds)
	{
		HRGN hRgn = ::CreateRectRgnIndirect(&(RECT)rcBounds);

		if(corners.rgnLT != NULL)
		{
			::CombineRgn(hRgn,hRgn,corners.rgnLT,RGN_DIFF);
		}
		if(corners.rgnRT != NULL)
		{
			geometry::Rect rect= stretch.GetPart(xgdi::StretchPart_RT);
			::OffsetRgn(corners.rgnRT,rcBounds.w - rect.w,0);
			::CombineRgn(hRgn,hRgn,corners.rgnRT,RGN_DIFF);
			::OffsetRgn(corners.rgnRT,rect.w - rcBounds.w,0);
		}
		if(corners.rgnLB != NULL)
		{
			geometry::Rect rect= stretch.GetPart(xgdi::StretchPart_LB);
			::OffsetRgn(corners.rgnLB,0,rcBounds.h - rect.h);
			::CombineRgn(hRgn,hRgn,corners.rgnLB,RGN_DIFF);
			::OffsetRgn(corners.rgnLB,0,rect.h - rcBounds.h);
		}
		if(corners.rgnRB != NULL)
		{
			geometry::Rect rect= stretch.GetPart(xgdi::StretchPart_RB);
			::OffsetRgn(corners.rgnRB,rcBounds.w - rect.w,rcBounds.h - rect.h);
			::CombineRgn(hRgn,hRgn,corners.rgnRB,RGN_DIFF);
			::OffsetRgn(corners.rgnRB,rect.w - rcBounds.w,rect.h - rcBounds.h);
		}
		return hRgn;
	}
};


