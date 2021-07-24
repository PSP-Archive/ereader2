/*
 * $Id: Skin.h 74 2007-10-20 10:46:39Z soarchin $

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
#include "Image.h"
#include <string.h>
#include <map>
using namespace std;

struct SkinElem
{
	SkinElem();
	~SkinElem();
	CHAR name[16];
	DWORD style;
	DWORD X1,Y1,X2,Y2,FC,BC,SFC,SBC,data1,data2;
	Image * FI, * BI;
};

struct StrCompare
{
	BOOL operator () (CONST CHAR * s1, CONST CHAR * s2)
	{
		return (strcmp(s1, s2) < 0);
	}
};

class Skin
{
public:
	Skin(CONST CHAR * name);
	virtual ~Skin();
	CONST CHAR * GetName();
	SkinElem * operator [](CONST CHAR * name);
private:
	friend class SkinMgr;
	typedef map<CONST CHAR *, SkinElem *, StrCompare> _SkinElemList;
	CHAR m_name[16];
	_SkinElemList m_skinelems;
};

class SkinMgr
{
public:
	SkinMgr();
	~SkinMgr();
	BOOL Load(CONST CHAR * filename);
	VOID Unload();
	Skin * operator [](CONST CHAR * name);
	SkinElem * GetElem(CONST CHAR * name, CONST CHAR * sub);
private:
	typedef map<CONST CHAR *, Skin *, StrCompare> _SkinList;
	_SkinList m_skinlist;
	typedef map<CONST CHAR *, Image *, StrCompare> _ImgList;
	_ImgList m_imgs;
};
