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

#include "LyricSourceLRC.h"
#include "EncodingFunc.h"
#include "LRCLyric.h"

std::wstring LyricSourceLRC::getSavePath(const metadb_handle_ptr &track)
{
	std::string path = track->get_path();
	std::wstring wpath = EncodingFunc::ToUTF16(path.substr(boost::find_last(path, "://").end() - path.begin()));
	wpath = wpath.substr(0, wpath.find_last_of(L"."));
	if(track->get_subsong_index() != 0)
		wpath += boost::lexical_cast<std::wstring>(track->get_subsong_index());
	wpath += L".lrc";
	if(m_config["lrcsavepath"].length())
	{
		class LRCTitleFormatCallback : public main_thread_callback
		{
		private:
			HANDLE m_event;
			std::string *m_out;
			const std::string &m_format;
			const metadb_handle_ptr &m_track;
		public:
			LRCTitleFormatCallback(HANDLE event, std::string *out, const std::string &format, const metadb_handle_ptr &track) : m_event(event), m_format(format), m_out(out), m_track(track) {}

			virtual void callback_run()
			{
				core_api::ensure_main_thread();
				pfc::string8 out;
				service_ptr_t<titleformat_object> script;
				static_api_ptr_t<titleformat_compiler>()->compile(script, m_format.c_str());
				m_track->format_title(NULL, out, script, NULL);
				m_out->assign(out.get_ptr());
				SetEvent(m_event);
			}
		};
		std::wstring dirname = wpath.substr(0, wpath.find_last_of(L'\\') + 1);
		std::wstring filename = wpath.substr(wpath.find_last_of(L"\\") + 1);
		HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);
		std::string out;
		std::string pathtmp = m_config["lrcsavepath"];
		if(pathtmp.at(pathtmp.length() - 1) != '\\')
			pathtmp += '\\';
		service_ptr_t<LRCTitleFormatCallback> p_callback = new service_impl_t<LRCTitleFormatCallback>(event, &out, pathtmp, track);
		static_api_ptr_t<main_thread_callback_manager>()->add_callback(p_callback);
		p_callback.release();
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
		if(PathIsRelative(wpath.c_str()))
			wpath = dirname + wpath;
		if(GetFileAttributes(wpath.c_str()) & FILE_ATTRIBUTE_DIRECTORY && (GetLastError() != ERROR_FILE_NOT_FOUND && GetLastError() != ERROR_PATH_NOT_FOUND))
			wpath += filename;
		return wpath;
	}

	if(path.find_first_of("file://") == std::string::npos)
		return std::wstring(L"");
	if(path.find_first_of("unpack://") == std::string::npos)
		return std::wstring(L"");
	return wpath;
}

boost::shared_ptr<Lyric> LyricSourceLRC::Get(const metadb_handle_ptr &track)
{
	std::wstring path = getSavePath(track);
	std::string lyric;

	HANDLE hf = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if(hf == INVALID_HANDLE_VALUE)
		return boost::shared_ptr<Lyric>(new LRCLyric(lyric, path));
	DWORD size = SetFilePointer(hf, 0, 0, FILE_END);
	SetFilePointer(hf, 0, 0, FILE_BEGIN);
	char *data = new char[size];
	DWORD dwRead;
	ReadFile(hf, (void *)data, size, &dwRead, NULL);

	CloseHandle(hf);

	if(data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) //utf8 bom
	{
		lyric.assign(data + 3);
		delete [] data;
	}
	else if(data[0] == 0xFF && data[1] == 0xFE) //utf16 bom
	{
		lyric = EncodingFunc::ToUTF8((wchar_t *)data);
		delete [] data;
	}
	else
	{
		lyric.assign(data);
		delete [] data;
	}


	return boost::shared_ptr<Lyric>(new LRCLyric(lyric, path));
}

