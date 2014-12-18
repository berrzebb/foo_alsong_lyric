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

#include "stdafx.h"
#include "UICanvas.h"
#include "UIWnd.h"
#include "ConfigStore.h"

UICanvas::UICanvas(HWND hWnd, HDC hdc) : m_hWnd(hWnd), m_destDC(hdc)
{
	GetClientRect(hWnd, &m_DrawRect);

	//double buffer
	m_hDC = CreateCompatibleDC(hdc);
	HBITMAP hBitmap;
	if(GetParent(m_hWnd) == NULL && (GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED))
	{
		RECT rt;
		GetWindowRect(m_hWnd, &rt);
		m_DrawRect.right = rt.right - rt.left;
		m_DrawRect.bottom = rt.bottom - rt.top;
		BITMAPINFO bmi;
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = m_DrawRect.right;
		bmi.bmiHeader.biHeight = -m_DrawRect.bottom;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB; 
		bmi.bmiHeader.biSizeImage = 0;
		bmi.bmiHeader.biXPelsPerMeter = 0;
		bmi.bmiHeader.biYPelsPerMeter = 0;
		bmi.bmiHeader.biClrUsed = 0;
		bmi.bmiHeader.biClrImportant = 0;

		hBitmap = CreateDIBSection(m_destDC, &bmi, DIB_RGB_COLORS, (void **)&m_bits, NULL, NULL);
		memset(m_bits, 0, sizeof(int) * m_DrawRect.right * m_DrawRect.bottom);
	}
	else
		hBitmap = CreateCompatibleBitmap(hdc, m_DrawRect.right, m_DrawRect.bottom);
	m_hOldBitmap = (HBITMAP)SelectObject(m_hDC, hBitmap);

	//FillRect(m_hDC, &m_DrawRect, (HBRUSH)(COLOR_WINDOW + 1));
}

UICanvas::UICanvas(HWND hWnd, HDC hdc, const RECT &drawRect) : m_hWnd(hWnd), m_destDC(hdc)
{
	memcpy(&m_DrawRect, &drawRect, sizeof(RECT));

	m_hDC = CreateCompatibleDC(hdc);
	HBITMAP hBitmap;
	hBitmap = CreateCompatibleBitmap(hdc, m_DrawRect.right, m_DrawRect.bottom);
	m_hOldBitmap = (HBITMAP)SelectObject(m_hDC, hBitmap);
}

UICanvas::~UICanvas()
{
	if(WndInstance.isResizing() && GetParent(m_hWnd) == NULL)
	{
		Gdiplus::Graphics g(m_hDC);
		Gdiplus::Pen pen(Gdiplus::Color(255, 0, 0, 0), (float)UIWnd::Resize_border);
		g.DrawRectangle(&pen, 0, 0, m_DrawRect.right - 1, m_DrawRect.bottom - 1);
	}

	if(GetParent(m_hWnd) == NULL && (GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED))
	{
		BLENDFUNCTION blend = {0};
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;
		SIZE sizeWnd = {m_DrawRect.right, m_DrawRect.bottom};
		POINT ptSrc = {0, 0};

		UpdateLayeredWindow(m_hWnd, m_destDC, NULL, &sizeWnd, m_hDC, &ptSrc, NULL, &blend, ULW_ALPHA);
	}
	else
		BitBlt(m_destDC, m_DrawRect.left, m_DrawRect.top, m_DrawRect.right - m_DrawRect.left, m_DrawRect.bottom - m_DrawRect.top, m_hDC, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(m_hDC, m_hOldBitmap));
	DeleteDC(m_hDC);
}

void UICanvas::RegisterCanvas()
{
	SqPlus::SQClassDefNoConstructor<UICanvas>(TEXT("UICanvas")).
		func(&UICanvas::DrawText, TEXT("DrawText")).
		func(&UICanvas::EstimateText, TEXT("EstimateText")).
		func(&UICanvas::SetDrawTextOrigin, TEXT("SetDrawTextOrigin")).
		func(&UICanvas::GetCanvasSize, TEXT("GetCanvasSize")).
		func(&UICanvas::Fill, TEXT("Fill")).
		func(&UICanvas::DrawImage, TEXT("DrawImage")).
		func(&UICanvas::SetTransparent, TEXT("SetTransparent"));

	SqPlus::SQClassDefNoConstructor<UIFont>(TEXT("UIFont")).
		overloadConstructor<UIFont(*)()>();

	SqPlus::SQClassDefNoConstructor<UISize>(TEXT("UISize")).
		overloadConstructor<UISize(*)()>().
		overloadConstructor<UISize(*)(int, int)>().
		var(&UISize::width, TEXT("width")).
		var(&UISize::height, TEXT("height"));

	SqPlus::SQClassDefNoConstructor<UIPoint>(TEXT("UIPoint")).
		overloadConstructor<UIPoint(*)()>().
		overloadConstructor<UIPoint(*)(int, int)>().
		var(&UIPoint::x, TEXT("x")).
		var(&UIPoint::y, TEXT("y"));
}

void UICanvas::DrawImage(int x, int y, int width, int height, const SQChar *path)
{
	Gdiplus::Bitmap bitmap(path);
	Gdiplus::Graphics g(m_hDC);
	g.DrawImage(&bitmap, x, y, width, height);
}

