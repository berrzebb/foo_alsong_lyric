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
#include "ConfigStore.h"
#include "resource.h"
#include "UIPreference.h"
#include "UIWnd.h"
#include "LyricSource.h"
#include "EncodingFunc.h"
#include "LyricManager.h"

static BOOL CALLBACK UIConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK ConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

class preferences_page_instance_alsong_lyric : public preferences_page_instance
{
private:
	HWND m_hWnd;
	HWND hProp;
	preferences_page_callback::ptr m_callback;
public:
	preferences_page_instance_alsong_lyric(HWND parent, preferences_page_callback::ptr callback) : m_callback(callback)
	{
		m_hWnd = uCreateDialog(IDD_PREF, parent, _PrefConfigProc, (LPARAM)this);
	}

	virtual t_uint32 get_state()
	{
		return 0;
	}

	virtual HWND get_wnd()
	{
		return m_hWnd;
	}

	virtual void apply()
	{
		SendMessage(hProp, PSM_APPLY, NULL, NULL);
	}

	virtual void reset()
	{
		
	}

	static int CALLBACK PropCallback(HWND hWnd, UINT message, LPARAM lParam)
	{
		switch (message)
		{
		case PSCB_PRECREATE:
			{
				LPDLGTEMPLATE lpTemplate = (LPDLGTEMPLATE)lParam;
				DWORD dwOldProtect;
				VirtualProtect(lpTemplate, sizeof(DLGTEMPLATE), PAGE_READWRITE, &dwOldProtect);

				lpTemplate->style = DS_SETFONT | DS_FIXEDSYS | WS_CHILD | WS_SYSMENU | WS_VISIBLE | DS_3DLOOK;
				lpTemplate->dwExtendedStyle = 0;
				return TRUE;
			}
		case PSCB_INITIALIZED:
			{
				ShowWindow(GetDlgItem(hWnd, 0x00000001), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, 0x00000002), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, 0x00003021), SW_HIDE);

				//Property sheet bug. See http://support.microsoft.com/kb/149501
				SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_CONTROLPARENT);
				SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}

		return 0;
	}

	BOOL PrefConfigProc(UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch(iMessage)
		{
		case WM_INITDIALOG:
			{
				PROPSHEETPAGE pages[2];
				HPROPSHEETPAGE hpages[2];
				pages[0].dwSize = sizeof(PROPSHEETPAGE);
				pages[0].dwFlags = PSP_DEFAULT | PSP_USETITLE;
				pages[0].hInstance = core_api::get_my_instance();
				pages[0].pszTemplate = MAKEINTRESOURCE(IDD_COMMON_PREF);
				pages[0].pfnDlgProc = &preferences_page_instance_alsong_lyric::CommonConfigProc;
				pages[0].pszTitle = TEXT("공통 설정");
				pages[0].lParam = NULL;

				pages[1].dwSize = sizeof(PROPSHEETPAGE);
				pages[1].dwFlags = PSP_DEFAULT | PSP_USETITLE;
				pages[1].hInstance = core_api::get_my_instance();
				pages[1].pszTemplate = MAKEINTRESOURCE(IDD_UI_PREF_COMMON);
				pages[1].pfnDlgProc = &UIPreference::ConfigProcDispatcher;
				pages[1].pszTitle = TEXT("외부 창 설정");
				pages[1].lParam = (LPARAM)(&cfg_outer.get_value());

				hpages[0] = CreatePropertySheetPage(&pages[0]);
				hpages[1] = CreatePropertySheetPage(&pages[1]);

				PROPSHEETHEADER psh;
				psh.dwSize = sizeof(psh);
				psh.dwFlags = PSH_DEFAULT | PSH_NOCONTEXTHELP | PSH_USEHICON | PSH_USECALLBACK | PSH_MODELESS;
				psh.hwndParent = m_hWnd;
				psh.hInstance = core_api::get_my_instance();
				psh.hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
				psh.pszCaption = TEXT("알송 가사 설정");
				psh.nPages = 2;
				psh.nStartPage = 0;
				psh.pfnCallback = &preferences_page_instance_alsong_lyric::PropCallback;
				psh.phpage = hpages;

				hProp = (HWND)PropertySheet(&psh);
			}
			return TRUE;
		case WM_SIZE:
			{
				RECT rt;
				GetWindowRect(m_hWnd, &rt);
				HWND hTab = (HWND)SendMessage(hProp, PSM_GETTABCONTROL, 0, 0);
				MoveWindow(hProp, 0, 0, rt.right - rt.left, rt.bottom - rt.top, TRUE);
				MoveWindow(hTab, 10, 3, rt.right - rt.left - 15, rt.bottom - rt.top - 5, TRUE);
			}
			return TRUE;
		case WM_DESTROY:
			SendMessage(hProp, PSM_APPLY, NULL, NULL);//마지막 위치 저장 -> 열때 탭복구
			return TRUE;
		}
		return FALSE;
	}

	static BOOL CALLBACK _PrefConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		static preferences_page_instance_alsong_lyric *_this = NULL;
		if(iMessage == WM_INITDIALOG)
		{
			_this = (preferences_page_instance_alsong_lyric *)lParam;
			_this->m_hWnd = hWnd;
		}
		if(_this)
			return _this->PrefConfigProc(iMessage, wParam, lParam);
		
		return FALSE;
	}

	static BOOL CALLBACK LyricSourceAddProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch(iMessage)
		{
		case WM_INITDIALOG:
			{
				std::vector<boost::shared_ptr<LyricSource> > list = LyricSourceManager::List();
				for(std::vector<boost::shared_ptr<LyricSource> >::iterator it = list.begin(); it != list.end(); it ++)
				{
					int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCES), CB_ADDSTRING, NULL, (LPARAM)EncodingFunc::ToUTF16((*it)->GetName()).c_str());
					SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCES), CB_SETITEMDATA, idx, (LPARAM)(*it).get());
				}
			}
			return TRUE;
		case WM_COMMAND:
			{
				if(HIWORD(wParam) == BN_CLICKED)
				{
					switch(LOWORD(wParam))
					{
					case IDC_LYRICSOURCE_ADD:
						{
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCES), CB_GETCURSEL, NULL, NULL);
							if(idx == CB_ERR)
							{
								MessageBox(hWnd, TEXT("선택 없음"), TEXT("Error"), MB_OK);
								return TRUE;
							}
							EndDialog(hWnd, SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCES), CB_GETITEMDATA, idx, NULL));
							return TRUE;
						}
					case IDC_CANCEL:
						{
							EndDialog(hWnd, 0);
						}
					}
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			break;
		}
		return FALSE;
	}

	static BOOL CALLBACK LyricSourceConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		static LyricSource *src;
		static int type;
		static HFONT font;
		switch(iMessage)
		{
		case WM_INITDIALOG:
			{
				src = (LyricSource *)((std::pair<LyricSource *, int> *)lParam)->first;
				type = ((std::pair<LyricSource *, int> *)lParam)->second;

				//init font
				NONCLIENTMETRICS ncm;
				ncm.cbSize = sizeof(ncm);
				SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), (PVOID)&ncm, NULL);
				font = CreateFontIndirect(&ncm.lfMessageFont);

				std::map<std::string, LyricSource::ConfigItemType> item = src->GetConfigItems(type);
				if(item.size() == 0)
				{
					EndDialog(hWnd, 0);
					return TRUE;
				}
				std::map<std::string, std::string> configitem = src->GetConfig();
				int cnt = 0;
				for(std::map<std::string, LyricSource::ConfigItemType>::iterator it = item.begin(); it != item.end(); it ++)
				{
					switch(it->second)
					{
					case LyricSource::ITEM_TYPE_STRING:
						{
							HWND hStatic = CreateWindowEx(NULL, L"static", EncodingFunc::ToUTF16(src->GetConfigLabel(it->first)).c_str(), WS_CHILD | WS_VISIBLE, 0, cnt * 30, 100, 25, hWnd, NULL, NULL, NULL);
							HWND hEdit = CreateWindowEx(NULL, L"edit", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 100, cnt * 30, 300, 25, hWnd, (HMENU)(cnt + 1), NULL, NULL);
							SendMessage(hStatic, WM_SETFONT, (WPARAM)font, TRUE);
							SendMessage(hEdit, WM_SETFONT, (WPARAM)font, TRUE);
							SendMessage(hEdit, WM_SETTEXT, NULL, (LPARAM)EncodingFunc::ToUTF16(configitem[it->first]).c_str());
						}
						break;
					case LyricSource::ITEM_TYPE_ENUM:
						{
							HWND hStatic = CreateWindowEx(NULL, L"static", EncodingFunc::ToUTF16(src->GetConfigLabel(it->first)).c_str(), WS_CHILD | WS_VISIBLE, 0, cnt * 30, 100, 25, hWnd, NULL, NULL, NULL);
							HWND hCombo = CreateWindowEx(NULL, L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 100, cnt * 30, 300, 25, hWnd, (HMENU)(cnt + 1), NULL, NULL);
							SendMessage(hStatic, WM_SETFONT, (WPARAM)font, TRUE);
							SendMessage(hCombo, WM_SETFONT, (WPARAM)font, TRUE);
							std::vector<std::string> items = src->GetConfigEnumeration(it->first);
							for(std::vector<std::string>::iterator item = items.begin(); item != items.end(); item ++)
								SendMessage(hCombo, CB_ADDSTRING, NULL, (LPARAM)EncodingFunc::ToUTF16(*item).c_str());
							if(configitem[it->first].length())
								SendMessage(GetDlgItem(hWnd, cnt + 1), CB_SETCURSEL, boost::lexical_cast<int>(configitem[it->first]), NULL);
						}
					}
					cnt ++;
				}
				return TRUE;
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, 1);
			break;
		case WM_COMMAND:
			{
				if(HIWORD(wParam) == BN_CLICKED)
				{
					switch(LOWORD(wParam))
					{
					case IDC_LYRICSOURCECFG_OK:
						{
							std::map<std::string, std::string> configitem = src->GetConfig();
							std::map<std::string, LyricSource::ConfigItemType> item = src->GetConfigItems(type);
							int cnt = 0;
							for(std::map<std::string, LyricSource::ConfigItemType>::iterator it = item.begin(); it != item.end(); it ++)
							{
								switch(it->second)
								{
								case LyricSource::ITEM_TYPE_STRING:
									configitem[it->first] = uGetDlgItemText(hWnd, cnt + 1).get_ptr();
									break;
								case LyricSource::ITEM_TYPE_ENUM:
									configitem[it->first] = boost::lexical_cast<std::string>((int)SendMessage(GetDlgItem(hWnd, cnt + 1), CB_GETCURSEL, NULL, NULL));
									break;
								}
								cnt ++;
							}
							std::string response = src->IsConfigValid(configitem);
							if(response != "")
							{
								MessageBox(hWnd, EncodingFunc::ToUTF16(response).c_str(), TEXT("오류"), MB_OK);
								break;
							}
							src->SetConfig(configitem);
							cfg_lyricsourcecfg.set_value(src->GetGUID(), src->GetConfig());
						}
						EndDialog(hWnd, 0);
						break;
					case IDC_LYRICSOURCECFG_CANCEL:
						EndDialog(hWnd, 1);
						break;
					}
				}
				return TRUE;
			}
			break;
		case WM_DESTROY:
			DeleteObject(font);
			return TRUE;
		}
		return FALSE;
	}

	static void OpenLyricSourceConfig(HWND parent, LyricSource *source, int type)
	{
		DialogBoxParam(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_LYRICSOURCECFG), parent, LyricSourceConfigProc, (LPARAM)&std::make_pair(source, type));
		LyricManagerInstance->UpdateConfig();
	}

	static BOOL CALLBACK CommonConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_INITDIALOG:
			{
				std::vector<GUID> enabledsources = cfg_enabledlyricsource.get_value();
				for(std::vector<GUID>::iterator it = enabledsources.begin(); it != enabledsources.end(); it ++)
				{
					boost::shared_ptr<LyricSource> src = LyricSourceManager::Get(*it);
					int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_ADDSTRING, NULL, (LPARAM)EncodingFunc::ToUTF16(src->GetName()).c_str());
					SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_SETITEMDATA, idx, (LPARAM)src.get());
				}

				std::vector<GUID> enabledsaves = cfg_enabledlyricsave.get_value();
				for(std::vector<GUID>::iterator it = enabledsaves.begin(); it != enabledsaves.end(); it ++)
				{
					boost::shared_ptr<LyricSource> src = LyricSourceManager::Get(*it);
					int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_ADDSTRING, NULL, (LPARAM)EncodingFunc::ToUTF16(src->GetName()).c_str());
					SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_SETITEMDATA, idx, (LPARAM)src.get());
				}

				CheckDlgButton(hWnd, IDC_SKIPEMPTY, cfg_skipempty);
			}
			return TRUE;
		case WM_COMMAND:
			{
				if(HIWORD(wParam) == BN_CLICKED)
				{
					switch(LOWORD(wParam))
					{
					case IDC_LYRICSOURCE_ADD:
						{
							LyricSource *res = (LyricSource *)DialogBox(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_LYRICSOURCE_ADD), hWnd, &preferences_page_instance_alsong_lyric::LyricSourceAddProc);
							if(res)
							{
								if(cfg_enabledlyricsource.exists(res->GetGUID()))
								{
									MessageBox(hWnd, TEXT("이미 있습니다."), TEXT("오류"), MB_OK);
									break;
								}
								cfg_enabledlyricsource.add(res->GetGUID());
								int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_ADDSTRING, NULL, (LPARAM)EncodingFunc::ToUTF16(res->GetName()).c_str());
								SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_SETITEMDATA, idx, (LPARAM)res);
								OpenLyricSourceConfig(hWnd, res, 1);
							}
						}
						break;
					case IDC_LYRICSOURCE_CONFIG:
						{
							LyricSource *res;
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}
							res = (LyricSource *)SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETITEMDATA, idx, NULL);
							OpenLyricSourceConfig(hWnd, res, 1);
							break;
						}
					case IDC_LYRICSOURCE_DELETE:
						{
							LyricSource *res;
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}

							res = (LyricSource *)SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETITEMDATA, idx, NULL);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_DELETESTRING, idx, NULL);
							cfg_enabledlyricsource.remove(res->GetGUID());
							break;
						}
					case IDC_LYRICSOURCE_UP:
						{
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}
							if(idx == 0)								
								break;
							LRESULT data = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETITEMDATA, idx, NULL);
							TCHAR str[255];
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETTEXT, idx, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_DELETESTRING, idx, NULL);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_INSERTSTRING, idx - 1, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_SETITEMDATA, idx - 1, data);
							cfg_enabledlyricsource.remove(idx);
							cfg_enabledlyricsource.insert(idx - 1, ((LyricSource *)data)->GetGUID());
							break;
						}
					case IDC_LYRICSOURCE_DOWN:
						{
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}
							int cnt = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETCOUNT, NULL, NULL);
							if(idx >= cnt - 1)
								break;
							LRESULT data = SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETITEMDATA, idx, NULL);
							TCHAR str[255];
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_GETTEXT, idx, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_DELETESTRING, idx, NULL);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_INSERTSTRING, idx + 1, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSOURCELIST), LB_SETITEMDATA, idx + 1, data);
							cfg_enabledlyricsource.remove(idx);
							cfg_enabledlyricsource.insert(idx + 1, ((LyricSource *)data)->GetGUID());
							break;
						}
					case IDC_LYRICSAVE_ADD:
						{
							LyricSource *res = (LyricSource *)DialogBox(core_api::get_my_instance(), MAKEINTRESOURCE(IDD_LYRICSOURCE_ADD), hWnd, &preferences_page_instance_alsong_lyric::LyricSourceAddProc);
							if(res)
							{
								if(cfg_enabledlyricsource.exists(res->GetGUID()))
								{
									MessageBox(hWnd, TEXT("이미 있습니다."), TEXT("오류"), MB_OK);
									break;
								}
								cfg_enabledlyricsave.add(res->GetGUID());
								int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_ADDSTRING, NULL, (LPARAM)EncodingFunc::ToUTF16(res->GetName()).c_str());
								SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_SETITEMDATA, idx, (LPARAM)res);
								OpenLyricSourceConfig(hWnd, res, 2);
							}
							break;
						}
					case IDC_LYRICSAVE_CONFIG:
						{
							LyricSource *res;
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}
							res = (LyricSource *)SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETITEMDATA, idx, NULL);
							OpenLyricSourceConfig(hWnd, res, 2);
							break;
						}
					case IDC_LYRICSAVE_DELETE:
						{
							LyricSource *res;
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}

							res = (LyricSource *)SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETITEMDATA, idx, NULL);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_DELETESTRING, idx, NULL);
							cfg_enabledlyricsave.remove(res->GetGUID());
							break;
						}
					case IDC_LYRICSAVE_UP:
						{
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}
							if(idx == 0)								
								break;
							LRESULT data = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETITEMDATA, idx, NULL);
							TCHAR str[255];
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETTEXT, idx, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_DELETESTRING, idx, NULL);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_INSERTSTRING, idx - 1, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_SETITEMDATA, idx - 1, data);
							cfg_enabledlyricsave.remove(idx);
							cfg_enabledlyricsave.insert(idx - 1, ((LyricSource *)data)->GetGUID());
							break;
						}
					case IDC_LYRICSAVE_DOWN:
						{
							int idx = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETCURSEL, NULL, NULL);
							if(idx < 0)
							{
								MessageBox(hWnd, TEXT("선택해주세요"), TEXT("오류"), MB_OK);
								break;
							}
							int cnt = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETCOUNT, NULL, NULL);
							if(idx >= cnt - 1)
								break;
							LRESULT data = SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETITEMDATA, idx, NULL);
							TCHAR str[255];
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_GETTEXT, idx, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_DELETESTRING, idx, NULL);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_INSERTSTRING, idx + 1, (LPARAM)str);
							SendMessage(GetDlgItem(hWnd, IDC_LYRICSAVELIST), LB_SETITEMDATA, idx + 1, data);
							cfg_enabledlyricsave.remove(idx);
							cfg_enabledlyricsave.insert(idx + 1, ((LyricSource *)data)->GetGUID());
							break;
						}
					}
					LyricManagerInstance->Reload();
				}
			}
			return TRUE;
		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == PSN_APPLY)
			{
				cfg_skipempty = (IsDlgButtonChecked(hWnd, IDC_SKIPEMPTY) ? true : false);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
			}
			else if(((LPNMHDR)lParam)->code == PSN_KILLACTIVE)
			{
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
			}
			return TRUE;
		}
		return FALSE;
	}
};

