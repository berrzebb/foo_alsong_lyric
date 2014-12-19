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

#include "resource.h"
#include "AlsongLyricLinkDialog.h"
#include "Lyric.h"
#include "LyricManager.h"
#include "AlsongLyric.h"
#include "LyricSourceAlsong.h"

AlsongLyricLinkDialog *g_LyricLinkDialog = NULL; //only one dialog

AlsongLyricLinkDialog::AlsongLyricLinkDialog(HWND hWndParent, const metadb_handle_ptr &track) : m_page(0), m_lyriccount(0)
{
	g_LyricLinkDialog = this;
	m_track = track;
	DialogBoxParam(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_LYRIC_MODIFY), hWndParent, (DLGPROC)&AlsongLyricLinkDialog::LyricModifyDialogProc, (LPARAM)this);
}

AlsongLyricLinkDialog::~AlsongLyricLinkDialog()
{
	EndDialog(m_hWnd, 0);
	g_LyricLinkDialog = NULL;
}

void AlsongLyricLinkDialog::OpenLyricLinkDialog(HWND hWndParent, const metadb_handle_ptr &track)
{
	if(g_LyricLinkDialog)
		return;
	g_LyricLinkDialog = new AlsongLyricLinkDialog(hWndParent, track);
	delete g_LyricLinkDialog;
	g_LyricLinkDialog = NULL;
}

void AlsongLyricLinkDialog::PopulateListView()
{
	HWND hListView = GetDlgItem(m_hWnd, IDC_LYRICLIST);
	Lyric *lrc = m_searchresult->Get();
	std::stringstream str;
	str << m_page * 100 + 1 << "~" << min(m_lyriccount, (m_page + 1) * 100) << "/" << m_lyriccount;
	uSetDlgItemText(m_hWnd, IDC_STATUS, str.str().c_str());
	int n = 0;
	ListView_DeleteAllItems(hListView);
	do
	{
		std::wstring artist = pfc::stringcvt::string_wide_from_utf8(lrc->GetArtist().c_str()).get_ptr();
		std::wstring title = pfc::stringcvt::string_wide_from_utf8(lrc->GetTitle().c_str()).get_ptr();
		LVITEM item;
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = n ++;
		item.iSubItem = 0;
		item.pszText = const_cast<WCHAR *>(artist.c_str());
		item.lParam = lrc->GetInternalID();
		ListView_InsertItem(hListView, &item);
		item.iSubItem = 1;
		item.mask = LVIF_TEXT;
		item.pszText = const_cast<WCHAR *>(title.c_str());
		ListView_SetItem(hListView, &item);
	}while((lrc = m_searchresult->Get()), lrc->HasLyric());
}

UINT CALLBACK AlsongLyricLinkDialog::LyricModifyDialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	AlsongLyricLinkDialog *_this;
	if(iMessage == WM_INITDIALOG)
	{
		_this = (AlsongLyricLinkDialog *)lParam;
		_this->m_hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)_this);
	}
	else
		_this = (AlsongLyricLinkDialog *)GetWindowLongPtr(hWnd, GWL_USERDATA);
	return _this->DialogProc(iMessage, wParam, lParam);
}

