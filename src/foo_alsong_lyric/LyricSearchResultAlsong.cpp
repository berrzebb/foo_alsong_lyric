﻿/*
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

#include "LyricSearchResultAlsong.h"

LyricSearchResultAlsong::LyricSearchResultAlsong(std::vector<AlsongLyric> data) 
{
	m_Document = data;
	m_LyricNode = m_Document.begin();
	m_LyricResultMap[-1] = AlsongLyric();
}

Lyric *LyricSearchResultAlsong::Get()
{
	if(m_LyricNode != m_Document.end() || m_LyricNode == m_Document.begin()){
		AlsongLyric ret(*m_LyricNode);
		m_LyricResultMap[ret.GetInternalID()] = ret;
		m_LyricNode++;
		return dynamic_cast<Lyric *>(&m_LyricResultMap.find(ret.GetInternalID())->second);
	}else{
		return dynamic_cast<Lyric *>(&m_LyricResultMap.find(-1)->second);
	}
}

Lyric *LyricSearchResultAlsong::Get(int id)
{
	if(m_LyricResultMap.find(id) != m_LyricResultMap.end())
		return dynamic_cast<Lyric *>(&m_LyricResultMap.find(id)->second);
	else
		return dynamic_cast<Lyric *>(&m_LyricResultMap.find(-1)->second);
}