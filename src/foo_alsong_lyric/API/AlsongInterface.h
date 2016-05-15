#pragma once
#include "./API/AlsongAPI.h"
class LyricSearchResultAlsong;
class CAlsongInterface
{
public:
	void GetLyric8(std::string Checksum,std::string MACAddress,std::string IPAddress,_ns1__GetLyric8Response& Response);
	int GetResembleLyric2Count(std::string artist,std::string Title);
	LyricSearchResultAlsong* GetResembleLyric2(std::string artist,std::string Title,const int& nPage = 0);
	std::string UploadLyric(const int& nType, std::string Hash,std::string Filename,std::string Title,std::string Artist,std::string Album, std::string Local_Mac,std::string Local_IP,std::string RawLyric,std::string strRegisterName,const int& PlayTime,const int& nInfoID = 0);
private:
	CAlsongAPI API;
};

