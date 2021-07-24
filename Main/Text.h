/*
 * $Id: Text.h 74 2007-10-20 10:46:39Z soarchin $

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
#include <vector>

using namespace std;

class Text
{
public:
	Text();
	virtual ~Text();
	virtual BOOL Open(CONST CHAR * filename, CONST CHAR * archive = NULL);
	virtual VOID Format(DWORD wordspace, DWORD rowwidth);
	INLINE BYTE * GetBuffer() {return m_text;}
	INLINE Charsets * GetEncode() {return m_encode;}
	INLINE VOID SetEncode(Charsets * encode) {m_encode = encode;}
	INLINE DWORD GetRowCount() {return m_rows.size();}
	INLINE CONST CHAR * GetRow(INT index, INT &count)
	{
		if(index >= 0 && index < (INT)m_rows.size())
		{
			count = m_rows[index].count;
			return m_rows[index].start;
		}
		count = 0;
		return NULL;
	}
	DWORD LocateRow(INT offset);
protected:
	struct _TextRow
	{
		CONST CHAR * start;
		INT count;
	};
	typedef vector<_TextRow> _RowList;
	_RowList m_rows;
	BYTE * m_text;
	DWORD m_len;
	Charsets * m_encode;
};
