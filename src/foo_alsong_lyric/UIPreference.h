/*
* foo_alsong_lyric														
* Copyright (C) 2007-2010 Inseok Lee <dlunch@gmail.com>
*
* This library is free software; you can redistribute it and/or modify it 
* under the terms of the GNU Lesser General Public License as published 
* by the Free Software Foundation; version 2.1 of the License.
*
* This library is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
* See the GNU Lesser General Public License for more details.
*
* You can receive a copy of the GNU Lesser General Public License from 
* http://www.gnu.org/
*/

#pragma once

#include "UICanvas.h"
#include "EncodingFunc.h"

class UIPreference
{
public:
	UIPreference() {}
	~UIPreference() {}

	enum AlignPosition
	{
		ALIGN_LEFT = 1,
		ALIGN_TOP = 1,
		ALIGN_MODDLE = 2,
		ALIGN_BOTTOM = 3,
		ALIGN_RIGHT = 3,
	};
	
	enum BgType
	{
		BG_SOLIDCOLOR = 0,
		BG_IMAGE = 1,
		BG_TRANSPARENT = 2,
	};

	UIFont GetNormalFont()
	{
		return UIFont(normalFont);
	}

	UIFont GetHighlightFont()
	{
		return UIFont(highlightFont);
	}

	COLORREF GetBkColor()
	{
		return backColor;
	}
	WCHAR *GetBgImagePath()
	{
		return bgImage;
	}
	int GetBgType()
	{
		return (int)bgType;
	}
	unsigned int GetnLine()
	{
		return nLine;
	}
	unsigned int GetLineMargin()
	{
		return LineMargin;
	}
	int GetVerticalAlign()
	{
		return VerticalAlign;
	}
	int GetHorizentalAlign()
	{
		return HorizentalAlign;
	}
	int GetFontTransparency()
	{
		return (fontTransparency == 100) ? 99 : fontTransparency;
	}
	UIFontDescription OpenFontPopup(HWND hWndFrom, const UIFontDescription &tmpl)
	{
		UIFontDescription out;
		CHOOSEFONT cf;
		memset(&cf, 0, sizeof(CHOOSEFONT));
		cf.lStructSize = sizeof(CHOOSEFONT);
		cf.hwndOwner = hWndFrom;
		LOGFONT lf = {0,};
		lstrcpy(lf.lfFaceName, tmpl.face);
		lf.lfHeight = -(int)((float)tmpl.size * 72 / 480 + 3);
		lf.lfWeight = tmpl.bold ? FW_BOLD : FW_NORMAL;
		cf.lpLogFont = &lf;
		cf.lpLogFont->lfItalic = tmpl.italic;
		cf.lpLogFont->lfStrikeOut = tmpl.strikeout;
		cf.lpLogFont->lfUnderline = tmpl.underline;
		cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_SCREENFONTS;
		cf.nFontType = BOLD_FONTTYPE | ITALIC_FONTTYPE | REGULAR_FONTTYPE | SCREEN_FONTTYPE;
		cf.rgbColors = tmpl.color;
		ChooseFont(&cf);
		lstrcpy(out.face, cf.lpLogFont->lfFaceName);
		out.size = (int)((float)(-cf.lpLogFont->lfHeight - 3) * 480 / 72 + 0.5f);
		out.bold = (cf.lpLogFont->lfWeight > FW_NORMAL); 
		out.italic = cf.lpLogFont->lfItalic;
		out.strikeout = cf.lpLogFont->lfStrikeOut;
		out.underline = cf.lpLogFont->lfUnderline;
		out.color = cf.rgbColors;
		return out;
	}
	int OpenFgColorPopup(HWND hWndFrom)
	{
		COLORREF color;
		if((color = OpenColorPopup(hWndFrom, oldFontColor)) != -1)
		{
			//fgColor = color;
			return color;
		}
		return -1;
	}
	int OpenBkColorPopup(HWND hWndFrom)
	{
		COLORREF color;
		if((color = OpenColorPopup(hWndFrom, backColor)) != -1)
		{/*
			bgType = BG_SOLIDCOLOR;
			bkColor = color;*/
			return color;
		}
		return -1;
	}
	std::wstring OpenBgImagePopup(HWND hWndFrom)
	{
		TCHAR temp[255] = {0,};
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = temp;
		ofn.lpstrFilter = TEXT("그림 파일(*.bmp;*.png;*.jpg;*.gif;*.jpeg)\0*.bmp;*.png;*.jpg;*.gif;*.jpeg\0\0");
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
		ofn.nMaxFile = 255;

		if(GetOpenFileName(&ofn))
		{
			//bgType = BG_IMAGE;
			return std::wstring(temp);
		}
		return std::wstring();
	}
	void SetDefault()
	{
		//oldFont = t_font_description::g_from_font((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		backColor = RGB(255, 255, 255);
		//oldFontColor = RGB(0, 0, 0);
		bgImage[0] = 0;
		bgType = BG_SOLIDCOLOR;
		nLine = 3;
		LineMargin = 100;
		VerticalAlign = ALIGN_MODDLE;
		HorizentalAlign = ALIGN_MODDLE;
		fontTransparency = 100;

		lstrcpy(normalFont.face, TEXT("돋움체"));
		normalFont.bold = 0;
		normalFont.color = RGB(0, 0, 0);
		normalFont.italic = 0;
		normalFont.size = 65;
		normalFont.strikeout = 0;
		normalFont.underline = 0;
		memcpy(&highlightFont, &normalFont, sizeof(UIFontDescription));
		highlightFont.bold = 1;
		memset(bReserved, 0, sizeof(bReserved));
	}
	void Ready()
	{
		//after copied
		if(nLine == 0 || VerticalAlign == 0 || HorizentalAlign == 0)
			SetDefault();
		if(normalFont.face[0] == 0 || normalFont.size == 0) //migration
		{
			lstrcpy(normalFont.face, EncodingFunc::ToUTF16(oldFont.m_facename).c_str());
			normalFont.bold = 0;
			normalFont.color = oldFontColor;
			normalFont.italic = 0;
			normalFont.size = oldFont.m_height;
			normalFont.strikeout = 0;
			normalFont.underline = 0;
			memcpy(&highlightFont, &normalFont, sizeof(UIFontDescription));
			highlightFont.bold = 1;
		}
		if(fontTransparency == 0)
			fontTransparency = 100; //migration
	}

