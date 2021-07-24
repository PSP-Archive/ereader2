/*
 * $Id: Skin.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Skin.h"
#include <stdlib.h>
#include <string.h>
#include "IORead.h"
#include "Charsets.h"
#include "Utils.h"
#include "Log.h"

SkinElem::SkinElem()
{
	memset(this, 0, sizeof(SkinElem));
}

SkinElem::~SkinElem()
{
}

Skin::Skin(CONST CHAR * name)
{
	if(strlen(name) > 15)
	{
		strncpy(m_name, name, 15);
		m_name[15] = 0;
	}
	else
		strcpy(m_name, name);
	m_skinelems.clear();
}

Skin::~Skin()
{
	m_skinelems.clear();
}

SkinElem * Skin::operator [](CONST CHAR * name)
{
	_SkinElemList::iterator iter = m_skinelems.find(name);
	if(iter == m_skinelems.end())
		return (SkinElem *)NULL;
	return (*iter).second;
}

SkinMgr::SkinMgr()
{
	m_skinlist.clear();
	m_imgs.clear();
}

SkinMgr::~SkinMgr()
{
	Unload();
}

BOOL SkinMgr::Load(CONST CHAR * filename)
{
	Unload();
	typedef map<CONST CHAR *, CONST CHAR *, StrCompare> _StrPair;
	IOReadZip zipfile;
	if(!zipfile.Open("skin.cfg", filename))
	{
		return FALSE;
	}

	INT indef = 0;
	CHAR * line, itext[256], * pdata;
	DWORD size = zipfile.GetSize();
	CHAR * data = new CHAR[size + 1];
	zipfile.Read(data, size);
	data[size] = 0;
	_StrPair define_list;
	Skin * skin = NULL;

	pdata = data;
	for(line = StrProcessLine(pdata); line != NULL; line = StrProcessLine(pdata))
	{
		DWORD idx;
		for(idx = 0; idx < size; idx ++)
		{
			if(line[idx] != ' ' && line[idx] != '\t')
				break;
		}
		CHAR * lstart = line + idx;
		if(lstart[0] == '\'')
			continue;
		if(lstart[0] == '[')
		{
			CHAR * p = strchr(&lstart[1], ']');
			if(p == NULL)
				continue;
			INT r = p - lstart - 1;
			strncpy(itext, &lstart[1], r);
			itext[r] = 0;
			if(stricmp(itext, "define") == 0)
				indef = 1;
			else if(stricmp(itext, "image") == 0)
				indef = 2;
			else
			{
				indef = 0;
				strlwr(itext);
				skin = new Skin(itext);
				m_skinlist[ASC.Dup(itext)] = skin;
			}
		}
		else
		{
			if(indef == 1)
			{
				CHAR * p = strchr(lstart, '=');
				if(p == NULL)
					continue;
				*p = 0;
				CHAR * p2 = p + 1;
				while(*p2 != 0 && *p2 != ' ' && *p2 != '\t' && *p2 != '\'')
					p2 ++;
				*p2 = 0;
				define_list[ASC.Dup(lstart)] = ASC.Dup(p + 1);
			}
			else if(indef == 2)
			{
				CHAR * p = strchr(lstart, '=');
				if(p == NULL)
					continue;
				*p = 0;
				CHAR * p2 = p + 1;
				while(*p2 != 0 && *p2 != ' ' && *p2 != '\t' && *p2 != '\'')
					p2 ++;
				*p2 = 0;
				m_imgs[ASC.Dup(lstart)] = Image::AutoLoad(p + 1, filename);
			}
			else
			{
				CHAR rstr[1024];
				CHAR * ps = lstart, * ts = rstr, * p;
				while((p = strchr(ps, '<')) != NULL)
				{
					strncpy(ts, ps, p - ps);
					ts += (p - ps);
					ps = p + 1;
					p = strchr(ps, '>');
					*p = 0;
					strlwr(ps);
					_StrPair::iterator iter = define_list.find(ps);
					if (iter != define_list.end())
					{
						strcpy(ts, (*iter).second);
						ts += strlen(ts);
					}
					ps = p + 1;
				}
				strcpy(ts, ps);
				UINT i = strcspn(rstr, " \t");
				if(i < strlen(rstr))
				{
					p = &rstr[i];
					*p = 0;
					ts = p + 1;
					while(*ts == ' ' || *ts == '\t')
						ts ++;
					SkinElem * elem = new SkinElem;
					if(strlen(rstr) > 15)
					{
						strncpy(elem->name, rstr, 15);
						elem->name[15] = 0;
					}
					else
						strcpy(elem->name, rstr);
					if(strcmp(elem->name, "desktop") == 0)
					{
						for(i = 0, p = strtok(ts, ","); p != NULL && i < 1; p = strtok(NULL, ","), i ++)
						{
							switch(i)
							{
							case 0:
								if(p[0] == '#')
									elem->BC = ColorStrToPixel(p);
								else if(p[0] == '&')
								{
									elem->BC = 0;
									elem->BI = m_imgs[p + 1];
								}
								break;
							}
						}
						if(i < 1)
							delete elem;
						else
							skin->m_skinelems[ASC.Dup(elem->name)] = elem;
					}
					else
					{
						for(i = 0, p = strtok(ts, ","); p != NULL && i < 9; p = strtok(NULL, ","), i ++)
						{
							switch(i)
							{
							case 0:
								elem->style = atoi(p);
								break;
							case 1:
								elem->X1 = atoi(p);
								break;
							case 2:
								elem->Y1 = atoi(p);
								break;
							case 3:
								elem->X2 = atoi(p);
								break;
							case 4:
								elem->Y2 = atoi(p);
								break;
							case 5:
								if(p[0] == '#')
									elem->FC = ColorStrToPixel(p);
								else if(p[0] == '&')
								{
									elem->FC = 0xFFFFFFFF;
									elem->FI = m_imgs[p + 1];
								}
								break;
							case 6:
								if(p[0] == '#')
									elem->BC = ColorStrToPixel(p);
								else if(p[0] == '&')
								{
									elem->BC = 0;
									elem->BI = m_imgs[p + 1];
								}
								break;
							case 7:
								if(p[0] == '#')
									elem->SFC = ColorStrToPixel(p);
								else
									elem->data1 = atoi(p);
								break;
							case 8:
								if(p[0] == '#')
									elem->SBC = ColorStrToPixel(p);
								else
									elem->data2 = atoi(p);
								break;
							}
						}
						if(i < 4)
							delete elem;
						else
							skin->m_skinelems[ASC.Dup(elem->name)] = elem;
					}
				}
			}
		}
	}

	_StrPair::iterator iter = define_list.begin();
	while(iter != define_list.end())
	{
		delete[] (*iter).first;
		delete[] (*iter).second;
		iter ++;
	}

	return TRUE;
}

VOID SkinMgr::Unload()
{
	{
		_SkinList::iterator iter = m_skinlist.begin();
		while(iter != m_skinlist.end())
		{
			delete[] (*iter).first;
			delete (*iter).second;
			iter ++;
		}
		m_skinlist.clear();
	}
	{
		_ImgList::iterator iter = m_imgs.begin();
		while(iter != m_imgs.end())
		{
			delete[] (*iter).first;
			delete (*iter).second;
			iter ++;
		}
		m_imgs.clear();
	}
}

Skin * SkinMgr::operator [](CONST CHAR * name)
{
	_SkinList::iterator iter = m_skinlist.find(name);
	if(iter == m_skinlist.end())
		return (Skin *)NULL;
	return (*iter).second;
}

SkinElem * SkinMgr::GetElem(CONST CHAR * name, CONST CHAR * sub)
{
	Skin * skin = (*this)[name];
	if(skin == NULL)
		return NULL;
	return (*skin)[sub];
}
