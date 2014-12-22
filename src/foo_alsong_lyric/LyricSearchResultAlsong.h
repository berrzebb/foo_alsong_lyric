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

#pragma once

#include "LyricSearchResult.h"
#include "AlsongLyric.h"
class LyricSearchResultAlsong : public LyricSearchResult
{
private:
	std::vector<AlsongLyric> m_Document;
	int offset;
	int mask;
	AlsongLyric m_LyricNode;
	std::map<int,AlsongLyric> m_LyricResultMap;
public:
	LyricSearchResultAlsong(std::vector<AlsongLyric> data);
	virtual Lyric *Get();
	virtual Lyric *Get(int id);
};