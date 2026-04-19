#pragma once

#include "xgdi.h"
#include "xgdidef.h"

NAMESPACE_BEGIN(xgdi)

inline BYTE HueToRGB(float rm1,float rm2,float rh)
{
	while(rh > 360.0f)
		rh -= 360.0f;
	while(rh < 0.0f)
		rh += 360.f;

	if(rh < 60.0f)
		rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
	else if(rh < 180.0f)
		rm1 = rm2;
	else if(rh < 240.0f)
		rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;

	float n = rm1*255;
	int m = min((int)n,255);
	m = max(0,m);
	return (BYTE)m;//(rm1 * 255);
}

inline void RGBtoHSL(BYTE R,BYTE G,BYTE B,float* H,float* S,float* L)
{
	BYTE minval = min(R,min(G,B));
	BYTE maxval = max(R,max(G,B));
	float mdiff = float(maxval) - float(minval);
	float msum  = float(maxval) + float(minval);

	*L = msum / 510.0f;

	if (maxval == minval) 
	{
		*S = 0.0f;
		*H = 0.0f; 
	}   
	else 
	{ 
		float rnorm = (maxval - R) / mdiff;      
		float gnorm = (maxval - G) / mdiff;
		float bnorm = (maxval - B) / mdiff;   

		*S = (*L <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

		if(R == maxval) 
			*H = 60.0f * (6.0f + bnorm - gnorm);
		if(G == maxval) 
			*H = 60.0f * (2.0f + rnorm - bnorm);
		if(B == maxval) 
			*H = 60.0f * (4.0f + gnorm - rnorm);
		if (*H > 360.0f) 
			*H -= 360.0f;
	}
}

inline void HSLtoRGB(BYTE& R, BYTE& G, BYTE& B, float H,float S,float L)
{
	BYTE r,g,b;

	L = min(1,L);
	S = min(1,S);

	if(S == 0.0)
	{
		r = g = b = (BYTE)(255 * L);
	} 
	else 
	{
		float rm1, rm2;

		if (L <= 0.5f) 
			rm2 = L + L * S;
		else
			rm2 = L + S - L * S;
		rm1 = 2.0f * L - rm2;   

		r = HueToRGB(rm1, rm2, H + 120.0f);
		g = HueToRGB(rm1, rm2, H);
		b = HueToRGB(rm1, rm2, H - 120.0f);
	}
	R = r;
	G = g;
	B = b;
}

inline void SetRgbHSL(unsigned char &R, unsigned char &G, unsigned char &B, float fFactor, COLORREF crColour, COLORREF crClip=RGB(255,0,255))
{

	BYTE rClip = GetRValue(crClip);
	BYTE gClip = GetGValue(crClip);
	BYTE bClip = GetBValue(crClip);

	if (rClip == R && gClip == G && bClip == B)
	{
		return;
	}

	BYTE Rdes = GetRValue(crColour);
	BYTE Gdes = GetGValue(crColour);
	BYTE Bdes = GetBValue(crColour);

	float Hsrc, Ssrc, Lsrc;
	float Hdes, Sdes, Ldes;
	RGBtoHSL(R, G, B, &Hsrc, &Ssrc, &Lsrc);
	RGBtoHSL(Rdes, Gdes, Bdes, &Hdes, &Sdes, &Ldes);
	BYTE r,g,b;
	float Llast = Lsrc + Ldes - fFactor;
	if (Llast < 0.0)
		Llast = 0.0;
	if(Llast > 1.0)
		Llast = 1.0;
	HSLtoRGB(r, g, b, Hdes, Sdes, Llast);
	R = r;
	G = g;
	B = b;
}

inline BOOL AdjustHSL(BYTE& R, BYTE& G, BYTE& B, int degHue,int perSaturation,int perLuminosity, COLORREF crClip=RGB(255,0,255))
{	
	BYTE rClip = GetRValue(crClip);
	BYTE gClip = GetGValue(crClip);
	BYTE bClip = GetBValue(crClip);

	if (rClip == R && gClip == G && bClip == B)
	{
		return FALSE;
	}

	float H,S,L;

	RGBtoHSL(R,G,B,&H,&S,&L);

	H += degHue;
	S = (S*perSaturation/100.0f);
	L = (L*perLuminosity/100.0f);

	BYTE Rdes, Gdes, Bdes;
	HSLtoRGB(Rdes, Gdes, Bdes, H, S, L);
	R = Rdes;
	G = Gdes;
	B = Bdes;
	return TRUE;
}



inline COLORREF GetColorLum(COLORREF color, double modulus, int flag = 2)
{
	if (modulus < 0 || modulus > 1)
		return -1;
	float h, s, l;
	xgdi::RGBtoHSL(GetRValue(color),GetGValue(color),GetBValue(color), &h, &s, &l);
	if (flag == 0)//减小
		l = l*modulus;
	else if (flag == 1)//增大
		l = l*(1+modulus)>1 ? 1 : l*(1+modulus);
	else if (flag == 2)//定值
		l = modulus;
	else
		return -1;

	BYTE r = GetRValue(color);
	BYTE g = GetGValue(color);
	BYTE b = GetBValue(color);
	xgdi::HSLtoRGB(r,g,b, h, s, l);
	return color = RGB(r,g,b); 
}
inline BOOL IsDark(COLORREF color, double darkF=0.6)
{
	int r = 0, g = 0, b = 0;
	r = GetRValue(color);
	g = GetGValue(color);
	b = GetBValue(color);
	float h, s, l;
	xgdi::RGBtoHSL(GetRValue(color),GetGValue(color),GetBValue(color), &h, &s, &l);
	if (l < darkF)
		return TRUE;
	return FALSE;
}
NAMESPACE_END(xgdi)