// {A58D6A8E-5932-4def-AD63-44185988B105}
static const GUID guid_prefs_alsong_lyric = { 0xa58d6a8e, 0x5932, 0x4def, { 0xad, 0x63, 0x44, 0x18, 0x59, 0x88, 0xb1, 0x5 } };

class preferences_page_alsong_lyric : public preferences_page_v3
{
public:
	virtual const char * get_name()
	{
		return "Alsong Live Lyric";
	}

	virtual GUID get_guid()
	{
		return guid_prefs_alsong_lyric;
	}

	virtual GUID get_parent_guid()
	{
		return preferences_page::guid_tools;
	}

	virtual preferences_page_instance::ptr instantiate(HWND parent, preferences_page_callback::ptr callback)
	{
		return new service_impl_t<preferences_page_instance_alsong_lyric>(parent, callback);
	}
};

static preferences_page_factory_t<preferences_page_alsong_lyric> foo_preferences_page_alsong_lyric;

//TODO: 줄간격 설정
BOOL UIPreference::UIConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam, HWND hParent)
{
	static int old_transparency;
	static bool old_layered;
	static bool old_nolayered;
	static bool old_topmost;
	static bool old_nonlyric;
	static UIPreference newSetting;
	static int shouldClose;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		{
			shouldClose = 0;
			memcpy(&newSetting, this, sizeof(UIPreference));
			old_transparency = cfg_outer_transparency;
			old_layered = cfg_outer_layered;
			old_topmost = cfg_outer_topmost;
			old_nolayered = cfg_outer_nolayered;
			old_nonlyric = cfg_outer_nonlyric;
			SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_SETRANGE32, 1, 20);
			SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, IDC_NLINE), 0);
			SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_SETPOS32, 0, GetnLine());

			SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_SETRANGE32, 0, 200);
			SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, IDC_LINEMARGIN), 0);
			SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_SETPOS32, 0, GetLineMargin());
			SendMessage(GetDlgItem(hWnd, IDC_FONT_TRANSPARENCY), TBM_SETRANGE, TRUE, MAKELONG(0, 100));
			SendMessage(GetDlgItem(hWnd, IDC_FONT_TRANSPARENCY), TBM_SETPOS, TRUE, fontTransparency);

			SendMessage(GetDlgItem(hWnd, IDC_OUTLINEBORDERSPIN), UDM_SETRANGE32, 0, 20);
			SendMessage(GetDlgItem(hWnd, IDC_OUTLINEBORDERSPIN), UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, IDC_OUTLINEBORDER), 0);
			SendMessage(GetDlgItem(hWnd, IDC_OUTLINEBORDERSPIN), UDM_SETPOS32, 0, outLineSize);
			SendMessage(GetDlgItem(hWnd, IDC_NONLYRICTEXT),WM_SETTEXT,NULL,(LPARAM)newSetting.GetNonLyricText());
