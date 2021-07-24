/*
 * $Id: Ctrl.h 74 2007-10-20 10:46:39Z soarchin $

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

class Ctrl
{
public:
	enum Button {
		SELECT		= 0x00000001,
		START		= 0x00000008,
		UP			= 0x00000010,
		RIGHT		= 0x00000020,
		DOWN		= 0x00000040,
		LEFT		= 0x00000080,
		LTRIGGER	= 0x00000100,
		RTRIGGER	= 0x00000200,
		TRIANGLE	= 0x00001000,
		CIRCLE		= 0x00002000,
		CROSS		= 0x00004000,
		SQUARE		= 0x00008000,
		HOME		= 0x00010000,
		HOLD		= 0x00020000,
		NOTE		= 0x00800000,
		FORWARD	= 0x10000000,
		BACK		= 0x20000000,
		PLAYPAUSE	= 0x40000000,
		ANALOG		= 0x80000000
	};
private:
	UINT m_lastBtn;
	UINT m_lastTick;
	UINT m_repeatTime;
	UINT m_delayTime;
	BOOL m_newkey;
public:
	Ctrl();
	~Ctrl();
	DWORD Read();
	DWORD ReadDelay();
	DWORD ReadDiff();
	VOID ReadAnalog(INT &x, INT &y);
	VOID WaitRelease();
};

extern Ctrl g_Ctrl;
