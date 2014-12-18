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

class EncodingFunc
{
public:
	static std::string ToUTF8(wchar_t *utf16)
	{
		std::vector<char> ret;
		unsigned int i = 0;
		while(utf16[i])
		{
			if(utf16[i] < 0xD800 || utf16[i] > 0xDFFF)
			{ //no surrogate pair
				if(utf16[i] < 0x80) //1 byte
				{
					ret.emplace_back((char)utf16[i]);
				}
				else if(utf16[i] < 0x800)
				{
					ret.emplace_back((char)(0xC0 | ((utf16[i] & 0x7C0) >> 6)));
					ret.emplace_back((char)(0x80 | (utf16[i] & 0x3F)));
				}
				else
				{
					ret.emplace_back((char)(0xE0 | ((utf16[i] & 0xF000) >> 12)));
					ret.emplace_back((char)(0x80 | ((utf16[i] & 0xFC0) >> 6)));
					ret.emplace_back((char)(0x80 | (utf16[i] & 0x3F)));
				}
			}
			else
			{
				int temp;
				temp = (utf16[i] & 0x3FF << 10) | (utf16[i + 1] & 0x3FF);
				temp += 0x10000;
				if(temp < 0x80) //1 byte
				{
					ret.emplace_back((char)temp);
				}
				else if(temp < 0x800)
				{
					ret.emplace_back((char)(0xC0 | ((temp & 0x7C0) >> 6)));
					ret.emplace_back((char)(0x80 | (temp & 0x3F)));
				}
				else if(temp < 0x10000)
				{
					ret.emplace_back((char)(0xE0 | ((temp & 0xF000) >> 12)));
					ret.emplace_back((char)(0x80 | ((temp & 0xFC0) >> 6)));
					ret.emplace_back((char)(0x80 | (temp & 0x3F)));
				}
				else
				{
					ret.emplace_back((char)(0xF0 | ((temp & 0x1C0000) >> 18)));
					ret.emplace_back((char)(0x80 | ((temp & 0x3F000) >> 12)));
					ret.emplace_back((char)(0x80 | ((temp & 0xFC0) >> 6)));
					ret.emplace_back((char)(0x80 | (temp & 0x3F)));
				}
				i ++;
			}
			i ++;
		}

		return std::string(ret.begin(), ret.end());
	}

	static std::wstring ToUTF16(std::string utf8)
	{
		std::wstring ret;
		unsigned int i = 0;
		while(i < utf8.length())
		{
			if(!((utf8[i] & 0xF0) == 0xF0))
			{
				//1 utf-16 character
				if((utf8[i] & 0xE0) == 0xE0)
				{
					ret += ((utf8[i] & 0x0f) << 12) | (utf8[i + 1] & 0x3f) << 6 | (utf8[i + 2] & 0x3f);
					i += 2;
				}
				else if((utf8[i] & 0xC0) == 0xC0)
				{
					ret += ((utf8[i] & 0x1f) << 6) | (utf8[i + 1] & 0x3f);
					i ++;
				}
				else
				{
					ret += utf8[i];
				}
			}
			else
			{
				int temp = ((utf8[i] & 0x07) << 18) | ((utf8[i + 1] & 0x3f) << 12) | ((utf8[i + 2] & 0x3f) << 6) | (utf8[i + 3] & 0x3f);
				i += 3;
				temp -= 0x10000;
				ret += 0xD800 | (temp & 0xFFC00);
				ret += 0xDC00 | (temp & 0x3FF);
			}
			i ++;
		}
		return ret;
	}
};