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

#include "LyricSourceAlsong.h"
#include "LyricSearchResultAlsong.h"
#include "md5.h"
#include "AlsongLyric.h"
#include "FLAC++/all.h"
#include "LyricsInfo.h"
#include "AudioHash.h"

DWORD LyricSourceAlsong::GetFileHash(const metadb_handle_ptr &track, CHAR *Hash)
{	
	service_ptr_t<file> sourcefile;
	abort_callback_impl abort_callback;
	try
	{
		archive_impl::g_open(sourcefile, track->get_path(), foobar2000_io::filesystem::open_mode_read, abort_callback);
		IAudioHash* audiohash;
		BYTE hashbuf[4];
		sourcefile->seek(0,abort_callback);
		sourcefile->read(hashbuf,4,abort_callback);
		if(!memcmp(hashbuf,"ID3",3)){
			audiohash = new Mpeg(sourcefile);
		}else if(!memcmp(hashbuf,"fLaC",4)){
			audiohash = new Flac(sourcefile);
		}else if(!memcmp(hashbuf,"OggS",4)){
			audiohash = new Ogg(sourcefile);
		}else{
			audiohash = new Cue(track);
		}
		memcpy_s(Hash,audiohash->Hash.size(),audiohash->Hash.data(),audiohash->Hash.size());
	}catch(...){
		return false;
	}
	return true;
}

boost::shared_ptr<Lyric> LyricSourceAlsong::Get(const metadb_handle_ptr &track)
{
	boost::shared_ptr<Lyric> result(new AlsongLyric());
	struct hostent *host;
	CHAR Hostname[80];
	CHAR *Local_IP;
	CHAR Local_Mac[20];

	gethostname(Hostname, 80);
	host = gethostbyname(Hostname);

	struct in_addr addr;
	if(host == NULL)
		return result;
	memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
	Local_IP = inet_ntoa(*((in_addr *)host->h_addr_list[0]));

	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen = sizeof(AdapterInfo);

	GetAdaptersInfo(AdapterInfo, &dwBufLen);
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

	while(pAdapterInfo) 
	{
		if(!lstrcmpA(pAdapterInfo->IpAddressList.IpAddress.String, Local_IP))
			break;
		pAdapterInfo = pAdapterInfo->Next;
	}
	if(pAdapterInfo == NULL)
		return result;

	CHAR HexArray[] = "0123456789ABCDEF";
	int i;
	for(i = 0; i < 12; i += 2)
	{
		Local_Mac[i] = HexArray[(pAdapterInfo->Address[i / 2] & 0xf0) >> 4];
		Local_Mac[i + 1] = HexArray[pAdapterInfo->Address[i / 2] & 0x0f];
	}
	Local_Mac[i] = 0;

	CHAR Hash[50];
	GetFileHash(track, Hash);

	CAlsongInterface Interface;
	_ns1__GetLyric8Response Response;
	Interface.GetLyric8(Hash,Local_Mac,Local_IP,Response);
	if(boost::this_thread::interruption_requested())
		return result;
	try
	{
		if(Response.GetLyric8Result == NULL) return boost::shared_ptr<Lyric>();
		AlsongLyricInfo Info;
		Info.nInfoID = stoi(Response.GetLyric8Result->strInfoID->data()); 
		if(Info.nInfoID != -1){
			Info.sTitle = Response.GetLyric8Result->strTitle->data();
			Info.sArtist = Response.GetLyric8Result->strArtist->data();
			Info.sAlbum = Response.GetLyric8Result->strAlbum->data();
			Info.sRegistrant = Response.GetLyric8Result->strRegisterName->data();
			Info.sLyric = Response.GetLyric8Result->strLyric->data();
			result.reset(new AlsongLyric(Info));
		}else{
			service_ptr_t<titleformat_object> to;
			pfc::string8 title;
			pfc::string8 artist;

			static_api_ptr_t<titleformat_compiler>()->compile_safe(to, "%title%");
			track->format_title(NULL, title, to, NULL);
			static_api_ptr_t<titleformat_compiler>()->compile_safe(to, "%artist%");
			track->format_title(NULL, artist, to, NULL);
			boost::shared_ptr<LyricSearchResult> Result(SearchLyric(artist.get_ptr(),title.get_ptr(),0));
			AlsongLyric lyric(static_cast<AlsongLyric>(*Result->Get()));
			if(Result->Get()->HasLyric())
				result.reset(Result->Get());
		}
	}
	catch(...)
	{
		return result;
	}
	return result;
}

