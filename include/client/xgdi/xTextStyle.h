#pragma once
#include "xgdi.h"
#include <Gdiplus.h>

NAMESPACE_BEGIN(xgdi)

class CTextStyle
{
public:
	static SIZE CalcTextExtentSize(const xstring& text,LPCWSTR fontName,int fontSize,DWORD dwFontEffect = FONT_EFFECT_NORMAL)
	{
		//just support single line
		using namespace Gdiplus;
		GraphicsPath path;		
		FontFamily fontFamily(fontName);
		StringFormat strformat(StringFormat::GenericTypographic());
		strformat.SetTrimming(StringTrimmingNone);
		strformat.SetFormatFlags(StringFormatFlagsNoClip | StringFormatFlagsNoFitBlackBox | StringFormatFlagsNoWrap);

		int fs = FontStyleRegular;
		if(dwFontEffect & FONT_EFFECT_BOLD)fs |= FontStyleBold;			
		if(dwFontEffect & FONT_EFFECT_ITALIC)fs |= FontStyleItalic;
		if(dwFontEffect & FONT_EFFECT_UNDERLINE)fs |= FontStyleUnderline;

		path.AddString(text.c_str(), text.size(), &fontFamily, 
						fs, fontSize, PointF(0,0), &strformat );
		RectF rcBound;
		path.GetBounds(&rcBound);
		SIZE s = {rcBound.Width,rcBound.Height};
		s.cx += -rcBound.GetLeft() - s.cx;
		return s;

	}
	static inline void DrawFlagToFormat(DWORD flag,Gdiplus::StringFormat& format)
	{
		if(flag & DT_CENTER)
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
		else if(flag & DT_RIGHT)
			format.SetAlignment(Gdiplus::StringAlignmentFar);
		else
			format.SetAlignment(Gdiplus::StringAlignmentNear);			

		if(flag & DT_VCENTER)
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	}
	static void RenderGrowText(HDC hDC,LPCWSTR text,const RECT& rect,int flag,LPCWSTR fontName,int fontSize,COLORREF fontclr,COLORREF growclr,DWORD dwFontEffect = FONT_EFFECT_NORMAL,int growlen = 5)
	{
		using namespace Gdiplus;
		Graphics graphics(hDC);
		graphics.SetSmoothingMode(SmoothingModeHighQuality);
		graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
		//graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
		graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
		FontFamily fontFamily(fontName);
		StringFormat strformat(Gdiplus::StringFormat::GenericTypographic());		
		strformat.SetTrimming(StringTrimmingNone);
		strformat.SetFormatFlags(StringFormatFlagsNoClip | StringFormatFlagsNoFitBlackBox | StringFormatFlagsNoWrap);
		DrawFlagToFormat(flag,strformat);
		
		int fs = FontStyleRegular;
		if(dwFontEffect & FONT_EFFECT_BOLD)fs |= FontStyleBold;			
		if(dwFontEffect & FONT_EFFECT_ITALIC)fs |= FontStyleItalic;
		if(dwFontEffect & FONT_EFFECT_UNDERLINE)fs |= FontStyleUnderline;
		
		GraphicsPath path;
		RectF rcText(rect.left,rect.top,rect.right - rect.left, rect.bottom - rect.top);

		path.AddString(text, wcslen(text), &fontFamily, 
			fs, fontSize, rcText, &strformat );

		static int f = 30;//255;
		//graphics.DrawRectangle(&Pen(Color(255,0,0)),rcText);
		int a = f/(growlen+1);
		int r = GetRValue(growclr);
		int g = GetGValue(growclr);
		int b = GetBValue(growclr);
		for(int i=1; i<=growlen; ++i)
		{			
			Pen pen(Color(f - a * i,r,g,b), i);
			pen.SetLineJoin(LineJoinRound);
			graphics.DrawPath(&pen, &path);
		}

		SolidBrush brush(Color(GetRValue(fontclr),GetGValue(fontclr),GetBValue(fontclr)));

		static bool g_ff = true;

		if(g_ff)
		{
			graphics.FillPath(&brush, &path);		
			//graphics.FillPath(&brush, &path);		
		}
		else
		{

			
			Gdiplus::Font _fontf(fontName,fontSize,fs,Gdiplus::UnitPixel);
			rcText.Offset(1,0);
			graphics.DrawString(text,wcslen(text),
				&_fontf,
				rcText,
				&strformat,
				&brush);
		}


	}

	static void RenderSharpText(HDC hDC,LPCWSTR text,const RECT& rcText,int flag,LPCWSTR fontName,int fontSize,COLORREF fontclr,DWORD dwFontEffect = FONT_EFFECT_NORMAL,int repeat = 1,int textRenderingHint = 5)
	{
		using namespace Gdiplus;
		Graphics graphics(hDC);

		FontFamily fontFamily(fontName);
		StringFormat format(Gdiplus::StringFormat::GenericTypographic());		
		format.SetTrimming(StringTrimmingNone);
		format.SetFormatFlags(StringFormatFlagsNoClip | StringFormatFlagsNoFitBlackBox | StringFormatFlagsNoWrap);

		DrawFlagToFormat(flag,format);
			
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);			
		graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);			
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		graphics.SetTextRenderingHint((TextRenderingHint)textRenderingHint);//Gdiplus::TextRenderingHintClearTypeGridFit/*TextRenderingHintAntiAlias*/

		int fs = FontStyleRegular;
		if(dwFontEffect & FONT_EFFECT_BOLD)fs |= FontStyleBold;			
		if(dwFontEffect & FONT_EFFECT_ITALIC)
			fs |= FontStyleItalic;
		if(dwFontEffect & FONT_EFFECT_UNDERLINE)fs |= FontStyleUnderline;
		Gdiplus::Font fontf(fontName,fontSize,fs,Gdiplus::UnitPixel);


		Gdiplus::RectF rcTextF(rcText.left,rcText.top,rcText.right - rcText.left,rcText.bottom - rcText.top);
		Gdiplus::SolidBrush brush(Gdiplus::Color(GetRValue(fontclr),GetGValue(fontclr),GetBValue(fontclr)));

		int len = _tcslen(text);
		
		//graphics.DrawRectangle(&Pen(Color(255,0,0)),rcTextF);
		for (int i=0;i<repeat;i++)
		{
			graphics.DrawString(text,len,
				&fontf,
				rcTextF,
				&format,
				&brush);
		}

	}
};


NAMESPACE_END(xgdi)
