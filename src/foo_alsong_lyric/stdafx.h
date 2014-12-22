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

#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT 0x0501

#pragma warning(disable:4180) //Visual C++ bug

#include <winsock2.h>
#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <IPHlpApi.h>

#include <uxtheme.h>
#include <vssym32.h>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <exception>
#include <typeinfo>
#define BOOST_BIND_ENABLE_STDCALL
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include "pugixml/pugixml.hpp"
#include "API/AlsongInterface.h"
#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

#include "../../sdk/foobar2000/SDK/foobar2000.h"
#include "../../sdk/foobar2000/helpers/helpers.h"
#include "../../sdk/foobar2000/columns_ui-sdk/ui_extension.h"
#include "../../3rdparty/Squirrel/sqplus/sqplus.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "../../sdk/foobar2000/shared/shared.lib")

using namespace Gdiplus;
using namespace std;

inline bool operator<( const GUID & lhs, const GUID & rhs ) //use guid as a key in map
{
	return ( memcmp( &lhs, &rhs, sizeof(GUID) ) > 0 ? true : false );
}

template <unsigned N, class T>
int arraysizeof(T (& arr)[N])
{
	return N;
}