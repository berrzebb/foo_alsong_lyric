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

#include "LyricSourceAlsong.h"
#include "LyricSearchResult.h"

class AlsongLyricLinkDialog
{
private:
	HWND m_hWnd;
	metadb_handle_ptr m_track;
	int m_lyriccount;
	boost::shared_ptr<LyricSearchResult> m_searchresult;
	boost::shared_ptr<boost::thread> m_searchlistthread;
	boost::mutex m_SecondLock; //lock tick/seconds
	int m_page;

	static UINT CALLBACK LyricModifyDialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	UINT DialogProc(UINT iMessage, WPARAM wParam, LPARAM lParam);
	void PopulateListView();
public:
	AlsongLyricLinkDialog(HWND hWndParent, const metadb_handle_ptr &track = NULL);
	~AlsongLyricLinkDialog();

	static void OpenLyricLinkDialog(HWND hWndParent, const metadb_handle_ptr &track = NULL);
};