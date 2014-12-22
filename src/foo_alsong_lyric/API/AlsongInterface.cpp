#include "AlsongInterface.h"
#include "stdafx.h"
#include "LyricSearchResultAlsong.h"
#include "AlsongLyric.h"
#define ALSONG_VERSION "3.02"
std::string Version(ALSONG_VERSION);
void CAlsongInterface::GetLyric8( std::string Checksum,std::string MACAddress,std::string IPAddress,_ns1__GetLyric8Response& Response )
{
	_ns1__GetLyric8* Request =new _ns1__GetLyric8;
	Request->stQuery = new ns1__ST_USCOREGET_USCORELYRIC5_USCOREQUERY;
	Request->stQuery->strChecksum = &Checksum;
	Request->stQuery->strVersion = &Version;
	Request->stQuery->strMACAddress = &MACAddress;
	Request->stQuery->strIPAddress = &IPAddress;
	API.Get()->GetLyric8(Request,Response);
	delete Request->stQuery;
	delete Request;
}
std::string CAlsongInterface::UploadLyric(int nType, std::string Hash,std::string Filename,std::string Title,std::string Artist,std::string Album, std::string Local_Mac,std::string Local_IP,std::string RawLyric,std::string strRegisterName,int PlayTime,int nInfoID)
{
	_ns1__UploadLyric* Request = new _ns1__UploadLyric;
	std::string dummy = "";
	Request->stQuery = new ns1__ST_USCOREUPLOAD_USCORELYRIC_USCOREQUERY;
	Request->stQuery->nUploadLyricType = nType;
	Request->stQuery->strMD5 = &Hash;
	Request->stQuery->strFileName = &Filename;
	Request->stQuery->strTitle = &Title;
	Request->stQuery->strArtist = &Artist;
	Request->stQuery->strAlbum = &Album;

	Request->stQuery->strRegisterName = &strRegisterName;
	Request->stQuery->strRegisterComment = &dummy;
	Request->stQuery->strRegisterEMail = &dummy;
	Request->stQuery->strRegisterURL = &dummy;
	Request->stQuery->strRegisterPhone = &dummy;

	Request->stQuery->strRegisterFirstName = &strRegisterName;
	Request->stQuery->strRegisterFirstComment = &dummy;
	Request->stQuery->strRegisterFirstEMail = &dummy;
	Request->stQuery->strRegisterFirstURL = &dummy;
	Request->stQuery->strRegisterFirstPhone = &dummy;

	if(nInfoID != 0)
		Request->stQuery->nInfoID = nInfoID;

	Request->stQuery->strLyric = &RawLyric;
	Request->stQuery->nPlayTime = PlayTime;
	Request->stQuery->strVersion = &Version;
	Request->stQuery->strMACAddress = &Local_Mac;
	Request->stQuery->strIPAddress = &Local_IP;

	_ns1__UploadLyricResponse Response;
	API.Get()->UploadLyric(Request,Response);
	delete Request->stQuery;
	delete Request;

	return std::string(*Response.UploadLyricResult);
}
int CAlsongInterface::GetResembleLyric2Count( std::string artist,std::string Title )
{
	_ns1__GetResembleLyric2Count* Request = new _ns1__GetResembleLyric2Count;
	_ns1__GetResembleLyric2CountResponse Response;
	Request->stQuery = new ns1__ST_USCOREGET_USCORERESEMBLELYRIC2_USCORECOUNT_USCOREQUERY;
	Request->stQuery->strArtistName = &artist;
	Request->stQuery->strTitle = &Title;
	API.Get()->GetResembleLyric2Count(Request,Response);
	delete Request->stQuery;
	delete Request;
	return boost::lexical_cast<int>(Response.GetResembleLyric2CountResult->strResembleLyricCount->c_str());
}

LyricSearchResultAlsong* CAlsongInterface::GetResembleLyric2( std::string artist,std::string Title,int nPage )
{
	boost::mutex AlsongMutex;
	_ns1__GetResembleLyric2* Request = new _ns1__GetResembleLyric2;
	_ns1__GetResembleLyric2Response Response;
	Request->stQuery = new ns1__ST_USCOREGET_USCORERESEMBLELYRIC2_USCOREQUERY;
	Request->stQuery->strArtistName = &artist;
	Request->stQuery->strTitle = &Title;
	Request->stQuery->nCurPage = nPage;
	API.Get()->GetResembleLyric2(Request,Response);
	delete Request->stQuery;
	delete Request;

	std::vector<AlsongLyric> LyricList;
	if(!Response.GetResembleLyric2Result) {
		LyricList.emplace_back(AlsongLyric());
		return new LyricSearchResultAlsong(LyricList);
	}
	AlsongMutex.lock();
	auto Result = Response.GetResembleLyric2Result->ST_USCOREGET_USCORERESEMBLELYRIC2_USCORERETURN;
	if(Result.empty()){
		AlsongMutex.unlock();
		return NULL;
	}
	while(Result.size()){
		auto item = Result.back();
		Result.pop_back();
		AlsongLyric ret(*item);
		LyricList.emplace_back(ret);
	}
	LyricSearchResultAlsong* res = new LyricSearchResultAlsong(LyricList);
	AlsongMutex.unlock();

	return res;

}
