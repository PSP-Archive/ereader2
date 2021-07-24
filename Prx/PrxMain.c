/*
 * $Id: PrxMain.c 78 2007-11-06 16:39:35Z soarchin $

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

#include "PrxMain.h"
#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <string.h>

PSP_MODULE_INFO("eReader2_p", 0x1006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

DWORD FindProc(CONST CHAR* szMod, CONST CHAR* szLib, DWORD nid)
{
	struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	VOID *entTab;
	INT entLen;

	pMod = sceKernelFindModuleByName(szMod);

	if (!pMod)
	{
		Kprintf("Cannot find module %s\n", szMod);
		return 0;
	}

	INT i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;

	while(i < entLen)
    {
		INT count;
		INT total;
		UINT *vars;

		entry = (struct SceLibraryEntryTable *) (entTab + i);

        if(entry->libname && !strcmp(entry->libname, szLib))
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;

			if(entry->stubcount > 0)
			{
				for(count = 0; count < entry->stubcount; count++)
				{
					if (vars[count] == nid)
						return vars[count+total];
				}
			}
		}

		i += (entry->len * 4);
	}

	return 0;
}

INT (* scePowerSetClockFrequency2)(INT cpufreq, INT ramfreq, INT busfreq);
INT (* scePowerIsBatteryCharging)(VOID);
VOID (* scePower_driver_A09FC577)(INT);
VOID (* scePower_driver_191A3848)(INT);

VOID er2SetSpeed(INT cpu, INT bus)
{
	STATIC BOOL inited = FALSE;

	if(!inited)
	{
		scePowerSetClockFrequency2 = (VOID *)FindProc("scePower_Service", "scePower", 0x545A7F3C);
		scePowerIsBatteryCharging = (VOID *)FindProc("scePower_Service", "scePower", 0x1E490401);
		scePower_driver_A09FC577 = (VOID *)FindProc("scePower_Service", "scePower_driver", 0xA09FC577);
		scePower_driver_191A3848 = (VOID *)FindProc("scePower_Service", "scePower_driver", 0x191A3848);
		inited = TRUE;
	}

	scePowerSetClockFrequency2(cpu, cpu, bus);
	if(scePowerIsBatteryCharging() != 0)
		return;
	STATIC INT ps1 = 0;
	if(ps1 == 0)
	{
		scePower_driver_A09FC577(1);
		ps1 = 1;
		return;
	}

	STATIC INT ps2 = 0;
	if(ps2 == 0)
	{
		ps1 = 0;
		ps2 = 1;
		return;
	}
	scePower_driver_191A3848(0);
	ps1 = 0;
	ps2 = 0;
}

INT module_start(SceSize args, void *argp)
{
	return 0;
}

INT module_stop()
{
	return 0;
}
