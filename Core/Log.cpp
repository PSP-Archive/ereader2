/*
 * $Id: Log.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#ifdef ENABLE_LOG

#include "Log.h"
#include "App.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pspkernel.h>
#include <psprtc.h>

Log * g_Log;

Log::Log(CONST CHAR * filename, DWORD flag)
{
	m_handle = sceIoOpen(filename, PSP_O_APPEND | PSP_O_RDWR | PSP_O_CREAT, 0777);
	m_flag = flag;
}

Log::~Log()
{
	sceIoClose(m_handle);
}

void Log::LogMsg(DWORD flag, CHAR * fmt, ...)
{
	if((flag & m_flag) == 0)
		return;

	static CHAR logstr[1024];

	pspTime curtime;
	sceRtcGetCurrentClockLocalTime(&curtime);
	sprintf(logstr, "%04d-%02d-%02d %02d:%02d:%02d", curtime.year, curtime.month, curtime.day, curtime.hour, curtime.minutes, curtime.seconds);

	switch(flag)
	{
	case DEBUG:
	    strcat(logstr, "[DEBUG] ");
		break;
	case ERROR:
	    strcat(logstr, "[ERROR] ");
		break;
	default:
	    strcat(logstr, "[FATAL] ");
		break;
	}
	sceIoWrite(m_handle, logstr, strlen(logstr));
	va_list args;
	va_start(args, fmt);
	vsprintf(logstr,fmt,args);
	sceIoWrite(m_handle, logstr, strlen(logstr));
	va_end(args);
	sceIoWrite(m_handle, "\r\n", 2);
}

#endif
