#pragma once
#include "xgdi.h"

NAMESPACE_BEGIN(xgdi)

template<bool t_bManaged>
class CCanvasT :
    implement ICanvas
{
public:
    CCanvasT(const CSize& sizeCanvas = 0, HDC hDC = NULL)
    {
        SetSize(sizeCanvas);
        Attach(hDC);
        m_crTransparent = RGB(255,0,255);
        hRgn_			= NULL;
    }

    ~CCanvasT()
    {
        Detach();
    }

    operator HDC() const
    {
        return m_hDC;
    }

    BOOL AlphaBlend(ICanvas* pCanvas, const RECT& rcDest, const RECT& rcSrc, BLENDFUNCTION blender)
    {
        CRect rectDest(rcDest), rectSrc(rcSrc);
        return ::AlphaBlend(
            m_hDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
            *(CCanvas*)pCanvas, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), blender);
    }

    BOOL AlphaSolidBlend(int nColorRef, const RECT& rcDest, const SIZE& szSrc,BLENDFUNCTION blender)
    {
        CRect rectDest(rcDest);
        CCanvas canvasSolid(szSrc, m_hDC);
		CRect rectSrc = CRect(0,0,szSrc.cx,szSrc.cy);
        canvasSolid.FillSolidRect(rectSrc, nColorRef);
        return ::AlphaBlend(
            m_hDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
            canvasSolid, 0, 0, szSrc.cx, szSrc.cy, blender);
    }

    BOOL Attach(const UI_HANDLE& hDC)
    {
        if(t_bManaged)
        {
            if(hDC)
            {
                m_hDC			= ::CreateCompatibleDC(hDC);
                m_hMemBitmap	= ::CreateCompatibleBitmap(hDC, m_sizeCanvas.cx, m_sizeCanvas.cy);
                m_hOldBitmap	= (HBITMAP)::SelectObject(m_hDC, (HGDIOBJ)m_hMemBitmap);
            }
            else
            {
                m_hDC			= hDC;
            }
        }
        else
        {
            m_hDC			= hDC;
        }

        return (m_hDC == NULL) ? FALSE : TRUE;
    }

    UI_HANDLE Detach()
    {
        if(m_hDC == NULL) return NULL;

        HDC hDC = NULL;

        if(t_bManaged)
        {
            ::SelectObject(m_hDC, (HGDIOBJ)m_hOldBitmap);
            ::DeleteObject((HGDIOBJ)m_hMemBitmap);
            ::DeleteDC(m_hDC);
        }
        else
        {
            hDC = m_hDC;
        }

        return hDC;
    }

    BOOL Flush(__in UI_HANDLE hUI, __in RECT rcDest, RECT rcSrc, __in int nFlag, COLORREF clrKey)
    {
        BOOL bRet = FALSE; 
        if(hUI != NULL)
        {
            CRect rectDest	= rcDest;
            CRect rectSrc	= rcSrc;

            if(nFlag & DIF_TRANSPARENT)
            {
                bRet = ::TransparentBlt(
                    hUI, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
                    m_hDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
                    clrKey);
            }
            else
            {
                bRet = ::StretchBlt(
                    hUI, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
                    m_hDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
                    SRCCOPY);
            }
            return bRet;
        }
        return FALSE;
    }

    BOOL Flush(ICanvas* pCanvas, RECT rcDest, RECT rcSrc, int nFlag, COLORREF clrKey)
    {			
        return Flush(pCanvas->GetUIHandle(), rcDest, rcSrc, nFlag, clrKey);
    }

    BOOL DrawIcon(IIcon* pIcon, const RECT& rcDest, int nFlag)
    {
        BOOL bRet = FALSE;
        if(NULL == pIcon) return FALSE;

        if(nFlag & xgdi::DIF_TRANSPARENT)
        {
            bRet = DrawImage(pIcon->GetImage(), rcDest, pIcon->GetRect(), nFlag, pIcon->GetColorKey());
        }
        else
        {
            bRet = DrawImage(pIcon->GetImage(), rcDest, pIcon->GetRect(), nFlag);
        }
        return bRet;
    }

    BOOL DrawIcon(IIcon* pIcon, const RECT& rcDest, const RECT& rcBolder, int nFlag)
    {
        BOOL bRet = FALSE;
        if(NULL == pIcon) return FALSE;

        CRect rcInner = pIcon->GetRect();
        rcInner.DeflateRect(&rcBolder);

        bRet = xgdi::DrawStretchImage(this, pIcon->GetImage(), rcDest, pIcon->GetRect(), rcInner, nFlag, pIcon->GetColorKey());

        return bRet;
    }

    BOOL DrawImage(ICanvas* pCanvas, const RECT& rcDest, const RECT& rcSrc, int nFlag, COLORREF clrKey)
    {
        BOOL bRet = FALSE;

        CRect rectDest(rcDest), rectSrc(rcSrc);
        if(rectSrc.right == -1) rectSrc.right = pCanvas->GetSize().cx;
        if(rectSrc.bottom == -1) rectSrc.bottom = pCanvas->GetSize().cy;

        HDC hBitmapDC = pCanvas->GetUIHandle();

        // Stretch Mode
        if(nFlag & DIF_STRETCH)
		{				
			if(nFlag & DIF_ALPHATRANSPARENT)
			{
				BLENDFUNCTION blend;
				blend.BlendOp = AC_SRC_OVER;
				blend.BlendFlags = 0;
				blend.AlphaFormat = AC_SRC_ALPHA;
				blend.SourceConstantAlpha = 255;
				bRet =::AlphaBlend(m_hDC, rectDest.left, rectDest.top, rectDest.Width(), 
					rectDest.Height(), hBitmapDC, rectSrc.left, rectSrc.top, 
					rectSrc.Width(), rectSrc.Height(), blend);
			} else if ( DIF_TRANSPARENT)
			{
				bRet = ::TransparentBlt(
					m_hDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
					hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
					clrKey);
			}else
			{
				bRet = ::StretchBlt(
					m_hDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
					hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
					SRCCOPY);
			}
        }
        // Tile Mode
        else if(nFlag & DIF_TILE)
        {
            bRet = FALSE;
        }
        // Center Mode
        else
        {
            int xOffset =   (rectDest.Width() - rectSrc.Width()) / 2;
            int yOffset =   (rectDest.Height() - rectSrc.Height()) / 2;

            if(nFlag & DIF_ALIGNLEFT)		xOffset = 0;
            if(nFlag & DIF_ALIGNTOP)		yOffset = 0;
            if(nFlag & DIF_ALIGNRIGHT)		xOffset += xOffset;
            if(nFlag & DIF_ALIGNBOTTOM)     yOffset += yOffset;

            rectDest.OffsetRect(xOffset, yOffset);
            //yOffset>0? rectDest.top   += yOffset : rectSrc.top -= yOffset;
            //xOffset>0? rectDest.left   += xOffset : rectSrc.left -= xOffset;

			if (nFlag & DIF_ALPHATRANSPARENT)
			{
				BLENDFUNCTION blend;
				blend.BlendOp = AC_SRC_OVER;
				blend.BlendFlags = 0;
				blend.AlphaFormat = AC_SRC_ALPHA;
				blend.SourceConstantAlpha = 255;
				bRet = ::AlphaBlend(
					m_hDC, rectDest.left, rectDest.top, rectSrc.Width(), rectSrc.Height(),
					hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
					blend);
			} else if (nFlag & DIF_TRANSPARENT)
			{
				bRet = ::TransparentBlt(
					m_hDC, rectDest.left, rectDest.top, rectSrc.Width(), rectSrc.Height(),
					hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
					clrKey);
			}
			else
			{
				bRet = ::BitBlt(
					m_hDC, rectDest.left, rectDest.top, rectSrc.Width(), rectSrc.Height(),
					hBitmapDC, rectSrc.left, rectSrc.top, SRCCOPY);
			}
        }

        return bRet;
    }

    BOOL DrawImage(IImage* pImage, const RECT& rcDest, const RECT& rcSrc, int nFlag)
    {
        return DrawImage(pImage, rcDest, rcSrc, nFlag, m_crTransparent);
    }

    BOOL DrawImage(IImage* pImage, const RECT& rcDest, const RECT& rcSrc, int nFlag, COLORREF clrKey)
    {
        BOOL bRet = FALSE;
        if(pImage)
        {
			CRect rectDest(rcDest), rectSrc(rcSrc);
			if(rectSrc.right == -1) rectSrc.right = pImage->GetSize().cx;
			if(rectSrc.bottom == -1) rectSrc.bottom = pImage->GetSize().cy;

			if(Gdiplus::Image* p = (Gdiplus::Image*)(pImage->GetGdiPlusHandle()))
			{
				Gdiplus::Graphics g((HDC)GetUIHandle());	
	 			g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	 			g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
	 			g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

				if( nFlag & DIF_STRETCH )
				{
					RectF rcDf(rectDest.left,rectDest.top,rectDest.right - rectDest.left,rectDest.bottom - rectDest.top);
					g.DrawImage(p,rcDf,rectSrc.left,rectSrc.top,rectSrc.right - rectSrc.left,rectSrc.bottom - rectSrc.top,UnitPixel);
				}else
				{
					int dh = rectDest.bottom - rectDest.top;
					int dw = rectDest.right - rectDest.left;
					int sh = rectSrc.bottom - rectSrc.top;
					int sw = rectSrc.right - rectSrc.left;
					RECT rcD = rectDest;
					RECT rcS = rectSrc;
					if( sw < dw )
						rcD.right = rcD.left + sw;
					else
						rcS.right = rcS.left + dw;

					if( sh < dh )
						rcD.bottom = rcD.top + sh;
					else
						rcS.bottom = rcS.top+ dh;

					RectF rcDf(rcD.left,rcD.top,rcD.right - rcD.left,rcD.bottom - rcD.top);
					g.DrawImage(p,rcDf,rcS.left,rcS.top,rcS.right - rcS.left,rcS.bottom - rcS.top,UnitPixel);

				}
				return TRUE;
			}

            HDC hBitmapDC = ::CreateCompatibleDC(m_hDC);
            HBITMAP hOldBmp = (HBITMAP)::SelectObject(hBitmapDC, (HGDIOBJ)(HBITMAP)(pImage->GetUIHandle()));



            // Stretch Mode
            if(nFlag & DIF_STRETCH)
            {				
				if(nFlag & DIF_ALPHATRANSPARENT)
				{
					BLENDFUNCTION blend;
					blend.BlendOp = AC_SRC_OVER;
					blend.BlendFlags = 0;
					blend.AlphaFormat = AC_SRC_ALPHA;
					blend.SourceConstantAlpha = 255;
					bRet =::AlphaBlend(m_hDC, rectDest.left, rectDest.top, rectDest.Width(), 
						rectDest.Height(), hBitmapDC, rectSrc.left, rectSrc.top, 
						rectSrc.Width(), rectSrc.Height(), blend);
				} else if ( DIF_TRANSPARENT)
				{
					bRet = ::TransparentBlt(
									 m_hDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
									 hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
									 clrKey);
				}else
				{
					bRet = ::StretchBlt(
						m_hDC, rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height(),
						hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
						SRCCOPY);
				}

            }
            // Tile Mode
            else if(nFlag & DIF_TILE)
            {
                bRet = FALSE;
            }
            // Center Mode
            else
            {
                int xOffset =   (rectDest.Width() - rectSrc.Width()) / 2;
                int yOffset =   (rectDest.Height() - rectSrc.Height()) / 2;

                if(nFlag & DIF_ALIGNLEFT)		xOffset = 0;
                if(nFlag & DIF_ALIGNTOP)		yOffset = 0;
                if(nFlag & DIF_ALIGNRIGHT)		xOffset += xOffset;
                if(nFlag & DIF_ALIGNBOTTOM)     yOffset += yOffset;

                rectDest.OffsetRect(xOffset, yOffset);
                //yOffset>0? rectDest.top   += yOffset : rectSrc.top -= yOffset;
                //xOffset>0? rectDest.left   += xOffset : rectSrc.left -= xOffset;

				if (nFlag & DIF_ALPHATRANSPARENT)
				{
					BLENDFUNCTION blend;
					blend.BlendOp = AC_SRC_OVER;
					blend.BlendFlags = 0;
					blend.AlphaFormat = AC_SRC_ALPHA;
					blend.SourceConstantAlpha = 255;
					bRet = ::AlphaBlend(
						m_hDC, rectDest.left, rectDest.top, rectSrc.Width(), rectSrc.Height(),
						hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
						blend);
				} else if (nFlag & DIF_TRANSPARENT)
				{
					bRet = ::TransparentBlt(
						m_hDC, rectDest.left, rectDest.top, rectSrc.Width(), rectSrc.Height(),
						hBitmapDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
						clrKey);
				}
				else
				{
					bRet = ::BitBlt(
						m_hDC, rectDest.left, rectDest.top, rectSrc.Width(), rectSrc.Height(),
						hBitmapDC, rectSrc.left, rectSrc.top, SRCCOPY);
				}

            }

            ::SelectObject(hBitmapDC, (HGDIOBJ)hOldBmp);
            ::DeleteDC(hBitmapDC);
        }

        return bRet;
    }

	int DrawText(xgdi::IRCFont* pFont, LPCTSTR lpszText, LPRECT pRectItem, int nFlag)
    {
        int nRet = 0;
        if(pFont)
        {
            HFONT hOldFont = (HFONT)::SelectObject(m_hDC, (HGDIOBJ)(HFONT)(*(CFont*)pFont));
            nRet = ::DrawText(m_hDC, lpszText, -1, pRectItem, nFlag);
            ::SelectObject(m_hDC, (HGDIOBJ)hOldFont);
        }
        else
        {
            nRet = ::DrawText(m_hDC, lpszText, -1, pRectItem, nFlag);
        }

        return nRet;
    }
	void DrawGrowText(xgdi::IRCFont* pFont,LPCTSTR lpszText,COLORREF crlText,COLORREF clrGrow,const RECT& rect,DWORD flag,int radius = 4)
	{
		static int g_bound = 4;		

		int width = rect.right - rect.left + g_bound * 2;
		int height = rect.bottom - rect.top + g_bound * 2;

		BITMAPINFO bmih = {0};
		bmih.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
		bmih.bmiHeader.biWidth = width;  
		bmih.bmiHeader.biHeight = height;    
		bmih.bmiHeader.biPlanes = 1;  
		bmih.bmiHeader.biBitCount = 32;   
		bmih.bmiHeader.biCompression = BI_RGB;    
		BYTE *pBitsBkg = NULL;   
		BYTE *pMaskBits = NULL; 
		HBITMAP hBitmapBkg = CreateDIBSection(m_hDC,&bmih,DIB_RGB_COLORS,(void **)&pBitsBkg,NULL,0); 
		HBITMAP hBitmapMask = CreateDIBSection(m_hDC,&bmih,DIB_RGB_COLORS,(void **)&pMaskBits,NULL,0); 
		HDC hdcMem = CreateCompatibleDC(m_hDC);  


		HGDIOBJ oldBitmap = ::SelectObject(hdcMem,hBitmapBkg);
		BitBlt(hdcMem,0,0,width,height,m_hDC,rect.left - g_bound,rect.top-g_bound,SRCCOPY);

		::SelectObject(hdcMem,hBitmapMask);
		HFONT hOldFont = (HFONT)::SelectObject(hdcMem, (HGDIOBJ)(HFONT)(*(CFont*)pFont));
				
		RECT rcText = {g_bound,g_bound,width+g_bound,height+g_bound};
		int nTextLen = _tcslen(lpszText);
		::DrawText(hdcMem,lpszText,nTextLen,&rcText,flag | DT_CALCRECT);

		int oldTextMode = ::SetBkMode(hdcMem, TRANSPARENT);
		::SetTextColor(hdcMem,RGB(255,255,255));
		::DrawText(hdcMem,lpszText,nTextLen,&rcText,flag & (~DT_VCENTER | ~DT_CENTER | ~DT_CALCRECT));
		
		FastImageBlur((int*)pMaskBits,width,height,radius);

		if(pMaskBits && pBitsBkg)
		{
			int grow_r = GetRValue(clrGrow);
			int grow_g = GetGValue(clrGrow);
			int grow_b = GetBValue(clrGrow);
			DWORD* pmask = (DWORD*)pMaskBits;
			DWORD* pbkg = (DWORD*)pBitsBkg;
			static float g_factor = 0.6f;
			for (int i=0; i < width; i++)
			{
				for (int j=0; j < height; j++)
				{
					DWORD mask = *pmask;
					DWORD bkg = *pbkg;
					float f = g_factor * (float)(GetRValue(mask))/255;

					int r = f * grow_r + GetRValue(bkg) * (1-f);
					int g = f * grow_g + GetGValue(bkg) * (1-f);
					int b = f * grow_b + GetBValue(bkg) * (1-f);
					*pbkg = RGB(r,g,b);

					pmask++;
					pbkg++;
				}
			}
		}

		::SelectObject(hdcMem,hBitmapBkg);
		::SetTextColor(hdcMem,crlText);
		::DrawText(hdcMem,lpszText,nTextLen,&rcText,flag & (~DT_VCENTER | ~DT_CENTER | ~DT_CALCRECT));
		BitBlt(m_hDC,rect.left - g_bound ,rect.top - g_bound ,width,height,hdcMem,0,0,SRCCOPY);	

		::SelectObject(hdcMem,oldBitmap);
		::SelectObject(hdcMem, (HGDIOBJ)hOldFont);
		::DeleteObject(hBitmapBkg);
		::DeleteObject(hBitmapMask);
		::DeleteDC(hdcMem);
	}

	void BlurImage(int radius = 4)
	{
		BITMAP bitmap = {0};
		HBITMAP hBitmap = ::SelectObject(m_hDC,NULL);
		GetObject(hBitmap,sizeof(bitmap), &bitmap);
		int size = bitmap.bmHeight * bitmap.bmWidth * bitmap.bmBitsPixel / 8;

		int* buff = new int[size];
		GetBitmapBits(hBitmap,size,buff);
		FastImageBlur(buff,bitmap.bmWidth,bitmap.bmHeight,radius);
		SetBitmapBits(hBitmap,size,buff);
		delete[] buff;

		::SelectObject(m_hDC,hBitmap);
		
	}

    int FillRect(const RECT& rcRect, IBrush* pBrush)
    {
        return ::FillRect(m_hDC, &rcRect, (HBRUSH)(*(CBrush*)pBrush));
    }

    BOOL FillRgn(IRegion* pRegion, IBrush* pBrush)
    {
        if(NULL == pRegion) return -1;

        return ::FillRgn(m_hDC, (HRGN)(*(CRegion*)pRegion), (HBRUSH)(*(CBrush*)pBrush));
    }

    int FillSolidRect(const RECT& rcRect, int nColorRef)
    {
		COLORREF clrOld = ::SetBkColor(m_hDC, nColorRef);
		ATLASSERT(clrOld != CLR_INVALID);
		if(clrOld != CLR_INVALID)
		{
			::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &rcRect, NULL, 0, NULL);
			::SetBkColor(m_hDC, clrOld);
		}
        return 0;
    }
	int FillSolidRect(const RECT& rcRect, int color, int alpha)
	{
		if (alpha <= 0)//0表示全透明 无需再画了
			return 0;
		if(alpha >= 255)//255表示完全不透明 无需做透明处理
		{
			FillSolidRect(rcRect, color);
			return 0;
		}

		int nXDest = rcRect.left;   // 目标X偏移
		int nYDest = rcRect.top;   // 目标Y偏移
		int nWidthDest = rcRect.right - rcRect.left;      // 目标宽度
		int nHeightDest = rcRect.bottom - rcRect.top;    // 目标高度

		HBITMAP hBitmap = CreateCompatibleBitmap( m_hDC, nWidthDest, nHeightDest);
		HDC hdcMem = ::CreateCompatibleDC( m_hDC);
		::SelectObject(hdcMem, hBitmap);
		::BitBlt( hdcMem, 0, 0, nWidthDest, nHeightDest, m_hDC, nXDest, nYDest, SRCCOPY);

		if( color==RGB(0,0,0) || (color==RGB(255,255,255)) ) //白色或者黑色
		{ 
			RECT rc ={0, 0, nWidthDest, nHeightDest};
			HBRUSH hBrush = ::CreateSolidBrush(color);
			::FillRect(hdcMem, &rc, hBrush);

			BLENDFUNCTION bf;
			bf.AlphaFormat = 0;
			bf.BlendFlags = 0;
			bf.BlendOp = AC_SRC_OVER;
			bf.SourceConstantAlpha = alpha;
			::AlphaBlend(m_hDC,nXDest,nYDest,nWidthDest,nHeightDest,hdcMem,0,0,nWidthDest, nHeightDest, bf);

			::DeleteObject( hBrush);
			::DeleteDC(hdcMem);
			::DeleteObject(hBitmap);
			return 0;
		}

		BITMAPINFO bmi;
		memset(&bmi, 0, sizeof(bmi));
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biWidth = nWidthDest;
		bmi.bmiHeader.biHeight = nHeightDest;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biCompression = BI_RGB;

		if( !GetDIBits( hdcMem, hBitmap, 0, nHeightDest, NULL, &bmi, DIB_RGB_COLORS) )  //获取位图信息
			return 0;
		DWORD InfoSize = bmi.bmiHeader.biSize + bmi.bmiHeader.biClrUsed*sizeof(RGBQUAD);  //位图信息和调色板大小
		// 分配空间大小须包括位图信息、调色板、位图数据
		BYTE* lpBits = new BYTE[ bmi.bmiHeader.biSizeImage + InfoSize];
		ATLASSERT( lpBits);
		::GetDIBits( hdcMem, hBitmap, 0, nHeightDest, lpBits, &bmi, DIB_RGB_COLORS);

		int r = GetRValue( color);
		int g = GetGValue( color);
		int b = GetBValue( color);

		BYTE* p = lpBits;
		for(size_t i = InfoSize; i<bmi.bmiHeader.biSizeImage + InfoSize; ++i, ++p)
		{
			switch(i%4)
			{
			case 0:{ *p=(BYTE)((alpha*b + (255-alpha)*(*p))/255); }	break;
			case 1:{ *p=(BYTE)((alpha*g + (255-alpha)*(*p))/255); }	break;
			case 2:{ *p=(BYTE)((alpha*r + (255-alpha)*(*p))/255); }	break;
			}
		}

		::SetDIBits(hdcMem, hBitmap, 0, nHeightDest, lpBits, &bmi, DIB_RGB_COLORS);
		::BitBlt( m_hDC, nXDest, nYDest, nWidthDest, nHeightDest, hdcMem, 0, 0, SRCCOPY);

		::DeleteObject(hBitmap);
		::DeleteDC( hdcMem);
		delete[] lpBits;
		lpBits = NULL;
		return 0;
	}

    int FillSolidRect(int x1, int y1, int x2, int y2, int nColorRef)
    {
        RECT rcRect = {x1, y1, x2, y2};
		return FillSolidRect(rcRect,nColorRef);
    }

    BOOL FillSolidRgn(IRegion* pRegion, int nColorRef)
    {
        if(NULL == pRegion) return -1;

        HBRUSH hBrush = ::CreateSolidBrush(nColorRef);
        BOOL bRet = ::FillRgn(m_hDC, (HRGN)(*(CRegion*)pRegion), hBrush);
        ::DeleteObject(hBrush);

        return bRet;
    }

    int FrameRect(const RECT& rcRect, IBrush* pBrush)
    {
        return ::FrameRect(m_hDC, &rcRect, (HBRUSH)(*(CBrush*)pBrush));
    }

    int FrameSolidRect(const RECT& rcRect, int nColorRef)
    {
        HBRUSH hBrush = ::CreateSolidBrush(nColorRef);
        int nRet = ::FrameRect(m_hDC, &rcRect, hBrush);
        ::DeleteObject(hBrush);

        return nRet;
    }

    int FrameSolidRect(int x1, int y1, int x2, int y2, int nColorRef)
    {
        HBRUSH hBrush = ::CreateSolidBrush(nColorRef);
        RECT rcRect = {x1, y1, x2, y2};
        int nRet = ::FrameRect(m_hDC, &rcRect, hBrush);
        ::DeleteObject(hBrush);

        return nRet;
    }

    int FrameSolidRgn(IRegion* pRegion, int nColorRef, int nXWeight, int nYWeight)
    {
        if(NULL == pRegion) return -1;

        HBRUSH hBrush = ::CreateSolidBrush(nColorRef);
        int nRet = ::FrameRgn(m_hDC, (HRGN)(*(CRegion*)pRegion), hBrush, nXWeight, nYWeight);
        ::DeleteObject(hBrush);

        return nRet;
    }

    const SIZE& GetSize() const
    {
        return m_sizeCanvas;
    }

    UI_HANDLE GetUIHandle() const
    {
        return m_hDC;
    }

    BOOL Line(IPen* pPen, int x1, int y1, int x2, int y2)
    {
        BOOL bRet;

        if(pPen)
        {
            HPEN hOldPen = (HPEN)::SelectObject(m_hDC, (HGDIOBJ)(HPEN)(*(CPen*)pPen));
            ::MoveToEx(m_hDC, x1, y1, NULL);
            bRet = ::LineTo(m_hDC, x2, y2);
            ::SelectObject(m_hDC, (HGDIOBJ)hOldPen);
        }
        else
        {
            ::MoveToEx(m_hDC, x1, y1, NULL);
            bRet = ::LineTo(m_hDC, x2, y2);
        }

        return bRet;
    }

    BOOL Present(const UI_HANDLE& hDC, const RECT &rcDest, DWORD dwRop)
    {
        return (t_bManaged ? Flush(hDC, rcDest, rcDest, (dwRop ? dwRop : DIF_TRANSPARENT), m_crTransparent) : FALSE);
    }

	IRegion* GetClipRgn()
	{				
		CRegion* pRegion = new CRegion;
		pRegion->CreateRectRgn(0, 0, m_sizeCanvas.cx, m_sizeCanvas.cy);
		::GetClipRgn(m_hDC,pRegion->GetUIHandle());
		return pRegion;
	}
	void SelectClipBox(const RECT& rect)
	{
		HRGN hRgn = CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
		::SelectClipRgn(m_hDC,hRgn);
		DeleteObject(hRgn);
	}
	void SelectClipRgn(IRegion* rgn)
	{
		::SelectClipRgn(m_hDC,rgn->GetUIHandle());
	}

	HRGN CreateClipOpAnd(RECT* rcArray, int size)
	{
		if(hRgn_ == NULL){
			hRgn_ = CreateRectRgn(0, 0, m_sizeCanvas.cx, m_sizeCanvas.cy);
			::CombineRgn(hRgn_, hRgn_, hRgn_, RGN_XOR);
		}

		HRGN hMid = CreateRectRgn(0, 0, 0, 0);

		for(unsigned i =0; i < size; ++i){
			HRGN h1 = CreateRectRgn(rcArray[i].left, rcArray[i].top, 
				rcArray[i].right, rcArray[i].bottom);
			::CombineRgn(hMid, hMid, h1, RGN_OR);
			DeleteObject(h1);
		}

		::CombineRgn(hRgn_, hRgn_, hMid, RGN_OR);
		::DeleteObject(hMid);
		::SelectClipRgn(m_hDC, hRgn_);
		return NULL;
	}

	virtual void ResetClip(HRGN hRgn)
	{
		::SelectClipRgn(m_hDC, NULL);
		::DeleteObject(hRgn_);

		hRgn_ = NULL;
	}

    int Draw3DRect(RECT rcBounds, COLORREF clr1, COLORREF clr2)
    {
		Draw3dRect(rcBounds.left, rcBounds.top, rcBounds.right - rcBounds.left,
			rcBounds.bottom - rcBounds.top, clr1, clr2);
        return TRUE;
    }

	void Draw3dRect(int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight)
	{
		FillSolidRect(x, y, cx - 2, 1, clrTopLeft);
		FillSolidRect(x, y, 1, cy - 2, clrTopLeft);
		FillSolidRect(x + cx, y, -2, cy, clrBottomRight);
		FillSolidRect(x, y + cy, cx, -2, clrBottomRight);
	}

    BOOL Clear(COLORREF clr)
    {
        HBRUSH	hBrush	=::CreateSolidBrush(clr);

        RECT	rcBounds;
        ZeroMemory(&rcBounds, sizeof(RECT));
        rcBounds.right	=m_sizeCanvas.cx;
        rcBounds.bottom	=m_sizeCanvas.cy;
        ::FillRect(m_hDC, &rcBounds, hBrush);

        DeleteObject(hBrush);

        return TRUE;
    }



	BOOL SetFont(xgdi::IRCFont* pFont)
    {
        if(!pFont) return FALSE;
        m_hOldFont = (HFONT)::SelectObject(m_hDC, (HGDIOBJ)(HFONT)(*(CFont*)pFont));

        return m_hOldFont ? TRUE : FALSE;
    }

    BOOL SetPen(IPen* pPen)
    {
        if(!pPen) return FALSE;
        m_hOldPen = (HPEN)::SelectObject(m_hDC, (HGDIOBJ)(HPEN)(*(CPen*)pPen));

        return m_hOldPen ? TRUE : FALSE;
    }

    int SetBkMode(int mode)
    {
        return ::SetBkMode(m_hDC, mode);
    }

    void SetSize(const SIZE& sizeCanvas, BOOL bNewUiHandle=FALSE)
    {
        m_sizeCanvas = sizeCanvas;
		if(bNewUiHandle)
		{
			HWND hDstp = ::GetDesktopWindow();
			HDC hdc = ::GetDC(hDstp);
			Attach(hdc);
			::ReleaseDC(hDstp, hdc);
		}
    }

    int SetTextColor(int nColor)
    {
        return ::SetTextColor(m_hDC, nColor);
    }
	
    virtual void Release()
    {
        delete this;
    }
