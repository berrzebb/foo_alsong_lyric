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

#include "UIPreference.h"

extern cfg_window_placement cfg_outer_window_placement;
extern cfg_bool cfg_outer_shown;
extern cfg_bool cfg_outer_topmost;
extern cfg_int cfg_outer_transparency;
extern cfg_bool cfg_outer_layered;
extern cfg_bool cfg_outer_blur;
extern cfg_bool cfg_outer_nonlyric;

extern cfg_string cfg_outer_script;
extern cfg_struct_t<UIPreference> cfg_outer;

extern cfg_bool cfg_outer_nolayered;

extern cfg_bool cfg_skipempty;

class cfg_lyricsource_var : public cfg_var
{
private:
	std::vector<GUID> m_reallist;
public:
	cfg_lyricsource_var(const GUID &guid) : cfg_var(guid) {}

	void add(const GUID &guid)
	{
		m_reallist.emplace_back(guid);
	}

	std::vector<GUID> get_value()
	{
		return m_reallist;
	}

	void remove(const GUID &guid)
	{
		std::vector<GUID>::iterator it = std::find(m_reallist.begin(), m_reallist.end(), guid);
		if(it == m_reallist.end())
			return;
		m_reallist.erase(it);
	}

	bool exists(const GUID &guid)
	{
		std::vector<GUID>::iterator it = std::find(m_reallist.begin(), m_reallist.end(), guid);
		if(it == m_reallist.end())
			return false;
		return true;
	}

	void remove(int idx)
	{
		m_reallist.erase(m_reallist.begin() + idx);
	}

	void insert(int idx, const GUID &guid)
	{
		m_reallist.insert(m_reallist.begin() + idx, guid);
	}

	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
	{
		for(std::vector<GUID>::iterator it = m_reallist.begin(); it != m_reallist.end(); it ++)
			p_stream->write(&*it, sizeof(GUID), p_abort);
	}

	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort)
	{
		int cnt = p_sizehint / sizeof(GUID);

		for(int i = 0; i < cnt; i ++)
		{
			GUID tmp;
			p_stream->read(&tmp, sizeof(GUID), p_abort);
			m_reallist.emplace_back(tmp);
		}
	}
};

class cfg_lyricsourcecfg_var : public cfg_var
{
private:
	std::map<GUID, std::map<std::string, std::string> > m_cfgmap; //guid, key, value
public:
	cfg_lyricsourcecfg_var(const GUID &guid) : cfg_var(guid) {}

	std::map<std::string, std::string> get_value(const GUID &guid)
	{
		return m_cfgmap[guid];
	}

	void set_value(const GUID &guid, const std::map<std::string, std::string> &value)
	{
		m_cfgmap[guid] = value;
	}

	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
	{
		p_stream->write_lendian_t(m_cfgmap.size(), p_abort);
		for(std::map<GUID, std::map<std::string, std::string> >::iterator it = m_cfgmap.begin(); it != m_cfgmap.end(); it ++)
		{
			p_stream->write(&it->first, sizeof(GUID), p_abort);
			p_stream->write_lendian_t(it->second.size(), p_abort);
			for(std::map<std::string, std::string>::iterator iit = it->second.begin(); iit != it->second.end(); iit ++)
			{
				p_stream->write_string(iit->first.c_str(), p_abort);
				p_stream->write_string(iit->second.c_str(), p_abort);
			}
		}
	}

	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort)
	{
		int cnt;
		p_stream->read_lendian_t(cnt, p_abort);
		for(int i = 0; i < cnt; i ++)
		{
			GUID guid;
			p_stream->read(&guid, sizeof(guid), p_abort);
			int itemcnt;
			p_stream->read_lendian_t(itemcnt, p_abort);
			for(int j = 0; j < itemcnt; j ++)
			{
				pfc::string8 key, value;
				p_stream->read_string(key, p_abort);
				p_stream->read_string(value, p_abort);
				m_cfgmap[guid][std::string(key.get_ptr())] = std::string(value.get_ptr());
			}
		}
	}
};
extern cfg_lyricsourcecfg_var cfg_lyricsourcecfg;
extern cfg_lyricsource_var cfg_enabledlyricsource;
extern cfg_lyricsource_var cfg_enabledlyricsave;

extern cfg_bool cfg_outer_taskbar;
extern cfg_bool cfg_use_jumplist;