DWORD LyricSourceLRC::Save(const metadb_handle_ptr &track, Lyric &lyric)
{
	std::wstring path = getSavePath(track);

	HANDLE hf = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
	if(hf == INVALID_HANDLE_VALUE)
		return 0;

	DWORD unused;
	//{"UTF-8", "UTF-8(BOM)", "UTF-16", "CP949"};
	if(m_config["lrcsaveencoding"] == "1")
	{
		const unsigned char bomtemp[] = {0xEF, 0xBB, 0xBF};
		WriteFile(hf, bomtemp, 3, &unused, NULL);
	}
	else if(m_config["lrcsaveencoding"] == "2")
	{
		const unsigned char bomtemp[] = {0xFF, 0xFE};
		WriteFile(hf, bomtemp, 2, &unused, NULL);
	}

	for(std::vector<LyricLine>::const_iterator it = lyric.GetIteratorAt(0); !lyric.IsEndOfLyric(it); it ++)
	{
		std::stringstream str;
		str << "[" << std::setfill('0') << std::setw(2) << it->time / 60 / 100 << ":" << std::setw(2) << it->time / 100 % 60 << "." << std::setw(2) << it->time % 100 << "]" << it->lyric << "\r\n";
		if(m_config["lrcsaveencoding"] == "0" || m_config["lrcsaveencoding"] == "1")
			WriteFile(hf, str.str().c_str(), str.str().size(), &unused, NULL);
		else if(m_config["lrcsaveencoding"] == "2")
		{
			std::wstring wstr = EncodingFunc::ToUTF16(str.str());
			WriteFile(hf, wstr.c_str(), wstr.size(), &unused, NULL);
		}
		else if(m_config["lrcsaveencoding"] == "3")
		{
			std::wstring wstr = EncodingFunc::ToUTF16(str.str());
			CHAR mbstr[512];
			int len = WideCharToMultiByte(949, NULL, wstr.c_str(), -1, mbstr, 512, NULL, NULL);
			mbstr[len] = 0;
			WriteFile(hf, mbstr, len - 1, &unused, NULL);
		}
	}

	CloseHandle(hf);

	return 1;
}

std::map<std::string, LyricSource::ConfigItemType> LyricSourceLRC::GetConfigItems(int type)
{
	static std::map<std::string, LyricSource::ConfigItemType> ret;
	if(ret.size() == 0)
	{
		ret["lrcsavepath"] = LyricSource::ITEM_TYPE_STRING;
		ret["lrcsaveencoding"] = LyricSource::ITEM_TYPE_ENUM;
	}

	return ret;
}

std::string LyricSourceLRC::GetConfigDescription(std::string item)
{
	static std::map<std::string, std::string> data;
	if(data.size() == 0)
	{
		data["lrcsavepath"] = EncodingFunc::ToUTF8(L"아무것도 적지 않으면 mp3경로에 저장되며, titleformat 사용 가능합니다.\nEx)C:\\Lyric\\%ARTIST%\\%TITLE%.lrc");
		data["lrcsaveencoding"] = EncodingFunc::ToUTF8(L"");
	}
	return data[item];
}

std::string LyricSourceLRC::GetConfigLabel(std::string item)
{
	static std::map<std::string, std::string> data;
	if(data.size() == 0)
	{
		data["lrcsavepath"] = EncodingFunc::ToUTF8(L"LRC 저장 위치");
		data["lrcsaveencoding"] = EncodingFunc::ToUTF8(L"LRC 파일 인코딩");

	}
	return data[item];
}

std::vector<std::string> LyricSourceLRC::GetConfigEnumeration(std::string item)
{
	static std::map<std::string, std::vector<std::string> > data;
	if(data.size() == 0)
	{
		std::string encodings[] = {"UTF-8", "UTF-8(BOM)", "UTF-16", "CP949"};
		data["lrcsaveencoding"] = std::vector<std::string>(encodings, encodings + arraysizeof(encodings));
	}

	return data[item];
}

std::string LyricSourceLRC::IsConfigValid(std::map<std::string, std::string>)
{
	return "";
}

LyricSourceFactory<LyricSourceLRC> LyricSourceLRCFactory;