UINT AlsongLyricLinkDialog::DialogProc(UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
	case WM_CLOSE:
		EndDialog(m_hWnd, 0);
		return TRUE;
	case WM_DESTROY:
		return TRUE;
	case WM_INITDIALOG:
		{
			uSetWindowText(m_hWnd, m_track->get_path());

			//set artist, title field
			service_ptr_t<titleformat_object> to;
			pfc::string8 artist;
			pfc::string8 title;

			static_api_ptr_t<titleformat_compiler>()->compile_safe(to, "%artist%");
			m_track->format_title(NULL, artist, to, NULL);
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to, "%title%");
			m_track->format_title(NULL, title, to, NULL);
			uSetDlgItemText(m_hWnd, IDC_ARTIST, artist.get_ptr());
			uSetDlgItemText(m_hWnd, IDC_TITLE, title.get_ptr());
			//perform listview initialization.

			LVCOLUMN lv;
			lv.mask = LVCF_WIDTH | LVCF_TEXT;
			lv.cx = 150;
			lv.pszText = TEXT("아티스트");
			ListView_InsertColumn(GetDlgItem(m_hWnd, IDC_LYRICLIST), 0, &lv);
			lv.pszText = TEXT("제목");
			ListView_InsertColumn(GetDlgItem(m_hWnd, IDC_LYRICLIST), 1, &lv);

			SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) | WS_DISABLED); //disable next, prev button
			SetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) | WS_DISABLED);
		}
		return TRUE;
	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			if(hdr->code == LVN_ITEMCHANGED && hdr->idFrom == IDC_LYRICLIST)
			{
				int nSel;
				NMLISTVIEW *nlv = (NMLISTVIEW *)lParam;
				nSel = nlv->iItem;
				LVITEM litem;
				litem.mask = LVIF_PARAM;
				litem.iItem = nSel;
				litem.iSubItem = 0;
				ListView_GetItem(GetDlgItem(m_hWnd, IDC_LYRICLIST), &litem);
				Lyric *res = m_searchresult->Get((int)litem.lParam);
				std::string lyric = res->GetRawLyric();
				boost::replace_all(lyric, "<br>", "\r\n");
				uSetDlgItemText(m_hWnd, IDC_LYRIC, lyric.c_str());
			}
		}
		return TRUE;
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case IDC_SEARCH:
				{
					pfc::string8 artist;
					uGetDlgItemText(m_hWnd, IDC_ARTIST, artist);

					pfc::string8 title;
					uGetDlgItemText(m_hWnd, IDC_TITLE, title);


					if(artist.get_length() == 0)
					{

						//MessageBox(m_hWnd, TEXT("아티스트를 입력해 주세요"), TEXT("에러"), MB_OK);
						//return TRUE;
						/// 아티스트가 입력되지 않았다면 기본값으로 공백
						artist.set_string("");
					}


					if(title.get_length() == 0)
					{
						///MessageBox(m_hWnd, TEXT("제목을 입력해 주세요"), TEXT("에러"), MB_OK);
						///return TRUE;
						/// 제목이 입력되지 않았다면 기본값으로 공백
						title.set_string("");
					}
					if(artist.toString() == "" && title.toString() == "")
					{
						MessageBox(m_hWnd, TEXT("제목이나 아티스트명을 입력해 주세요"), TEXT("에러"), MB_OK);
						return TRUE;
					}
					/// 아티스트나 타이틀에 타이틀 포맷이 존재한다면?
					if(!strcmp(artist.toString(),"%artist%"))
					{
						service_ptr_t<titleformat_object> to;
						static_api_ptr_t<titleformat_compiler>()->compile_safe(to, "%artist%");
						m_track->format_title(NULL, artist, to, NULL);
						uSetDlgItemText(m_hWnd, IDC_ARTIST, artist.get_ptr());
					}
					if(!strcmp(title.toString(),"%title%"))
					{
						service_ptr_t<titleformat_object> to;
						static_api_ptr_t<titleformat_compiler>()->compile_safe(to, "%title%");
						m_track->format_title(NULL, title, to, NULL);
						uSetDlgItemText(m_hWnd, IDC_TITLE, title.get_ptr());
					}
					m_page = 0;
					m_lyriccount = LyricSourceAlsong().SearchLyricGetCount(artist.toString(),title.toString());
					m_searchresult = LyricSourceAlsong().SearchLyric(artist.toString(), title.toString(), m_page);
					if(m_searchlistthread){
					m_searchlistthread->interrupt();
					m_searchlistthread->join();
					m_searchlistthread.reset();
					}
					m_searchlistthread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&AlsongLyricLinkDialog::PopulateListView, this)));
					SetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE) | WS_DISABLED);
					if(m_lyriccount > 100)
						SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) & ~WS_DISABLED);
					else
						SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) | WS_DISABLED);
				}
				break;
			case IDC_RESET:
				SetDlgItemText(m_hWnd, IDC_ARTIST, TEXT(""));
				SetDlgItemText(m_hWnd, IDC_TITLE, TEXT(""));
				SetDlgItemText(m_hWnd, IDC_STATUS, TEXT(""));
				ListView_DeleteAllItems(GetDlgItem(m_hWnd, IDC_LYRICLIST));
				SetDlgItemText(m_hWnd, IDC_LYRIC, TEXT(""));
				SetFocus(GetDlgItem(m_hWnd, IDC_ARTIST));
				SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) | WS_DISABLED);
				SetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) | WS_DISABLED);
				SetWindowLong(GetDlgItem(m_hWnd, IDC_ARTIST), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_ARTIST), GWL_STYLE) & ~WS_DISABLED);
				SetWindowLong(GetDlgItem(m_hWnd, IDC_TITLE), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_ARTIST), GWL_STYLE) & ~WS_DISABLED);
				//reset;
				break;
			case IDC_NEWLYRIC:
				//something
				break;
			case IDC_PREV:
				{
					if(m_page == 0)
						return TRUE;
					m_page --;
					pfc::string8 artist;
					uGetDlgItemText(m_hWnd, IDC_ARTIST, artist);
					pfc::string8 title;
					uGetDlgItemText(m_hWnd, IDC_TITLE, title);
					m_searchresult = LyricSourceAlsong().SearchLyric(artist.toString(), title.toString(), m_page);
					if(m_searchlistthread){
					m_searchlistthread->interrupt();
					m_searchlistthread->join();
					m_searchlistthread.reset();
					}
					m_searchlistthread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&AlsongLyricLinkDialog::PopulateListView, this)));
					if(m_page != 0)
						SetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE) & ~WS_DISABLED);
					else
						SetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE) | WS_DISABLED);
					if(m_lyriccount / 100 != m_page)
						SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) & ~WS_DISABLED);
					else
						SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) | WS_DISABLED);
				}
				break;
			case IDC_NEXT:
				{
					if(m_page == m_lyriccount / 100)
						return TRUE;
					m_page ++;
					pfc::string8 artist;
					uGetDlgItemText(m_hWnd, IDC_ARTIST, artist);
					pfc::string8 title;
					uGetDlgItemText(m_hWnd, IDC_TITLE, title);

					m_searchresult = LyricSourceAlsong().SearchLyric(artist.toString(), title.toString(), m_page);
					if(m_searchlistthread){
					m_searchlistthread->interrupt();
					m_searchlistthread->join();
					m_searchlistthread.reset();
					}
					m_searchlistthread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&AlsongLyricLinkDialog::PopulateListView, this)));

					if(m_page != 0)
						SetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE) & ~WS_DISABLED);
					else
						SetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_PREV), GWL_STYLE) | WS_DISABLED);
					if(m_lyriccount / 100 != m_page)
						SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) & ~WS_DISABLED);
					else
						SetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE, GetWindowLong(GetDlgItem(m_hWnd, IDC_NEXT), GWL_STYLE) | WS_DISABLED);

				}
				break;
			case IDC_SYNCEDIT:
				break;
			case IDC_REGISTER:
				{
					int nSel;
					nSel = SendMessage(GetDlgItem(m_hWnd, IDC_LYRICLIST), LVM_GETSELECTIONMARK, 0, 0);
					LVITEM litem;
					litem.mask = LVIF_PARAM;
					litem.iItem = nSel;
					litem.iSubItem = 0;
					ListView_GetItem(GetDlgItem(m_hWnd, IDC_LYRICLIST), &litem);
					if(LyricSourceAlsong().Save(m_track, *m_searchresult->Get(litem.lParam)))
					{
						MessageBox(m_hWnd, TEXT("등록 성공"), TEXT("안내"), MB_OK);

						static_api_ptr_t<play_control> pc;
						metadb_handle_ptr p_track;
						pc->get_now_playing(p_track);
						if(p_track == m_track)
							LyricManager::Reload(p_track);

						EndDialog(m_hWnd, 0);
						return TRUE;
					}
					MessageBox(m_hWnd, TEXT("등록 실패"), TEXT("안내"), MB_OK);
				}
				break;
			case IDC_CANCEL:
				EndDialog(m_hWnd, 0);
				break;
			}
		}
		return TRUE;
	}
	return FALSE;
}