//			if(Setting->Script)
//				uSetDlgItemText(hWnd, IDC_UISCRIPT, Setting->Script->get_ptr());
			if(GetParent(hParent) == NULL)
			{
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_SETRANGE, TRUE, MAKELONG(0, 100));
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_SETPOS, TRUE, cfg_outer_transparency);

				int pos;
				pos = SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
				TCHAR temp[255];
				wsprintf(temp, TEXT("%d%%"), pos);
				SetWindowText(GetDlgItem(hWnd, IDC_TRANSPARENCY_LABEL), temp);

				pos = SendMessage(GetDlgItem(hWnd, IDC_FONT_TRANSPARENCY), TBM_GETPOS, 0, 0);
				wsprintf(temp, TEXT("%d%%"), pos);
				SetWindowText(GetDlgItem(hWnd, IDC_FONT_TRANSPARENCY_LABEL), temp);

				CheckDlgButton(hWnd, IDC_LAYERED, cfg_outer_layered);
				CheckDlgButton(hWnd, IDC_NOLAYERED, cfg_outer_nolayered);
				CheckDlgButton(hWnd, IDC_TOPMOST, cfg_outer_topmost);
				CheckDlgButton(hWnd, IDC_OUTER_BLUR, cfg_outer_blur);
				CheckDlgButton(hWnd, IDC_NONLYRICTEXTCHK, cfg_outer_nonlyric);
				HMODULE dwm = LoadLibrary(TEXT("dwmapi.dll"));
				if(dwm)
				{
					typedef HRESULT __stdcall DwmIsCompositionEnabled( __out BOOL* pfEnabled );
					DwmIsCompositionEnabled *dice = (DwmIsCompositionEnabled*)GetProcAddress(dwm, "DwmIsCompositionEnabled");
					if(dice)
					{
						BOOL b;
						dice(&b);
						if(b == FALSE)
							SendMessage(GetDlgItem(hWnd, IDC_OUTER_BLUR), WM_CLOSE, 0, 0);
					}
					FreeLibrary(dwm);
				}
			}
			else
			{
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY_STATIC), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY_LABEL), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_LAYERED), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_NOLAYERED), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_TOPMOST), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_OUTER_BLUR), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_NONLYRICTEXTCHK), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_RESETWNDPOS), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCYNOTICE), WM_CLOSE, 0, 0);

			}

			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("위"));
			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("가운데"));
			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("아래"));
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("왼쪽"));
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("가운데"));
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("오른쪽"));

			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_SETCURSEL, GetVerticalAlign() - 1, NULL);
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_SETCURSEL, GetHorizentalAlign() - 1, NULL);

			SendMessage(GetDlgItem(hWnd, IDC_MANUALSCRIPT), WM_CLOSE, 0, 0);
			SendMessage(GetDlgItem(hWnd, IDC_UISCRIPT), WM_CLOSE, 0, 0);

			SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_ADDSTRING, NULL, (LPARAM)TEXT("색"));
			SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_ADDSTRING, NULL, (LPARAM)TEXT("그림"));
			SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_ADDSTRING, NULL, (LPARAM)TEXT("없음"));

			SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_SETCURSEL, GetBgType(), NULL);
		}

		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rt;
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			SetRect(&rt, 0, 200, clientRect.right, clientRect.bottom);
			{
				UICanvas canvas(hWnd, hdc, rt);
				canvas.Fill(0, 0, rt.right, rt.bottom, RGB(255, 255, 255));
				BgType newBgType = static_cast<BgType>(SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_GETCURSEL, NULL, NULL));
				if(newBgType == BG_SOLIDCOLOR)
					canvas.Fill(0, 0, rt.right, rt.bottom, newSetting.backColor);
				else if(newBgType == BG_IMAGE)
					canvas.DrawImage(0, 0, rt.right, rt.bottom, newSetting.bgImage);
				else
					canvas.Fill(0, 0, rt.right, rt.bottom, RGB(255, 255, 255));
				canvas.DrawText(UIFont(newSetting.normalFont), L"\1일반 글꼴 미리보기 1234abc", 2, 1.f, 100, newSetting.outLineColor, SendMessage(GetDlgItem(hWnd, IDC_OUTLINEBORDERSPIN), UDM_GETPOS32, NULL, NULL));
				canvas.DrawText(UIFont(newSetting.highlightFont), L"\1현재가사 글꼴 미리보기 1234abc", 2, 1.f, 100, newSetting.outLineColor, SendMessage(GetDlgItem(hWnd, IDC_OUTLINEBORDERSPIN), UDM_GETPOS32, NULL, NULL));
			}
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_HSCROLL:
		if(GetParent(hParent) == NULL)
		{
			int pos;
			pos = SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
			TCHAR temp[255];
			wsprintf(temp, TEXT("%d%%"), pos);
			SetWindowText(GetDlgItem(hWnd, IDC_TRANSPARENCY_LABEL), temp);

			pos = SendMessage(GetDlgItem(hWnd, IDC_FONT_TRANSPARENCY), TBM_GETPOS, 0, 0);
			wsprintf(temp, TEXT("%d%%"), pos);
			SetWindowText(GetDlgItem(hWnd, IDC_FONT_TRANSPARENCY_LABEL), temp);
			SendMessage(GetParent(hWnd), PSM_CHANGED, (WPARAM)hWnd, 0);

			RECT rt;
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			SetRect(&rt, 0, 200, clientRect.right, clientRect.bottom);
			InvalidateRect(hWnd, &rt, TRUE);
		}
		break;
	case WM_COMMAND:
		if(HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == CBN_SELCHANGE)
			SendMessage(GetParent(hWnd), PSM_CHANGED, (WPARAM)hWnd, 0);
			
		if(HIWORD(wParam) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case IDC_BKCOLOR:
				newSetting.backColor = OpenBkColorPopup(hWnd);
				break;
			case IDC_BGIMAGE:
				lstrcpy(newSetting.bgImage, OpenBgImagePopup(hWnd).c_str());
				break;
			case IDC_NORMALFONT_CHANGE:
				newSetting.normalFont = OpenFontPopup(hWnd, newSetting.normalFont);
				break;
			case IDC_HIGHLIGHTFONT_CHANGE:
				newSetting.highlightFont = OpenFontPopup(hWnd, newSetting.highlightFont);
				break;
			case IDC_OUTLINECOLOR_CHANGE:
				newSetting.outLineColor = OpenColorPopup(hWnd, newSetting.outLineColor);
				break;
			case IDC_HIGHLIGHTFONTCOLOR_CHANGE:
				newSetting.highlightFont.color = OpenColorPopup(hWnd, newSetting.highlightFont.color);
				break;
			case IDC_FONTCOLOR_CHANGE:
				newSetting.normalFont.color = OpenColorPopup(hWnd, newSetting.normalFont.color);
				break;
			}
		}
		RECT rt;
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		SetRect(&rt, 0, 200, clientRect.right, clientRect.bottom);
		InvalidateRect(hWnd, &rt, TRUE);
		break;
	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			memcpy(this, &newSetting, sizeof(UIPreference));
			nLine = SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_GETPOS32, NULL, NULL);
			LineMargin = SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_GETPOS32, NULL, NULL);
			outLineSize = SendMessage(GetDlgItem(hWnd, IDC_OUTLINEBORDERSPIN), UDM_GETPOS32, NULL, NULL);
			fontTransparency = SendMessage(GetDlgItem(hWnd, IDC_FONT_TRANSPARENCY), TBM_GETPOS, 0, 0);
			if(GetParent(hParent) == NULL)
			{
				cfg_outer_transparency = SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
				cfg_outer_layered = (IsDlgButtonChecked(hWnd, IDC_LAYERED) ? true : false);
				cfg_outer_nolayered = (IsDlgButtonChecked(hWnd, IDC_NOLAYERED) ? true : false);
				cfg_outer_topmost = (IsDlgButtonChecked(hWnd, IDC_TOPMOST) ? true : false);
				cfg_outer_blur = (IsDlgButtonChecked(hWnd, IDC_OUTER_BLUR) ? true : false);
				cfg_outer_nonlyric = (IsDlgButtonChecked(hWnd, IDC_NONLYRICTEXTCHK) ? true : false);

				WndInstance.StyleUpdated();
			}

			VerticalAlign = static_cast<AlignPosition>(SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_GETCURSEL, NULL, NULL) + 1);
			HorizentalAlign = static_cast<AlignPosition>(SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_GETCURSEL, NULL, NULL) + 1);
			bgType = static_cast<BgType>(SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_GETCURSEL, NULL, NULL));
			GetWindowText(GetDlgItem(hWnd,IDC_NONLYRICTEXT),newSetting.nonlyrictext,128);
