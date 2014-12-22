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
#include "UIWnd.h"
#include "ConfigStore.h"
#include "UIManager.h"
#include "LyricManager.h"

DECLARE_COMPONENT_VERSION(
"Alsong lyric plugin(MOD)",
"0.2.0",
"Alsong lyric plugin for foobar2000\n"
"Developed by dlunch(dlunch@gmail.com)\n"
"Modder by berrzebb(ph4913@gmail.com)\n"
"Repository by http://github.com/berrzebb/foo_alsong_lyric \n"
"compiled: " __DATE__ "\n"
"with Panel API version: " UI_EXTENSION_VERSION)

class initquit_plugin : public initquit 
{
private:
	ULONG_PTR gdiplus_token;
public:
	virtual void on_init() 
	{
		WSADATA wd;
		WSAStartup(MAKEWORD(2, 2), &wd);

		TIMECAPS ptc;
		timeGetDevCaps(&ptc, sizeof(TIMECAPS));
		timeBeginPeriod(ptc.wPeriodMin);

		GdiplusStartupInput gsi;
		gsi.GdiplusVersion = 1;
		gsi.DebugEventCallback = NULL;
		gsi.SuppressBackgroundThread = FALSE;
		gsi.SuppressExternalCodecs = TRUE;

		GdiplusStartup(&gdiplus_token, &gsi, NULL);
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(icex);
		icex.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
		InitCommonControlsEx(&icex);

		if(!LyricManagerInstance)
			LyricManagerInstance = new LyricManager();

		WndInstance.Create();
		if (cfg_outer_shown)
			WndInstance.Show();
	}

	virtual void on_quit() 
	{
		WndInstance.Destroy();
		TIMECAPS ptc;
		timeGetDevCaps(&ptc, sizeof(TIMECAPS));
		timeEndPeriod(ptc.wPeriodMin);
		WSACleanup();
		GdiplusShutdown(gdiplus_token);

//		SquirrelVM::AppFinalShutdown();
		if(LyricManagerInstance)
			delete LyricManagerInstance;
	}
};

static initquit_factory_t<initquit_plugin> plugin_initquit;
