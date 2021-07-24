/*
 * $Id: Text.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Text.h"
#include "Display.h"
#include "IORead.h"
#include "Log.h"

Text::Text()
{
	m_rows.clear();
	m_text = NULL;
	m_len = 0;
	m_encode = &GBK;
}

Text::~Text()
{
	m_rows.clear();
	if(m_text != NULL)
	{
		delete m_text;
		m_text = NULL;
	}
}

BOOL Text::Open(CONST CHAR * filename, CONST CHAR * archive)
{
	IOReadBase * ioread = IOReadBase::AutoOpen(filename, archive);
	if(ioread == NULL)
		return FALSE;
	m_len = ioread->GetSize();
	m_text = new BYTE[m_len + 1];
	ioread->Read(m_text, m_len);
	m_text[m_len] = 0;
	delete ioread;
	return TRUE;
}

VOID Text::Format(DWORD wordspace, DWORD rowwidth)
{
	m_rows.clear();
	if(m_text == NULL)
		return;
	if(m_len < 2)
		return;

	CONST CHAR * nstr = (CONST CHAR *)m_text, * cstr = NULL;
	if(*(WORD*)m_text == 0xFEFF)
	{
		m_encode = &UCS;
		nstr += 2;
	}
	else if(*(WORD*)m_text == 0xFFFE)
	{
		m_encode = &UCSBE;
		nstr += 2;
	}
	else if(m_len > 2 && m_text[0] == 0xEF && m_text[1] == 0xBB && m_text[2] == 0xBF)
	{
		m_encode = &UTF8;
		nstr += 3;
	}
	_TextRow row;
	row.start = nstr;
	row.count = 0;
	DWORD width = 0;
	while(nstr != NULL)
	{
		cstr = nstr;
		WORD ch = m_encode->ProcessChar(cstr, nstr);
		if(ch == '\r' || ch == '\n')
		{
			row.count = cstr - row.start;
			m_rows.push_back(row);
			width = 0;
			row.count = 0;
			if(nstr == NULL)
				continue;
			if(ch == '\r')
			{
				CONST CHAR * nnstr;
				ch = m_encode->ProcessChar(nstr, nnstr);
				if(ch == '\n')
				{
					nstr = nnstr;
				}
			}
			row.start = nstr;
			continue;
		}
		width += g_Display.GetCharWidth(ch);
		if(width > rowwidth)
		{
			row.count = cstr - row.start;
			m_rows.push_back(row);
			width = g_Display.GetCharWidth(ch);
			row.start = cstr;
			row.count = 0;
		}
		width += wordspace;
	}
	if(row.start != NULL && cstr > row.start)
	{
		row.count = cstr - row.start;
		m_rows.push_back(row);
	}
}

DWORD Text::LocateRow(INT offset)
{
	DWORD index = 0;
	for(; index < m_rows.size(); index ++)
	{
		if((BYTE *)m_rows[index].start - m_text > offset)
			break;
	}
	return (index > 0) ? index - 1 : 0;
}