//			if(Setting->Script)
//				uGetDlgItemText(hWnd, IDC_UISCRIPT, *(Setting->Script));


			RECT rt;
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			SetRect(&rt, 0, 200, clientRect.right, clientRect.bottom);
			InvalidateRect(hWnd, &rt, TRUE);

			SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
			if(((LPPSHNOTIFY)lParam)->lParam)
				shouldClose = 1;
		}
		else if(((LPNMHDR)lParam)->code == PSN_KILLACTIVE)
		{
			SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
			//DestroyWindow(GetParent(hWnd));
		}
		else if(((LPNMHDR)lParam)->code == PSN_RESET)
		{
			if(GetParent(hParent) == NULL)
			{
				cfg_outer_transparency = old_transparency;
				cfg_outer_layered = old_layered;
				cfg_outer_nolayered = old_nolayered;
				cfg_outer_topmost = old_topmost;
				cfg_outer_nonlyric = old_nonlyric;
				WndInstance.StyleUpdated();
			}
			//DestroyWindow(((LPNMHDR)lParam)->hwndFrom);
			PostMessage(hWnd, WM_USER + 1, (WPARAM)((LPNMHDR)lParam)->hwndFrom, 0);
		}
		else if(((LPNMHDR)lParam)->code == -211)
		{
			if(shouldClose)
				//DestroyWindow(((LPNMHDR)lParam)->hwndFrom);
				PostMessage(hWnd, WM_USER + 1, (WPARAM)((LPNMHDR)lParam)->hwndFrom, 0);
		}
		break;
	case WM_USER + 1:
		DestroyWindow((HWND)wParam);
	default:
		return FALSE;
	}
	return TRUE;
}

