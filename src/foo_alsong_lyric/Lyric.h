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

#pragma once

#include <string>
#include <vector>

struct LyricLine
{
	LyricLine(DWORD _time, const std::string &_lyric) : time(_time), lyric(_lyric) {}
	LyricLine() : time(0), lyric("") {}
	DWORD time;
	std::string lyric;

	int operator ==(const LyricLine &line) const
	{
		return (lyric == line.lyric) && (time == line.time);
	}
};

class Lyric
{
protected:
	std::string m_Title;
	std::string m_Album;
	std::string m_Artist;
	std::string m_Registrant;
	std::string m_Lyric;
	
	std::vector<LyricLine> m_LyricLines;
	std::vector<LyricLine>::iterator m_LyricIterator;

	DWORD Split(const char *Delimiter);
public:
	Lyric();
	Lyric(const char *raw);
	~Lyric();

	void Clear();
	
	std::vector<LyricLine>::const_iterator GetIteratorAt(unsigned int time) const; //returns lyric right after time.
	
	std::string GetRawLyric() const
	{
		return m_Lyric;
	}

	std::string GetTitle() const
	{
		return m_Title;
	}

	std::string GetAlbum() const
	{
		return m_Album;
	}

	std::string GetArtist() const
	{
		return m_Artist;
	}

	std::string GetRegistrant() const
	{
		return m_Registrant;
	}

	int IsEndOfLyric(std::vector<LyricLine>::const_iterator it) const
	{
		return it == m_LyricLines.end();
	}

	int IsBeginOfLyric(std::vector<LyricLine>::const_iterator it) const
	{
		return it == m_LyricLines.begin();
	}

	int HasLyric() const
	{
		return m_LyricLines.size() != 0;
	}

	int IsValidIterator(std::vector<LyricLine>::const_iterator) const;

	virtual int GetInternalID() const = 0;
};