void UICanvas::Fill(int x, int y, int width, int height, COLORREF color)
{
	if(!m_hDC)
		return;

	Gdiplus::Graphics g(m_hDC);
	Gdiplus::Color brush_color;
	if(GetParent(m_hWnd))
		brush_color.SetFromCOLORREF(color);
	else
		brush_color = Gdiplus::Color((255 * cfg_outer_transparency) / 100, GetRValue(color), GetGValue(color), GetBValue(color));
	Gdiplus::SolidBrush brush(brush_color);	
	g.FillRectangle(&brush, x, y, width, height);
}

void UICanvas::SetTransparent()
{
	HWND wnd_parent = GetParent(m_hWnd);
	if(wnd_parent)
	{
		POINT pt = {0, 0}, pt_old = {0,0};
		MapWindowPoints(m_hWnd, wnd_parent, &pt, 1);
		OffsetWindowOrgEx(m_hDC, pt.x, pt.y, &pt_old);
		BOOL b_ret = SendMessage(wnd_parent, WM_ERASEBKGND,(WPARAM)m_hDC, 0);
		SetWindowOrgEx(m_hDC, pt_old.x, pt_old.y, 0); //notify parent to redraw background
	}
}

void UICanvas::DrawText(const UIFont &font, const SQChar *text, int align, float heightratio, int fontTransparency, COLORREF outlineColor, uint32_t outlineSize)
{
	if(!m_hDC)
		return;
	
	Graphics g(m_hDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	SolidBrush brush(Gdiplus::Color((255 * fontTransparency) / 100, GetRValue(font.m_Color), GetGValue(font.m_Color), GetBValue(font.m_Color)));
	if(GetParent(m_hWnd) == NULL && (GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED))
	{
		g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
		brush.SetColor(Gdiplus::Color((255 * fontTransparency) / 100, GetRValue(font.m_Color), GetGValue(font.m_Color), GetBValue(font.m_Color)));
	}

	StringFormat strformat;
	RectF box;
	if(align == 2)
		strformat.SetAlignment(StringAlignmentCenter);
	else if(align == 1)
		strformat.SetAlignment(StringAlignmentNear);
	else
		strformat.SetAlignment(StringAlignmentFar);
	
	if(outlineSize == 0)
	{ 
		g.DrawString(text + 1, wcslen(text + 1), font.m_Font.get(), Gdiplus::RectF((float)m_TextPos.x, (float)m_TextPos.y, (float)m_DrawRect.right - m_TextPos.x, (float)m_DrawRect.bottom - m_TextPos.y), &strformat, &brush);
	}
	else
	{
		GraphicsPath path;
		FontFamily family;
		font.m_Font->GetFamily(&family);
		path.AddString(text + 1, wcslen(text + 1), &family, font.m_Font->GetStyle(), font.m_Font->GetSize(), Gdiplus::RectF((float)m_TextPos.x, (float)m_TextPos.y, (float)m_DrawRect.right - m_TextPos.x, (float)m_DrawRect.bottom - m_TextPos.y), &strformat);
		Pen pen(Gdiplus::Color((255 * fontTransparency) / 100, GetRValue(outlineColor), GetGValue(outlineColor), GetBValue(outlineColor)), (float)outlineSize);
		pen.SetLineJoin(LineJoinRound);
		//path.Widen(&pen);

		g.DrawPath(&pen, &path);
		g.FillPath(&brush, &path);
	}
	g.MeasureString(text + 1, wcslen(text + 1), font.m_Font.get(), Gdiplus::RectF((float)m_TextPos.x, (float)m_TextPos.y, (float)m_DrawRect.right - m_TextPos.x, (float)m_DrawRect.bottom - m_TextPos.y), &strformat, &box);

	m_TextPos.y += (int)(box.Height * heightratio);
}

void UICanvas::SetDrawTextOrigin(const UIPoint &pt)
{
	m_TextPos.x = pt.x;
	m_TextPos.y = pt.y;
}

UISize UICanvas::GetCanvasSize()
{
	return UISize(m_DrawRect.right, m_DrawRect.bottom);
}

UISize UICanvas::EstimateText(const UIFont &font, const SQChar *text)
{
	if(!m_hDC)
		return UISize(0, 0);

	Graphics g(m_hDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	RectF box;
	StringFormat strformat;
	g.MeasureString(text + 1, wcslen(text + 1), font.m_Font.get(), Gdiplus::RectF((float)m_TextPos.x, (float)m_TextPos.y, (float)m_DrawRect.right - m_TextPos.x, (float)m_DrawRect.bottom - m_TextPos.y), &strformat, &box);

	UISize sz;
	sz.width = (int)box.Width;
	sz.height = (int)box.Height;

	return sz;
}

UIFont::UIFont() : m_Color(0)
{
}

UIFont::UIFont(const UIFontDescription & fontdesc)
{
	m_Color = fontdesc.color;
	int style = 0;
	if(fontdesc.bold)
		style |= Gdiplus::FontStyleBold;
	if(fontdesc.italic)
		style |= Gdiplus::FontStyleItalic;
	if(fontdesc.strikeout)
		style |= Gdiplus::FontStyleStrikeout;
	if(fontdesc.underline)
		style |= Gdiplus::FontStyleUnderline;
	m_Font = boost::shared_ptr<Gdiplus::Font>(new Gdiplus::Font(fontdesc.face, (float)fontdesc.size * 100 / 480, style, Gdiplus::UnitWorld));
}
