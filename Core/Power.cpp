/*
 * $Id: Power.cpp 78 2007-11-06 16:39:35Z soarchin $

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

#include "Power.h"
#include "PrxMain.h"
#include <pspkernel.h>
#include <psppower.h>

VOID Power::SetClock(DWORD cpu, DWORD bus)
{
#if _PSP_FW_VERSION == 371
	er2SetSpeed(cpu, bus);
#else
	if(cpu > 222 || bus > 111)
		scePowerSetClockFrequency(cpu, cpu, bus);
	else
	{
		scePowerSetClockFrequency(222, 222, 111);
		scePowerSetBusClockFrequency(bus);
		scePowerSetCpuClockFrequency(cpu);
	}
#endif
}

SINGLE Power::GetCpu()
{
	return scePowerGetCpuClockFrequencyFloat();
}

SINGLE Power::GetBus()
{
	return scePowerGetBusClockFrequencyFloat();
}

INT Power::GetLifePercent()
{
	return scePowerGetBatteryLifePercent();
}

INT Power::GetLifeTime()
{
	return scePowerGetBatteryLifeTime();
}

INT Power::GetTempe()
{
	return scePowerGetBatteryTemp();
}

INT Power::GetVolt()
{
	return scePowerGetBatteryVolt();
}
