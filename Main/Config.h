/*
 * $Id: Config.h 77 2007-11-05 09:21:58Z soarchin $

 * Copyright 2007 aeolusc

 * This file is part of eReader2.

 * eReader2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * eReader2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Datatype.h"
#include "Charsets.h"
#include "Database.h"

class Config
{
public:
	Config();
	~Config();
	VOID Init(Database * db);
	template<typename T>
	T operator [](INT idx) { return (T)m_conflist[idx].data; }
	template<typename T>
	VOID Set(INT idx, T data)
	{
		switch(m_conflist[idx].itype)
		{
		case IT_Int:
			m_conflist[idx].data = (VOID *)data;
			break;
		case IT_Text:
			m_conflist[idx].data = (VOID *)ASC.Dup((CONST CHAR *)data);
		}
	}
protected:
	enum ItemType {IT_Int = 0, IT_Text = 1};
	struct ConfItem
	{
		CONST CHAR * name;
		ItemType itype;
		VOID * data;
	};
	static ConfItem m_conflist[];
	Database * m_db;
};

Config * g_Config;

#define Conf(n) (*g_Config)[n];
