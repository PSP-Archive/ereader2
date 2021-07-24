/*
 * Bookmark.cpp

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

#include "Bookmark.h"
#include "Log.h"
#include <stdio.h>

Bookmark::Bookmark()
{
	m_db = NULL;
}

Bookmark::~Bookmark()
{
}

VOID Bookmark::Init(Database * db)
{
	m_db = db;
	m_db->Exec("CREATE TABLE IF NOT EXISTS bookmark (filename TEXT, defoff INTEGER, off1 INTEGER, off2 INTEGER, off3 INTEGER, off4 INTEGER, PRIMARY KEY(filename) )");
}

DWORD Bookmark::Load(CONST CHAR * filename, INT index)
{
	CHAR cmd[256];
	if(index < 0)
		sprintf(cmd, "SELECT defoff FROM bookmark WHERE filename='%s'", filename);
	else
		sprintf(cmd, "SELECT off%d FROM bookmark WHERE filename='%s'", index, filename);
	if(!m_db->Prepare(cmd))
		return 0;
	if(!m_db->Step())
		return 0;
	INT res = m_db->GetColInt(0);
	m_db->Finalize();
	return res;
}

VOID Bookmark::Save(CONST CHAR * filename, DWORD offset, INT index)
{
	CHAR cmd[256];
	if(index < 0)
		sprintf(cmd, "INSERT OR REPLACE INTO bookmark (filename, defoff) VALUES ('%s', %u)", filename, (UINT)offset);
	else
		sprintf(cmd, "INSERT OR REPLACE INTO bookmark (filename, off%d) VALUES ('%s', %u)", index, filename, (UINT)offset);
	m_db->Exec(cmd);
}