static HWND g_ConfigParent = NULL; //TODO

BOOL CALLBACK UIPreference::ConfigProcDispatcher(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static UIPreference *_this = NULL;
	if(iMessage == WM_INITDIALOG)
		_this = (UIPreference *)((PROPSHEETPAGE *)lParam)->lParam;

	if(iMessage == WM_DESTROY)
	{
		_this = NULL;
		g_ConfigParent = NULL;
	}

	if(_this)
		return _this->UIConfigProc(hWnd, iMessage, wParam, lParam, g_ConfigParent);
	else
		return FALSE;
}

void UIPreference::OpenConfigPopup(HWND hParent)
{
	if(g_ConfigParent)
	{
		MessageBox(hParent, TEXT("Cannot open more than two config popup"), TEXT("error"), MB_OK);
		return;
	}
	g_ConfigParent = hParent;

	HPROPSHEETPAGE pages[1];
	PROPSHEETPAGE page;
	page.dwSize = sizeof(PROPSHEETPAGE);
	page.dwFlags = PSP_DEFAULT;
	page.hInstance = core_api::get_my_instance();
	page.pszTemplate = MAKEINTRESOURCE(IDD_UI_PREF_COMMON);
	page.pfnDlgProc = &UIPreference::ConfigProcDispatcher;
	page.lParam = (LPARAM)this;
	pages[0] = CreatePropertySheetPage(&page);

	PROPSHEETHEADER psh;
	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_DEFAULT | PSH_NOCONTEXTHELP | PSH_USEHICON | PSH_MODELESS;
	psh.hwndParent = hParent;
	psh.hInstance = core_api::get_my_instance();
	psh.hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
	psh.pszCaption = TEXT("고급 설정");
	psh.nPages = 1;
	psh.nStartPage = 0;
	psh.phpage = pages;

	int nRet = PropertySheet(&psh);
}
