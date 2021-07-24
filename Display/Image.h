/*
 * $Id: Image.h 74 2007-10-20 10:46:39Z soarchin $

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
#include "Display.h"

class Image
{
public:
	Image();
	virtual ~Image();
	virtual BOOL LoadFile(CONST CHAR * filename, CONST CHAR * archive = 0) = 0;
	INLINE VOID Draw(INT x, INT y, INT w = 0, INT h = 0, INT startx = 0, INT starty = 0, INT ow = 0, INT oh = 0) {g_Display.PutImage(x, y, ((w == 0) ? m_Width : w), ((h == 0) ? m_Height : h), m_Pitch / PIXEL_BYTES, startx, starty, ow, oh, m_Buffer, m_swizzled);}
	INLINE INT GetWidth() {return m_Width;}
	INLINE INT GetHeight() {return m_Height;}
	INLINE BOOL GetSwizzled() {return m_swizzled;}
	INLINE PIXEL * GetScanline(INT scanidx) {if(scanidx >= m_Height) return (PIXEL *)0; else return m_Buffer + m_Pitch * scanidx;}
	VOID Swizzle();
	STATIC Image * AutoLoad(CONST CHAR * filename, CONST CHAR * archive = 0);
protected:
	INT m_Width, m_Height, m_Pitch;
	BOOL m_swizzled;
	PIXEL m_backColor;
	PIXEL * m_Buffer;
};

class PNG: public Image
{
public:
	PNG();
	virtual ~PNG();
	virtual BOOL LoadFile(CONST CHAR * filename, CONST CHAR * archive = 0);
};
