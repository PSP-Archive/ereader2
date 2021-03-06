/*
 * $Id: Bookmark.h 74 2007-10-20 10:46:39Z soarchin $

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
#include "Database.h"

class Bookmark
{
public:
	Bookmark();
	~Bookmark();
	VOID Init(Database * db);
	DWORD Load(CONST CHAR * filename, INT index = -1);
	VOID Save(CONST CHAR * filename, DWORD offset, INT index = -1);
protected:
	Database * m_db;
};
