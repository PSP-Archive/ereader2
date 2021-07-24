/*
 * $Id: Database.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Database.h"
#include "sqlite3.h"

Database::Database(CONST CHAR * filename)
{
	m_stmt = NULL;
	sqlite3 *db;
	INT rc;

	rc = sqlite3_open(filename, &db);
	if( rc )
	{
		m_db = NULL;
		sqlite3_close(db);
		return;
	}
	m_db = db;
	Exec("BEGIN");
	Exec("PRAGMA page_size = 1024");
	Exec("PRAGMA default_cache_size = 256");
	Exec("COMMIT");
	return;
}

Database::~Database()
{
	Finalize();
	if(m_db != NULL)
	{
		sqlite3 *db = (sqlite3 *)m_db;
		sqlite3_close(db);
		m_db = NULL;
	}
}

BOOL Database::InitSucc()
{
	return (m_db != NULL);
}

BOOL Database::Exec(CONST CHAR * cmd)
{
	if(m_db == NULL)
		return FALSE;
	CHAR *zErrMsg = 0;
	sqlite3 *db = (sqlite3 *)m_db;
	INT rc;
	rc = sqlite3_exec(db, cmd, 0, 0, &zErrMsg);
	if( rc!=SQLITE_OK )
	{
		sqlite3_free(zErrMsg);
		return FALSE;
	}
	return TRUE;
}

BOOL Database::Prepare(CONST CHAR * cmd)
{
	if(m_db == NULL)
		return FALSE;
	Finalize();
	sqlite3 *db = (sqlite3 *)m_db;
	sqlite3_stmt *stmt;
	INT rc;
	rc = sqlite3_prepare(db, cmd, -1, &stmt, NULL); 
	if (rc == SQLITE_OK)
	{
		m_stmt = stmt;
		return TRUE;
	}
	return FALSE;
}

BOOL Database::Step()
{
	if(m_stmt == NULL)
		return FALSE;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	INT rc;
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW)
	{
		return TRUE;
	}
	Finalize();
	return FALSE;
}

VOID Database::Finalize()
{
	if(m_stmt != NULL)
	{
		sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
		sqlite3_finalize(stmt);
		m_stmt = NULL;
	}
}

INT Database::GetCols()
{
	if(m_stmt == NULL)
		return 0;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_count(stmt);
}

INT Database::GetColInt(INT col)
{
	if(m_stmt == NULL)
		return 0;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_int(stmt, col);
}

DOUBLE Database::GetColFloat(INT col)
{
	if(m_stmt == NULL)
		return 0.0;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_double(stmt, col);
}

CONST BYTE * Database::GetColText(INT col)
{
	if(m_stmt == NULL)
		return NULL;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_text(stmt, col);
}
