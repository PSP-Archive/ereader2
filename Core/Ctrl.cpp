/*
 * $Id: Ctrl.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Ctrl.h"
#include "Thread.h"
#include <pspctrl.h>

Ctrl g_Ctrl;

Ctrl::Ctrl()
{
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	m_lastBtn = 0;
	m_lastTick = 0;
	m_repeatTime = 50000;
	m_delayTime = 200000;
	m_newkey = FALSE;
}

Ctrl::~Ctrl()
{
}

DWORD Ctrl::Read()
{
	SceCtrlData ctl;
	sceCtrlPeekBufferPositive(&ctl,1);
	ctl.Buttons &= 0xF3F9;
	m_lastBtn  = ctl.Buttons;
	m_lastTick = ctl.TimeStamp;
	return (Ctrl::Button)m_lastBtn;
}

DWORD Ctrl::ReadDelay()
{
	SceCtrlData ctl;
	while(TRUE) {
		sceCtrlPeekBufferPositive(&ctl,1);
		ctl.Buttons &= 0xF3F9;
		Thread::Delay(30000);
		if(ctl.Buttons != 0)
		{
			if(ctl.Buttons == m_lastBtn)
			{
				if(m_newkey)
				{
					if(ctl.TimeStamp - m_lastTick >= m_delayTime)
					{
						m_newkey = FALSE;
						break;
					}
				}
				else
				{
					if(ctl.TimeStamp - m_lastTick >= m_repeatTime)
						break;
				}
			}
			else
			{
				m_lastBtn = ctl.Buttons;
				m_newkey = TRUE;
				break;
			}
		}
	}
	m_lastTick = ctl.TimeStamp;
	return (Ctrl::Button)m_lastBtn;
}

DWORD Ctrl::ReadDiff()
{
	SceCtrlData ctl;
	sceCtrlPeekBufferPositive(&ctl,1);
	ctl.Buttons &= 0xF3F9;
	while(ctl.Buttons == m_lastBtn) {
		Thread::Delay(30000);
		sceCtrlPeekBufferPositive(&ctl,1);
		ctl.Buttons &= 0xF3F9;
	}
	m_lastBtn  = ctl.Buttons;
	m_lastTick = ctl.TimeStamp;
	return (Ctrl::Button)ctl.Buttons;
}

VOID Ctrl::ReadAnalog(INT &x, INT &y)
{
	SceCtrlData ctl;
	sceCtrlPeekBufferPositive(&ctl,1);
	x = ((INT)ctl.Lx) - 128;
	y = ((INT)ctl.Ly) - 128;
}

VOID Ctrl::WaitRelease()
{
	if(m_lastBtn == 0)
		return;
	SceCtrlData ctl;
	sceCtrlPeekBufferPositive(&ctl,1);
	while(ctl.Buttons == m_lastBtn) {
		Thread::Delay(30000);
		sceCtrlPeekBufferPositive(&ctl,1);
	} 
}
