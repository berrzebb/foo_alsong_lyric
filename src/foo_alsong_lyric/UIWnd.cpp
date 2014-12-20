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
#include "UIManager.h"
#include "UIWnd.h"
#include "UIPreference.h"
#include <propkey.h>
#include <propvarutil.h>

//TODO:UpdateLayeredWindow() 사용. http://msdn.microsoft.com/en-us/library/bb773289(VS.85).aspx 로 직접 창틀 그리기

UIWnd WndInstance; //singleton

//SHGetPropertyStoreForWindow prototype
typedef DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *SHGPSFW)(HWND hwnd,
														  REFIID riid,
														  void** ppv);


/* interface ICustomDestinationList */
/* [unique][object][uuid] */ 

typedef /* [v1_enum] */ 
enum KNOWNDESTCATEGORY
{	KDC_FREQUENT	= 1,
KDC_RECENT	= ( KDC_FREQUENT + 1 ) 
} 	KNOWNDESTCATEGORY;

//icustomdestinationlist prototype
EXTERN_C const IID IID_ICustomDestinationList;
MIDL_INTERFACE("6332debf-87b5-4670-90c0-5e57b408a49e")
ICustomDestinationList : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetAppID( 
		/* [string][in] */ __RPC__in_string LPCWSTR pszAppID) = 0;

	virtual HRESULT STDMETHODCALLTYPE BeginList( 
		/* [out] */ __RPC__out UINT *pcMinSlots,
		/* [in] */ __RPC__in REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out_opt void **ppv) = 0;

	virtual HRESULT STDMETHODCALLTYPE AppendCategory( 
		/* [string][in] */ __RPC__in_string LPCWSTR pszCategory,
		/* [in] */ __RPC__in_opt IObjectArray *poa) = 0;

	virtual HRESULT STDMETHODCALLTYPE AppendKnownCategory( 
		/* [in] */ KNOWNDESTCATEGORY category) = 0;

	virtual HRESULT STDMETHODCALLTYPE AddUserTasks( 
		/* [in] */ __RPC__in_opt IObjectArray *poa) = 0;

	virtual HRESULT STDMETHODCALLTYPE CommitList( void) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetRemovedDestinations( 
		/* [in] */ __RPC__in REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out_opt void **ppv) = 0;

	virtual HRESULT STDMETHODCALLTYPE DeleteList( 
		/* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszAppID) = 0;

	virtual HRESULT STDMETHODCALLTYPE AbortList( void) = 0;

};

//dwm api

// Blur behind data structures
#define DWM_BB_ENABLE                 0x00000001  // fEnable has been specified
#define DWM_BB_BLURREGION             0x00000002  // hRgnBlur has been specified
#define DWM_BB_TRANSITIONONMAXIMIZED  0x00000004  // fTransitionOnMaximized has been specified

#pragma pack(push, 1)
typedef struct _DWM_BLURBEHIND
{
	DWORD dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND, *PDWM_BLURBEHIND;
#pragma pack(pop)
typedef HRESULT __stdcall DwmEnableBlurBehindWindow( HWND hWnd, const DWM_BLURBEHIND* pBlurBehind );
typedef HRESULT __stdcall DwmIsCompositionEnabled( __out BOOL* pfEnabled );

const int UIWnd::Resize_border = 6;

UIWnd::UIWnd() : m_isResizing(false)
{

}

HWND UIWnd::Create() 
{
	assert(m_hWnd == NULL);
	
	cfg_outer.get_value().Ready();
	m_UI = new UIManager(&cfg_outer.get_value(), &cfg_outer_script);

	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WindowProc;
	wcex.hInstance = NULL;
	wcex.hIconSm = NULL;
	wcex.hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszClassName = TEXT("UILyricWindow");
	wcex.cbWndExtra = sizeof(UIWnd *);
	RegisterClassEx(&wcex);
	
	m_hWnd = CreateWindowEx(
		(cfg_outer_topmost ? WS_EX_TOPMOST : 0) | 
		(cfg_outer_layered ? WS_EX_TRANSPARENT : 0) | 
		(cfg_outer_nolayered ? 0 : WS_EX_LAYERED) | 
		(cfg_outer_taskbar ? WS_EX_TOOLWINDOW : 0),
		TEXT("UILyricWindow"),
		TEXT("가사 창"),
		WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 200,
		NULL,
		0,
		NULL,
		this );
	//windows 7 taskbar

	HMODULE shell32 = LoadLibrary(L"shell32.dll");
	SHGPSFW SHGetPropertyStoreForWindow = reinterpret_cast<SHGPSFW>(GetProcAddress(shell32, "SHGetPropertyStoreForWindow"));
	m_Propstore = NULL;
	if(SHGetPropertyStoreForWindow && cfg_use_jumplist)
	{
		SHGetPropertyStoreForWindow(m_hWnd, IID_IPropertyStore, (void **)&m_Propstore);
		wchar_t appid[] = L"dlunch.foo_alsong_lyric";
		if(m_Propstore)
		{
			PROPVARIANT propvar;
			InitPropVariantFromString(appid, &propvar);
			m_Propstore->SetValue(PKEY_AppUserModel_ID, propvar); //use separate window
			InitPropVariantFromBoolean(true, &propvar);
			m_Propstore->SetValue(PKEY_AppUserModel_PreventPinning, propvar); //not to pin
		}
		std::vector<std::pair<std::wstring,std::wstring>> Tasks;
		Tasks.emplace_back(std::make_pair<std::wstring,std::wstring>(std::wstring(L"알송 실시간 가사"),std::wstring(L"알송 실시간 가사 창")));		
		Tasks.emplace_back(std::make_pair<std::wstring,std::wstring>(std::wstring(L"Alsong Lyric Window Config"),std::wstring(L"알송 실시간 가사 창 설정")));
		Tasks.emplace_back(std::make_pair<std::wstring,std::wstring>(std::wstring(L"알송 가사 추가/변경"),std::wstring(L"가사 검색")));
		AddTaskList(Tasks,L"");
		AddTaskList(Tasks,appid);
	}
	m_isBlur = false;
	if(cfg_outer_blur)
	{
		DWM_BLURBEHIND bb = {0};
		bb.dwFlags = DWM_BB_ENABLE;
		bb.fEnable = true;
		bb.hRgnBlur = NULL;

		HMODULE dwm = LoadLibrary(TEXT("dwmapi.dll"));
		if(dwm)
		{
			DwmEnableBlurBehindWindow* debbw = (DwmEnableBlurBehindWindow*)GetProcAddress(dwm, "DwmEnableBlurBehindWindow");
			DwmIsCompositionEnabled *dice = (DwmIsCompositionEnabled*)GetProcAddress(dwm, "DwmIsCompositionEnabled");
			if(debbw && dice)
			{
				BOOL b;
				dice(&b);
				if(b == TRUE)
				{
					debbw(m_hWnd, &bb);
					m_isBlur = true;
				}
			}
			FreeLibrary(dwm);
		}
	}
	ShowWindow(m_hWnd, SW_HIDE);
	HMENU hMenu = GetSystemMenu(m_hWnd, FALSE);
	AppendMenu(hMenu, MF_STRING, 1000, TEXT("고급 설정..."));
	
	return m_hWnd;
}
int UIWnd::AddTaskList(std::vector<std::pair<std::wstring,std::wstring>> data,std::wstring appid)
{
	ICustomDestinationList *destlist = NULL;
	CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_ICustomDestinationList, (void **)&destlist);
	if(destlist) //main window list
	{
		UINT MinSlot;
		IObjectArray *removed;
		if(appid.size())
			destlist->SetAppID(appid.c_str());
		destlist->BeginList(&MinSlot, IID_IObjectArray, (void **)&removed);
		IObjectCollection *tasks;
		CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER, IID_IObjectCollection, (void **)&tasks);
		size_t sz = data.size();
		std::vector<IShellLink*> linklist;
		for(int i = 0; i< sz; ++i)
		{
			IShellLink* link;
			CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&link);
			wchar_t name[255];
			GetModuleFileName(GetModuleHandle(L"foobar2000.exe"), name, 255);
			link->SetPath(name);
			link->SetArguments((std::wstring(L"/command:\"") + data[i].first + L"\"").c_str());

			IPropertyStore *propstore;
			link->QueryInterface(IID_IPropertyStore, (void **)&propstore);
			PROPVARIANT pv;
			InitPropVariantFromString(data[i].second.c_str(), &pv);
			propstore->SetValue(PKEY_Title, pv);
			propstore->Commit();
			propstore->Release();
			tasks->AddObject(link);
			linklist.emplace_back(link);
		}
		IObjectArray *arr;
		tasks->QueryInterface(IID_IObjectArray, (void **)&arr);
		destlist->AddUserTasks(arr);
		destlist->CommitList();

		destlist->Release();
		tasks->Release();
		arr->Release();
		size_t linksz = linklist.size();
		for(int i=0; i < linksz; i++)
		{
			linklist[i]->Release();
		}
		linklist.clear();
		removed->Release();

		return 1;
	}

	return 0;
}
int UIWnd::AddTaskList(std::wstring command, std::wstring display, std::wstring appid)
{
	ICustomDestinationList *destlist = NULL;
	CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_ICustomDestinationList, (void **)&destlist);
	if(destlist) //main window list
	{
		UINT MinSlot;
		IObjectArray *removed;
		if(appid.size())
			destlist->SetAppID(appid.c_str());
		destlist->BeginList(&MinSlot, IID_IObjectArray, (void **)&removed);
		IObjectCollection *tasks;
		CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER, IID_IObjectCollection, (void **)&tasks);

		IShellLink *link;
		CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&link);
		wchar_t name[255];
		GetModuleFileName(GetModuleHandle(L"foobar2000.exe"), name, 255);
		link->SetPath(name);
		link->SetArguments((std::wstring(L"/command:\"") + command + L"\"").c_str());

		IPropertyStore *propstore;
		link->QueryInterface(IID_IPropertyStore, (void **)&propstore);
		PROPVARIANT pv;
		InitPropVariantFromString(display.c_str(), &pv);
		propstore->SetValue(PKEY_Title, pv);
		propstore->Commit();
		propstore->Release();
		tasks->AddObject(link);
		IObjectArray *arr;
		tasks->QueryInterface(IID_IObjectArray, (void **)&arr);
		destlist->AddUserTasks(arr);
		destlist->CommitList();

		destlist->Release();
		tasks->Release();
		arr->Release();
		link->Release();
		removed->Release();
		
		return 1;
	}

	return 0;
}

void UIWnd::Destroy() 
{
	// Destroy the window.
	if(m_hWnd)
	{
		if(m_Propstore)
		{
			PROPVARIANT propvar;
			propvar.vt = VT_EMPTY;
			m_Propstore->SetValue(PKEY_AppUserModel_ID, propvar);
			m_Propstore->SetValue(PKEY_AppUserModel_PreventPinning, propvar); //reset
			m_Propstore->Release();
		}
		DestroyWindow(m_hWnd);
		delete m_UI;
	}
}

void UIWnd::Show()
{
	ShowWindow(m_hWnd, SW_SHOW);
	cfg_outer_shown = true;
	m_UI->Invalidated(m_hWnd);
}

void UIWnd::Hide()
{
	ShowWindow(m_hWnd, SW_HIDE);
	cfg_outer_shown = false;
}

void UIWnd::StyleUpdated()
{
	SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME);

	SetWindowLong(m_hWnd, GWL_EXSTYLE, (cfg_outer_topmost ? WS_EX_TOPMOST : 0) | (cfg_outer_layered ? WS_EX_TRANSPARENT : 0) | (cfg_outer_nolayered ? 0 : WS_EX_LAYERED) | (cfg_outer_taskbar ? WS_EX_TOOLWINDOW : 0));
	if(cfg_outer_shown)
		SetWindowPos(m_hWnd, (cfg_outer_topmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_FRAMECHANGED);
	else
		SetWindowPos(m_hWnd, (cfg_outer_topmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

	DWM_BLURBEHIND bb = {0};
	bb.dwFlags = DWM_BB_ENABLE;
	if(cfg_outer_blur)
		bb.fEnable = true;
	else if(!cfg_outer_blur)
		bb.fEnable = false;
	bb.hRgnBlur = NULL;

	HMODULE dwm = LoadLibrary(TEXT("dwmapi.dll"));
	if(dwm)
	{
		DwmEnableBlurBehindWindow* debbw = (DwmEnableBlurBehindWindow*)GetProcAddress(dwm, "DwmEnableBlurBehindWindow");
		DwmIsCompositionEnabled *dice = (DwmIsCompositionEnabled*)GetProcAddress(dwm, "DwmIsCompositionEnabled");
		if(debbw && dice)
		{
			BOOL b;
			dice(&b);
			if(b == TRUE)
			{
				debbw(m_hWnd, &bb);
				m_isBlur = true;
			}
		}
		FreeLibrary(dwm);
	}
	m_UI->Invalidated(m_hWnd);
}

LRESULT CALLBACK UIWnd::WindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if(iMessage == WM_NCCREATE)
		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)((CREATESTRUCT *)lParam)->lpCreateParams);
	UIWnd *_this = (UIWnd *)GetWindowLongPtr(hWnd, GWL_USERDATA);
	switch(iMessage)
	{
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		cfg_outer_shown = false;
		return 0;
	case WM_SYSCOMMAND:
		if(wParam == 1000)
			_this->m_UI->ShowConfig(hWnd);

		break;
	case WM_SHOWWINDOW:
	case WM_SIZE:
		_this->m_UI->Invalidated(hWnd);
		break;
	case WM_NCHITTEST:
		{
			LRESULT ret = DefWindowProc(hWnd, iMessage, wParam, lParam);
			if(ret == HTCLIENT)
			{
				POINT pt;
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);
				ScreenToClient(hWnd, &pt);

				int border = 3;
				if(_this->isResizing())
					border = Resize_border;
				RECT rt;
				GetClientRect(hWnd, &rt);

				if(pt.x < border && pt.y < border)
					return HTTOPLEFT;
				if(pt.x < border && pt.y > rt.bottom - border - 1)
					return HTBOTTOMLEFT;
				if(pt.y < border - 1 && pt.x > rt.right - border - 1)
					return HTTOPRIGHT;
				if(pt.y > rt.bottom - border - 1 && pt.x > rt.right - border - 1)
					return HTBOTTOMRIGHT;
				if(pt.x < border)
					return HTLEFT;
				if(pt.y < border)
					return HTTOP;
				if(pt.x > rt.right - border - 1)
					return HTRIGHT;
				if(pt.y > rt.bottom - border - 1)
					return HTBOTTOM;

				return HTCAPTION;
			}
			return ret;
		}
	case WM_DESTROY:
		if(!IsIconic(hWnd))
			cfg_outer_window_placement.on_window_destruction(hWnd);
		break;
	case WM_CREATE:
		cfg_outer_window_placement.on_window_creation(hWnd);
		_this->m_hWnd = hWnd;
		break;
	}
	if(_this)
		return _this->m_UI->ProcessMessage(hWnd, iMessage, wParam, lParam);
	else
		return DefWindowProc(hWnd, iMessage, wParam, lParam);
}