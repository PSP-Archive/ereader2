/*
 * $Id: App.cpp 77 2007-11-05 09:21:58Z soarchin $

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

#include "App.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <psppower.h>
#include "Music.h"
#include "Network.h"
#include "Thread.h"
#include "Filesys.h"
#include "Log.h"

#if _PSP_FW_VERSION < 200
static void exception_handler(PspDebugRegBlock *regs)
{
	pspDebugScreenInit();

	pspDebugScreenSetBackColor(0x00FF0000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenClear();

	pspDebugScreenPrintf("The application has just crashed\n");
	pspDebugScreenPrintf("Please contact aeolusc @ pspchina.com or post comments at www.pspsp.org\n\n");
	pspDebugScreenPrintf("Exception Details:\n");
	pspDebugDumpException(regs);
	pspDebugScreenPrintf("\nThe offending routine may be identified with:\n\n"
		"\tpsp-addr2line -e target.elf -f -C 0x%x 0x%x 0x%x\n",
		(unsigned int)regs->epc, (unsigned int)regs->badvaddr, (unsigned int)regs->r[31]);
	pspDebugScreenPrintf("\nThanks for support!\n");
}

__attribute__ ((constructor))
void loaderInit()
{
	pspKernelSetKernelPC();
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
	pspSdkInstallKernelLoadModulePatch();
	pspDebugInstallErrorHandler(exception_handler);
}
#endif

App * App::m_mainApp = NULL;

INT App::CallbackThread(INT args, VOID * argp)
{
	INT cbid;
	cbid = sceKernelCreateCallback("Exit Callback", nExitCallback, argp);
	sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", nPowerCallback, argp);
	scePowerRegisterCallback(0, cbid);

	Thread::SleepAsCB();
	return 0;
};

INT App::nMainThread(INT args, VOID *argp)
{
	Dir::Init();
	App * thapp = (App *)argp;
#ifdef ENABLE_LOG
	CHAR logpath[256];
	thapp->GetRelativePath("eReader2.log", logpath);
	g_Log = new Log(logpath, Log::ERROR | Log::DEBUG);
#endif
	INT result = thapp->MainThread(0, NULL);
	Thread::Sleep();
	return result;
}

INT App::nExitCallback(INT arg1, INT arg2, VOID *arg)
{
	App * thapp = (App *)arg;
	INT res = 0;
	if(thapp != NULL)
		res = thapp->ExitCallback(arg1, arg2);
	return res;
}

INT App::nPowerCallback(INT arg1, INT arg2, VOID *arg)
{
	App * thapp = (App *)arg;
	INT res = 0;
	if(thapp != NULL)
		res = thapp->PowerCallback(arg1, arg2);
	sceDisplayWaitVblankStart();
	return res;
}

App::App()
{
	m_appDir[0] = 0;
	m_mainApp = this;
}

VOID App::Start(INT argc, CHAR * argv[])
{
	INT thid;
	srand((DWORD)time(NULL));
	MusicManager::Init();
	g_Network.Init();
	if(!Init(argc, argv))
		sceKernelSleepThread();
	thid  = sceKernelCreateThread("Callback Thread", (SceKernelThreadEntry)CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, sizeof(App), this);

	thid = sceKernelCreateThread("Main Thread", (SceKernelThreadEntry)nMainThread, 0x18, 0x40000, PSP_THREAD_ATTR_USER, NULL);
	if(thid < 0)
		sceKernelSleepThread();
	sceKernelStartThread(thid, sizeof(App), this);
    Thread::Sleep();
}

App::~App()
{
}

CONST CHAR * App::GetPath()
{
	if(m_appDir[0] == 0)
	{
		getcwd(m_appDir, 256);
		if(m_appDir[strlen(m_appDir) - 1] != '/')
			strcat(m_appDir, "/");
	}
	return m_appDir;
}

CHAR * App::GetRelativePath(CONST CHAR * path, CHAR * destPath)
{
	strcpy(destPath, GetPath());
	strcat(destPath, path);
	return destPath;
}

VOID App::Exit()
{
	sceKernelDelayThread(1000000);
#ifdef ENABLE_LOG
	delete g_Log;
#endif
//	Dir::Release();
	sceKernelExitGame();
}
