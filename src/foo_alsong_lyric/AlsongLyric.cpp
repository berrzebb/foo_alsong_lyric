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

#include "AlsongLyric.h"
#include "./API/AlsongAPI.h"
AlsongLyric::AlsongLyric(const _ns1__GetLyric8Response& Response)
{
	auto item = Response.GetLyric8Result;
	if(!item)
		return;
	if(!item->strInfoID->c_str()){
		m_Title = item->strTitle->c_str();
		m_Artist = item->strArtist->c_str();
		m_Album = item->strAlbum->c_str();
		m_Registrant = item->strRegisterFirstName->c_str();
		m_Lyric = item->strLyric->c_str();

		if(!m_Album.compare(m_Title))
			m_Album.clear();
		m_nInfoID = -1;
	}
	else{
		m_Title = item->strTitle->c_str();
		m_Artist = item->strArtist->c_str();
		m_Album = item->strAlbum->c_str();
		m_Registrant = item->strRegisterFirstName->c_str();
		m_Lyric = item->strLyric->c_str();
		m_nInfoID = boost::lexical_cast<int>(item->strInfoID->c_str());
	}
	Split("<br>");
}

AlsongLyric::AlsongLyric(const ns1__ST_USCOREGET_USCORERESEMBLELYRIC2_USCORERETURN& Response)
{
	if(!Response.strInfoID->c_str()){
		m_Title = Response.strTitle->c_str();
		m_Artist = Response.strArtistName->c_str();
		m_Album = Response.strAlbumName->c_str();
		m_Registrant = Response.strRegisterFirstName->c_str();
		m_Lyric = Response.strLyric->c_str();

		if(!m_Album.compare(m_Title))
			m_Album.clear();
		m_nInfoID = -1;
	}
	else{
		m_Title = Response.strTitle->c_str();
		m_Artist = Response.strArtistName->c_str();
		m_Album = Response.strAlbumName->c_str();
		m_Registrant = Response.strRegisterFirstName->c_str();
		m_Lyric = Response.strLyric->c_str();
		m_nInfoID = boost::lexical_cast<int>(Response.strInfoID->c_str());
	}
	Split("<br>");
}
