#pragma once
#include <string>
struct LyricsInfo{
	std::string sTitle;
	std::string sArtist;
	std::string sAlbum;
	std::string sRegistrant;
	std::string sLyric;

	void clear(){
		sTitle.clear();
		sAlbum.clear();
		sArtist.clear();
		sRegistrant.clear();
		sLyric.clear();
	}
};
struct AlsongLyricInfo : LyricsInfo {
	int nInfoID;
};