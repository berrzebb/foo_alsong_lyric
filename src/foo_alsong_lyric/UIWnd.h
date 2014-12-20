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

#include "UIManager.h"

class UIWnd
{
private:
	HWND m_hWnd;
	UIManager *m_UI;
	IPropertyStore *m_Propstore;
	int AddTaskList(std::wstring command, std::wstring display, std::wstring appid);
	int AddTaskList(std::vector<std::pair<std::wstring,std::wstring>> data,std::wstring appid);
	bool m_isResizing;
	bool m_isBlur; //determines if windows is blurred by dwm
public:
	UIWnd();
	void Show();
	void Hide();
	HWND Create();
	void Destroy(); 
	void StyleUpdated();
	void StartResize()
	{
		m_isResizing = true;
		StyleUpdated();
	}

	void EndResize()
	{
		m_isResizing = false;
		StyleUpdated();
	}

	HWND GetHWND()
	{
		return m_hWnd;
	}

	int isResizing()
	{
		return m_isResizing;
	}

	const static int Resize_border;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

extern UIWnd WndInstance;