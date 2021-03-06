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

#include "Lyric.h"

Lyric::Lyric()
{

}

Lyric::Lyric(const char *raw)
{
	Info.sLyric = raw;
	Split("\r\n");
}

Lyric::Lyric(const Lyric& other) : Info(other.Info)
{
	Split("\r\n");
}

Lyric::~Lyric()
{

}

DWORD Lyric::Split(const char *Delimiter)
{
	m_LyricLines.clear();

	const char *nowpos = Info.sLyric.data();
	const char *lastpos = nowpos;
	int delimlen = lstrlenA(Delimiter);
	unsigned int pos;
	while(pos = boost::find_first(nowpos, Delimiter).end() - nowpos) //<br>자르기
	{
		if(pos <= 10) break;
		nowpos = nowpos + pos;

		int time = StrToIntA(lastpos + 1) * 60 * 100 + StrToIntA(lastpos + 4) * 100 + StrToIntA(lastpos + 7);
		lastpos += 10; //strlen("[34:56.78]");

		std::string temp(lastpos, nowpos - lastpos - delimlen);
		if(temp.length() == 0)
			temp = " ";
		m_LyricLines.emplace_back(time, temp);
		lastpos = nowpos;
	}

	m_LyricIterator = m_LyricLines.begin();

	return S_OK;
}

void Lyric::Clear()
{
	Info.clear();
	m_LyricLines.clear();
	m_LyricIterator = m_LyricLines.begin();
}

std::vector<LyricLine>::const_iterator Lyric::GetIteratorAt(unsigned int time) const
{
	std::vector<LyricLine>::const_iterator it;
	for(it = m_LyricLines.cbegin(); it != m_LyricLines.cend() && it->time < time; ++it);

	return it;
}

int Lyric::IsValidIterator(std::vector<LyricLine>::const_iterator it) const
{
	try
	{
		if(!it._Ptr)
			return false;
		return std::find(m_LyricLines.cbegin(), m_LyricLines.cend(), *it) != m_LyricLines.cend();
	}
	catch(...)
	{
		return false;
	}
}