public:
	#define GET_ARRAR_LINE(arr,linelen,lineidx) ((arr) + (lineidx) * (linelen))
	//Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
	void FastImageBlur(int * imgArray , int iWidth , int iHeight , int radius)
	{   
		if (radius<1 || imgArray == NULL){ 
			return; 
		}  
		int * pix = imgArray;  

		int w=iWidth;  
		int h=iHeight; 
		int wm=w-1; 
		int hm=h-1; 
		int wh=w*h;
		int div=radius+radius+1;   
		int * r=new int[wh]; 
		int * g=new int[wh]; 
		int * b=new int[wh];
		int rsum,gsum,bsum,x,y,i,p,yp,yi,yw;    
		int * vmin = new int[max(w,h)];
		int divsum=(div+1)>>1;  
		divsum*=divsum;
		int * dv=new int[256*divsum]; 
		for (i=0;i<256*divsum;i++){
			dv[i]=(i/divsum); 
		}   
		yw=yi=0;    
		int stackWidth = 3;
		int* stack = new int[stackWidth * div];   
		int stackpointer;  
		int stackstart; 
		int * sir;  
		int rbs;   
		int r1=radius+1;  
		int routsum,goutsum,boutsum;   
		int rinsum,ginsum,binsum;   
		for (y=0;y<h;y++){   
			rinsum=ginsum=binsum=routsum=goutsum=boutsum=rsum=gsum=bsum=0;      
			for(i=-radius;i<=radius;i++){  
				p=pix[yi+min(wm,max(i,0))];     
				sir=GET_ARRAR_LINE(stack,stackWidth,i+radius);     
				sir[0]=(p & 0xFF0000)>>16;         
				sir[1]=(p & 0x00FF00)>>8;   
				sir[2]=(p & 0x0000FF);       
				rbs=r1-abs(i);    
				rsum+=sir[0]*rbs;    
				gsum+=sir[1]*rbs;         
				bsum+=sir[2]*rbs;     
				if (i>0){         
					rinsum+=sir[0];   
					ginsum+=sir[1];     
					binsum+=sir[2];      
				} else {            
					routsum+=sir[0];      
					goutsum+=sir[1];         
					boutsum+=sir[2];         
				}    
			}     
			stackpointer=radius; 
			for (x=0;x<w;x++){  
				r[yi]=dv[rsum];    
				g[yi]=dv[gsum];      
				b[yi]=dv[bsum];     
				rsum-=routsum;  
				gsum-=goutsum;         
				bsum-=boutsum;        
				stackstart=stackpointer-radius+div;  
				sir=GET_ARRAR_LINE(stack,stackWidth,(stackstart%div));
				routsum-=sir[0];        
				goutsum-=sir[1];         
				boutsum-=sir[2];    
				if(y==0){        
					vmin[x]=min(x+radius+1,wm);  
				}        
				p=pix[yw+vmin[x]];  
				sir[0]=(p & 0xFF0000)>>16;    
				sir[1]=(p & 0x00FF00)>>8;     
				sir[2]=(p & 0x0000FF);      
				rinsum+=sir[0];       
				ginsum+=sir[1];      
				binsum+=sir[2];     
				rsum+=rinsum;         
				gsum+=ginsum;     
				bsum+=binsum;    
				stackpointer=(stackpointer+1)%div;  
				sir=GET_ARRAR_LINE(stack,stackWidth,stackpointer%div);  
				routsum+=sir[0];        
				goutsum+=sir[1];       
				boutsum+=sir[2];        
				rinsum-=sir[0];       
				ginsum-=sir[1];      
				binsum-=sir[2];    
				yi++;    
			}    
			yw+=w;
		}  
		for (x=0;x<w;x++){  
			rinsum=ginsum=binsum=routsum=goutsum=boutsum=rsum=gsum=bsum=0;   
			yp=-radius*w;    
			for(i=-radius;i<=radius;i++){  
				yi=max(0,yp)+x;   
				sir=GET_ARRAR_LINE(stack,stackWidth,i+radius);   
				sir[0]=r[yi];          
				sir[1]=g[yi];         
				sir[2]=b[yi];          
				rbs=r1-abs(i);       
				rsum+=r[yi]*rbs;        
				gsum+=g[yi]*rbs;          
				bsum+=b[yi]*rbs;           
				if (i>0){             
					rinsum+=sir[0];       
					ginsum+=sir[1];   
					binsum+=sir[2];     
				} else {              
					routsum+=sir[0];    
					goutsum+=sir[1];       
					boutsum+=sir[2];       
				}          
				if(i<hm){        
					yp+=w;     
				}     
			}     
			yi=x; 
			stackpointer=radius;     
			for (y=0;y<h;y++)
			{   
				pix[yi]=0xFF000000 | (dv[rsum]<<16) | (dv[gsum]<<8) | dv[bsum]; 
				rsum-=routsum;    
				gsum-=goutsum;     
				bsum-=boutsum;     
				stackstart=stackpointer-radius+div;    
				sir=GET_ARRAR_LINE(stack,stackWidth,stackstart%div);    
				routsum-=sir[0];        
				goutsum-=sir[1];         
				boutsum-=sir[2];         
				if(x==0){           
					vmin[y]=min(y+r1,hm)*w; 
				}        
				p=x+vmin[y];   
				sir[0]=r[p]; 
				sir[1]=g[p];      
				sir[2]=b[p];      
				rinsum+=sir[0];   
				ginsum+=sir[1];   
				binsum+=sir[2];      
				rsum+=rinsum;       
				gsum+=ginsum;      
				bsum+=binsum;        
				stackpointer=(stackpointer+1)%div;     
				sir=GET_ARRAR_LINE(stack,stackWidth,stackpointer);    
				routsum+=sir[0];        
				goutsum+=sir[1];          
				boutsum+=sir[2];      
				rinsum-=sir[0];    
				ginsum-=sir[1];      
				binsum-=sir[2];    
				yi+=w;    
			}  
		}   
		delete []r; 
		delete []g;
		delete []b; 
		delete []vmin;  
		delete []dv;
		delete []stack;
	}  




protected:
    HDC			m_hDC;

    HBITMAP		m_hMemBitmap, m_hOldBitmap;
    HFONT		m_hOldFont;
    HPEN		m_hOldPen;
    CSize		m_sizeCanvas;
    COLORREF    m_crTransparent;
    HRGN		hRgn_;
};

typedef CCanvasT<true>	CCanvas;
typedef CCanvasT<false>	CCanvasHandle;

NAMESPACE_END(xgdi)