DWORD LyricSourceAlsong::Save(const metadb_handle_ptr &track, const std::string &Artist, const std::string &Title, const std::string &RawLyric, const std::string &Album, const std::string &Registrant, int nInfoID)
{
	std::string strRegisterName;
	if(Registrant.length() == 0)
		strRegisterName = "Alsong Lyric Plugin for Foobar2000";//나머지는 생략
	else
		strRegisterName = Registrant;

	//UploadLyricType - 1:Link 새거 2:Modify 수정 5:ReSetLink 아예 새거

	struct hostent *host;
	CHAR Hostname[80];
	CHAR *Local_IP;
	CHAR Local_Mac[20];
	CHAR Hash[50];

	std::string Filename = track->get_path();
	int PlayTime = (int)(track->get_length() * 1000);

	gethostname(Hostname, 80);
	host = gethostbyname(Hostname);

	struct in_addr addr;
	memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
	Local_IP = inet_ntoa(*((in_addr *)host->h_addr_list[0]));

	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen = sizeof(AdapterInfo);

	GetAdaptersInfo(AdapterInfo, &dwBufLen);
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

	while(pAdapterInfo) 
	{
		if(!lstrcmpA(pAdapterInfo->IpAddressList.IpAddress.String, Local_IP))
			break;
		pAdapterInfo = pAdapterInfo->Next;
	}
	if(pAdapterInfo == NULL)
		return false;

	CHAR HexArray[] = "0123456789ABCDEF";
	int i;
	for(i = 0; i < 12; i += 2)
	{
		Local_Mac[i] = HexArray[(pAdapterInfo->Address[i / 2] & 0xf0) >> 4];
		Local_Mac[i + 1] = HexArray[pAdapterInfo->Address[i / 2] & 0x0f];
	}
	Local_Mac[i] = 0;

	GetFileHash(track, Hash);
	
	CAlsongInterface Interface;
	try
	{
		std::string temp = 	Interface.UploadLyric(1,Hash,Filename,Title,Artist,Album,Local_Mac,Local_IP,RawLyric,strRegisterName,PlayTime,nInfoID);
		const char *res = temp.c_str();
		if(boost::find_first(res, "Successed"))
			return true;
	}
	catch(...)
	{
		return false;
	}
	return false;
}

DWORD LyricSourceAlsong::Save(const metadb_handle_ptr &track, Lyric &lyric)
{
	return LyricSourceAlsong::Save(track, lyric.GetArtist(), lyric.GetTitle(), lyric.GetRawLyric(), lyric.GetAlbum(), lyric.GetRegistrant(), dynamic_cast<AlsongLyric &>(lyric).GetInternalID());
}

boost::shared_ptr<LyricSearchResult> LyricSourceAlsong::SearchLyric(const std::string &Artist, const std::string Title, int nPage)
{
	CAlsongInterface Interface;
	try
	{
		return boost::shared_ptr<LyricSearchResult>(Interface.GetResembleLyric2(Artist,Title,nPage));
	}
	catch(...)
	{
		return boost::shared_ptr<LyricSearchResult>();
	}
}

int LyricSourceAlsong::SearchLyricGetCount(const std::string &Artist, const std::string &Title)
{
	CAlsongInterface Interface;
	try
	{
		return Interface.GetResembleLyric2Count(Artist,Title); //TODO: Test
	}
	catch(...)
	{
		return 0;
	}
}

std::map<std::string, LyricSource::ConfigItemType> LyricSourceAlsong::GetConfigItems(int type)
{
	static std::map<std::string, LyricSource::ConfigItemType> ret;
	if(ret.size() == 0)
	{
	}

	return ret;
}

std::string LyricSourceAlsong::GetConfigDescription(std::string item)
{
	static std::map<std::string, std::string> data;
	if(data.size() == 0)
	{
	}
	return data[item];
}

std::string LyricSourceAlsong::GetConfigLabel(std::string item)
{
	static std::map<std::string, std::string> data;
	if(data.size() == 0)
	{
	}
	return data[item];
}

std::vector<std::string> LyricSourceAlsong::GetConfigEnumeration(std::string item)
{
	static std::map<std::string, std::vector<std::string> > data;
	if(data.size() == 0)
	{
	}

	return data[item];
}

std::string LyricSourceAlsong::IsConfigValid(std::map<std::string, std::string>)
{
	return "";
}
LyricSourceFactory<LyricSourceAlsong> LyricSourceAlsongFactory;
