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

#include "Lyric.h"
#include "LyricSearchResult.h"

interface LyricSource
{
public:
	virtual boost::shared_ptr<Lyric> Get(const metadb_handle_ptr &track) = 0;
	virtual DWORD Save(const metadb_handle_ptr &track, Lyric &lyric) = 0;
	virtual int SearchLyricGetCount(const std::string &Artist, const std::string &Title) = 0;
	virtual boost::shared_ptr<LyricSearchResult> SearchLyric(const std::string &Artist, const std::string Title, int nPage = 0) = 0;

	virtual std::string GetName() = 0;
	virtual GUID GetGUID() = 0;

	virtual std::map<std::string, std::string> GetConfig() = 0;
	virtual void SetConfig(const std::map<std::string, std::string> &setting) = 0;

	enum ConfigItemType
	{
		ITEM_TYPE_STRING,
		ITEM_TYPE_ENUM,
	};

	virtual std::map<std::string, ConfigItemType> GetConfigItems(int type) = 0; //type 1:get, type 2:save
	virtual std::string GetConfigDescription(std::string item) = 0;
	virtual std::string GetConfigLabel(std::string item) = 0;
	virtual std::vector<std::string> GetConfigEnumeration(std::string item) = 0;
	virtual std::string IsConfigValid(std::map<std::string, std::string>) = 0; //Check for config validitiy.
};

class LyricSourceManager
{
private:
	static std::map<GUID, boost::shared_ptr<LyricSource> > m_lyricSources;
public:
	static void Add(boost::shared_ptr<LyricSource> source)
	{
		m_lyricSources[source->GetGUID()] = source;
	}

	static boost::shared_ptr<LyricSource> Get(GUID guid)
	{
		if(m_lyricSources.find(guid) != m_lyricSources.end())
			return m_lyricSources[guid];

		return boost::shared_ptr<LyricSource>();
	}

	static std::vector<boost::shared_ptr<LyricSource> > List()
	{
		std::vector<boost::shared_ptr<LyricSource> > ret;
		for(std::map<GUID, boost::shared_ptr<LyricSource> >::iterator it = m_lyricSources.begin(); it != m_lyricSources.end(); it ++)
			ret.emplace_back(it->second);

		return ret;
	}
};

template<typename T>
class LyricSourceFactory
{
public:
	LyricSourceFactory()
	{
		LyricSourceManager::Add(boost::shared_ptr<T>(new T()));
	}
};