	COLORREF GetOutlineColor()
	{
		return outLineColor;
	}

	uint32_t GetOutlineSize()
	{
		return outLineSize;
	}

	void OpenConfigPopup(HWND hParent);
	static BOOL CALLBACK ConfigProcDispatcher(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
private:
	COLORREF OpenColorPopup(HWND hWndFrom, COLORREF color)
	{
		static COLORREF acrCustClr[16] = {RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 
			RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 
			RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), 
			RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255)};

		CHOOSECOLOR choosecolor;
		ZeroMemory(&choosecolor, sizeof(choosecolor));
		choosecolor.lStructSize = sizeof(CHOOSECOLOR);
		choosecolor.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT ;
		choosecolor.hwndOwner = hWndFrom;
		choosecolor.lpCustColors = (LPDWORD)acrCustClr;
		choosecolor.rgbResult = color;
		if(ChooseColor(&choosecolor))
			return choosecolor.rgbResult;
		return color;
	}
	BOOL UIConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam, HWND hParent);

	t_font_description oldFont; //not used
	COLORREF backColor;
	COLORREF oldFontColor; //not used
	WCHAR bgImage[MAX_PATH];
	BgType bgType; //0: 색, 1: 이미지, 2:투명한 배경

	DWORD nLine;
	DWORD LineMargin;//%단위

	BYTE VerticalAlign; //상하정렬. 1:위 2:가운데 3:아래
	BYTE HorizentalAlign; //좌우정렬. 1:왼쪽 2:가운데 3:오른쪽
	
	UIFontDescription normalFont;
	UIFontDescription highlightFont;

	int fontTransparency;

	uint32_t outLineSize;
	COLORREF outLineColor;

	BYTE bReserved[446]; //구조체 크기가 변하면 설정이 초기화된다. 나중에 변수 추가할때 여기서 뺄것
};

DECLARE_INSTANCE_TYPE(UIPreference)