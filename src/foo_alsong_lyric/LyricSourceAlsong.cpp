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

DWORD LyricSourceAlsong::GetFileHash(const metadb_handle_ptr &track, CHAR *Hash)
{	
	int i;
	DWORD Start = 0; //Start Address
	BYTE MD5[16];
	BYTE temp[255]; 

	service_ptr_t<file> sourcefile;
	abort_callback_impl abort_callback;
	pfc::string8 str = track->get_path();

	try
	{
		archive_impl::g_open(sourcefile, str, foobar2000_io::filesystem::open_mode_read, abort_callback);
	}
	catch(...)
	{
		return false;
	}
	//TODO:cue일때 특별 처리(subsong_index가 있을 때)
	char *fmt = (char *)str.get_ptr() + str.find_last('.') + 1;

	try 
	{

		if(!StrCmpIA(fmt, "cue"))
		{
			file_info_impl info;
			track->get_info(info);
			const char *realfile = info.info_get("referenced_file");
			const char *ttmp = info.info_get("referenced_offset");
			int m, s, ms;
			if(ttmp == NULL)
				m = s = ms = 0;
			else
			{
				std::stringstream stream(ttmp);
				char unused;
				stream >> m >> unused >> s >> unused >> ms;
				const char *pregap = info.info_get("pregap");
				if(pregap)
				{
					std::stringstream stream(pregap);
					int pm, ps, pms;
					stream >> pm >> unused >> ps >> unused >> pms;
					m += pm;
					s += ps;
					ms += pms;
				}
			}

			audio_chunk_impl chunk;
			pfc::string realfilename = pfc::io::path::getDirectory(str) + "\\" + realfile;

			input_helper helper;

			// open input
			helper.open(service_ptr_t<file>(), make_playable_location(realfilename.get_ptr(), 0), input_flag_simpledecode, abort_callback);

			helper.get_info(0, info, abort_callback);
			helper.seek(m * 60 + s + ms * 0.01, abort_callback);

			if (!helper.run(chunk, abort_callback)) return false;		

			t_uint64 length_samples = audio_math::time_to_samples(info.get_length(), chunk.get_sample_rate());
			//chunk.get_channels();
			std::vector<double> buf;
			buf.resize(0x200000);
			while (true)
			{
				// Store the data somewhere.
				audio_sample *sample = chunk.get_data();
				int len = chunk.get_data_length();
				buf.insert(buf.end(), sample, sample + len);
				if(buf.size() > 0x200000)
					break;

				bool decode_done = !helper.run(chunk, abort_callback);
				if (decode_done) break;
			}

			md5((unsigned char *)&buf[0], min(buf.size(), 0x200000), MD5);
		}
		else
		{
			if(!StrCmpIA(fmt, "mp3"))
			{
				while(1) //ID3가 여러개 있을수도 있음
				{ //ID3는 보통 맨 처음에 있음
					sourcefile->seek(Start, abort_callback);
					sourcefile->read(temp, 3, abort_callback);
					if(temp[0] == 'I' && temp[1] == 'D' && temp[2] == '3')
					{
						sourcefile->read(temp, 7, abort_callback);
#define ID3_TAGSIZE(x) ((*(x) << 21) | (*((x) + 1) << 14) | (*((x) + 2) << 7) | *((x) + 3))
						Start += ID3_TAGSIZE(temp + 3) + 10;
#undef ID3_TAGSIZE
					}
					else
						break;
				}
				sourcefile->seek(Start, abort_callback);
				for(;;Start ++)
				{
					BYTE temp;
					sourcefile->read_lendian_t(temp, abort_callback);
					if(temp == 0xFF) //MP3 Header까지
						break;
				}
			}
			else if(!StrCmpIA(fmt, "ogg"))
			{
				//처음 나오는 vorbis setup header 검색
				i = 0;
				CHAR SetupHeader[7] = {0x05, 0x76, 0x6F, 0x72, 0x62, 0x69, 0x73}; //Vorbis Setup Header
				CHAR BCV[3] = {'B', 'C', 'V'}; //codebook start?
				while(1)
				{
					sourcefile->seek(i, abort_callback);
					sourcefile->read(temp, 7, abort_callback);
					if(!memcmp(temp, SetupHeader, 7))
					{
						sourcefile->seek(i + 7 + 1, abort_callback);
						sourcefile->read(temp, 3, abort_callback);
						if(!memcmp(temp, BCV, 3)) //Setup Header와 BCV 사이에 뭔가 바이트가 하나 더 있다.
						{
							//여기부터다
							Start = i + 7 + 1 + 3;
							break;
						}
					}
					i ++;
					if(i > sourcefile->get_size(abort_callback))
						return false; //에러
				}

			}
			else
				Start = 0;

			BYTE *buf = (BYTE *)malloc(0x28000);

			try
			{
				sourcefile->seek(Start, abort_callback);
				sourcefile->read(buf, min(0x28000, (size_t)sourcefile->get_size(abort_callback) - Start), abort_callback);
			}
			catch(...)
			{
				free(buf);
				return false;
			}

			md5(buf, min(0x28000, (size_t)sourcefile->get_size(abort_callback) - Start), MD5); //FileSize < 0x28000 일수도
			free(buf);
		}
	}
	catch(...)
	{
		return false;
	}

	CHAR HexArray[] = "0123456789abcdef";

	for(i = 0; i < 32; i += 2)
	{
		Hash[i] = HexArray[(MD5[i / 2] & 0xf0) >> 4];
		Hash[i + 1] = HexArray[MD5[i / 2] & 0x0f];
	}
	Hash[i] = 0;

	return true;
}

boost::shared_ptr<Lyric> LyricSourceAlsong::Get(const metadb_handle_ptr &track)
{
	struct hostent *host;
	CHAR Hostname[80];
	CHAR *Local_IP;
	CHAR Local_Mac[20];

	gethostname(Hostname, 80);
	host = gethostbyname(Hostname);

	struct in_addr addr;
	if(host == NULL)
		return boost::shared_ptr<Lyric>(new AlsongLyric());
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
		return boost::shared_ptr<Lyric>(new AlsongLyric());

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
		return boost::shared_ptr<Lyric>(new AlsongLyric());

	try
	{
		return boost::shared_ptr<Lyric>(new AlsongLyric(Response));
	}
	catch(...)
	{
		return boost::shared_ptr<Lyric>();
	}
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
