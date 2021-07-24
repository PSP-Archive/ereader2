/*
 * $Id: Config.cpp 77 2007-11-05 09:21:58Z soarchin $

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

#include "Config.h"
#include <string.h>
#include "Charsets.h"

Config::ConfItem Config::m_conflist[] = 
{
	{"LastFile", IT_Text, 0},
	{NULL}
};

Config::Config()
{
	m_db = NULL;
}

Config::~Config()
{
	ConfItem * item = &m_conflist[0];
	while(item->name != 0)
	{
		if(item->itype == IT_Text)
			delete (CONST CHAR *)item->data;
		item ++;
	}
}

VOID Config::Init(Database * db)
{
	m_db = db;
	m_db->Exec("CREATE TABLE IF NOT EXISTS config ( name TEXT, value TEXT, PRIMARY KEY(name) )");
	m_db->Prepare("SELECT name, value FROM config");
	while(m_db->Step())
	{
		ConfItem * item = &m_conflist[0];
		while(item->name != 0)
		{
			if(stricmp((CONST CHAR *)m_db->GetColText(0), item->name) == 0)
			{
				switch(item->itype)
				{
				case IT_Int:
					item->data = (VOID *)m_db->GetColInt(1);
					break;
				case IT_Text:
					item->data = (VOID *)ASC.Dup((CONST CHAR *)m_db->GetColText(1));
					break;
				}
				break;
			}
			item ++;
		}
	}
	m_db->Finalize();
}
