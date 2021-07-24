/*
 * $Id: eReader2.cpp 78 2007-11-06 16:39:35Z soarchin $

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

#include <stdio.h>
#include <string.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <psppower.h>
#include "App.h"
#include "Ctrl.h"
#include "Layout.h"
#include "Music.h"
#include "Input.h"
#include "Network.h"
#include "Thread.h"
#include "Wnd.h"
#include "Skin.h"
#include "Filesys.h"
#include "Text.h"
#include "Bookmark.h"
#include "Database.h"
#include "Power.h"
#include "Utils.h"
#include "Log.h"
#include "Version.h"

APP_GLOBAL(SkinMgr *, g_skinmgr)
APP_GLOBAL(Bookmark *, g_bookmark)
APP_GLOBAL(MusicManager *, g_musicman)
APP_GLOBAL(Database *, g_database)

APP_START(eReader2, 0, 1)
	CHAR g_path[256];
	virtual BOOL Init(INT argc, CHAR * argv[])
	{
#if _PSP_FW_VERSION == 371
		pspSdkLoadStartModule("eReader2_p.prx", PSP_MEMORY_PARTITION_KERNEL);
#endif
		return TRUE;
	}
	VOID IoInit(BOOL firstRun = TRUE)
	{
		CHAR path[256];

		// Initialize database
		Database * g_database = new Database(GetRelativePath("Data/eReader2.db", path));
		// Initialize bookmark
		g_bookmark = new Bookmark();
		g_bookmark->Init(g_database);
		if(!firstRun)
		{
			Dir::PowerUp();
			g_musicman->PowerUp();
		}
	}
	VOID IoRelease(BOOL onExit = TRUE)
	{
		if(!onExit)
		{
			g_musicman->PowerDown();
			sceKernelDelayThread(1000000);
			Dir::PowerDown();
		}
		delete g_bookmark;
		delete g_database;
	}
	virtual INT MainThread(INT args, VOID *argp)
	{
		CHAR path[256];

		sceIoMkdir(GetRelativePath("Data", path), 0777);

		IoInit();

		// Load font
		if(!g_Display.LoadZippedFont(GetRelativePath("fonts.zip", path), "FONT"))
			Exit();

		// Load music list
		g_musicman = new MusicManager();
		g_musicman->AddDir("fatms:/PSP/MUSIC/");
		g_musicman->Start();

		// Load skin
		g_skinmgr = new SkinMgr();
		g_skinmgr->Load(GetRelativePath("Skin/Default.zip", path));
		strcpy(g_path, "fatms:/");

		Power::SetClock(33, 16);
		FileList();

		return 0;
	}
	virtual INT ExitCallback(INT arg1, INT arg2)
	{
//		delete g_musicman;
		delete g_skinmgr;
		IoRelease();
		Exit();
		return 0;
	}
	virtual INT PowerCallback(INT arg1, INT arg2)
	{
		if (arg2 & PSP_POWER_CB_POWER_SWITCH || arg2 & PSP_POWER_CB_SUSPENDING || arg2 & PSP_POWER_CB_STANDBY) {
			Power::SetClock(222, 111);
			sceKernelDelayThread(1000000);
			IoRelease(FALSE);
		} else if (arg2 & PSP_POWER_CB_RESUME_COMPLETE) {
			sceKernelDelayThread(2000000);
			IoInit(FALSE);
			Power::SetClock(33, 16);
		}
		return 0;
	}

	enum FileType {
		UNKNOWN,
		TEXT,
		HTML,
		PNG
	};
	struct _FileTypeTable {
		CONST CHAR *ext;
		FileType type;
	};
	FileType GetFileType(CONST CHAR * filename)
	{
		STATIC _FileTypeTable _fttable[] = {
			{".txt", TEXT},
			{".html", HTML},
			{".htm", HTML},
			{".png", PNG},
			{NULL, UNKNOWN}
		};
		CONST CHAR * strExt = GetFileExt(filename);
		_FileTypeTable * entry;
		for(entry = _fttable; entry->ext != NULL; entry ++)
			if(stricmp(entry->ext, strExt) == 0)
				return entry->type;
		return UNKNOWN;
	}

	VOID FileList()
	{
		Layout l((*g_skinmgr)["file"]);
		Wnd * info = l.Add("info", WT_STATIC);
		info->SetCharsets(&GBK);
		CHAR s[100];
		sprintf(s, "%s %.1f/%.1f", VERSION, Power::GetCpu(), Power::GetBus());
		info->AddItem(s);
		Wnd * list = l.Add("list", WT_LISTBOX);
		list->SetStyle(list->GetStyle() | Static::SS_COPYTEXT);
		l.AddScrollBar("scrollbar", "list");
		CHAR lastname[256];
		lastname[0] = 0;
		while(TRUE)
		{
			Dir dir;
			FileInfo * info;
			DWORD count = dir.ReadDir(g_path, info);
			list->RemoveAll();
			INT idx = 0;
			for(DWORD i = 0; i < count; i ++)
				if(info[i].shortname[0] != '.' || info[i].shortname[1] == '.')
				{
					INT itemidx;
					if(info[i].attr & 0x10)
					{
						WORD dname[256];
						dname[0] = '[';
						dname[1] = 0;
						UCS.Cat((CHAR *)dname, (CONST CHAR *)info[i].filename);
						UCS.Cat((CHAR *)dname, "]\0\0\0");
						itemidx = list->AddItem((CONST CHAR *)dname);
					}
					else
						itemidx = list->AddItem((CONST CHAR *)info[i].filename);
					list->SetItemData(itemidx, i);
					if(stricmp(info[i].shortname, lastname) == 0)
					{
						list->SetSel(itemidx);
						idx = itemidx;
					}
				}
			BOOL redraw = TRUE;
			while(TRUE)
			{
				if(redraw)
				{
					l.Show();
					redraw = FALSE;
				}
				DWORD key = g_Ctrl.ReadDelay();
				if(key == Ctrl::START)
					g_musicman->Pause();
				if(key == Ctrl::UP)
				{
					if(idx > 0) idx --; else idx = list->GetItemCount() - 1;
				}
				if(key == Ctrl::DOWN)
				{
					if(idx < list->GetItemCount() - 1) idx ++; else idx = 0;
				}
				if(key == Ctrl::LTRIGGER)
					idx = 0;
				if(key == Ctrl::RTRIGGER)
					idx = list->GetItemCount() - 1;
				if(key == Ctrl::LEFT)
				{
					if(idx > 0)
						idx -= list->GetPageItemCount();
					if(idx < 0)
						idx = 0;
				}
				if(key == Ctrl::RIGHT)
				{
					if(idx < list->GetItemCount() - 1)
						idx += list->GetPageItemCount();
					if(idx >= list->GetItemCount())
						idx = list->GetItemCount() - 1;
				}
				if(key == Ctrl::CROSS)
				{
					idx = 0;
					DWORD i = list->GetItemData(idx);
					if(strcmp(info[i].shortname, "..") == 0)
						key = Ctrl::CIRCLE;
				}
				if(key == Ctrl::CIRCLE)
				{
					DWORD i = list->GetItemData(idx);
					if(info[i].attr & 0x10)
					{
						if(strcmp(info[i].shortname, "..") == 0)
						{
							g_path[strlen(g_path) - 1] = 0;
							CHAR * p = strrchr(g_path, '/');
							if(p != NULL)
							{
								strcpy(lastname, p + 1);
								p[1] = 0;
								break;
							}
						}
						else
						{
							strcat(g_path, info[i].shortname);
							strcat(g_path, "/");
							lastname[0] = 0;
							break;
						}
					}
					else
					{
						CHAR filename[256];
						strcpy(filename, g_path);
						strcat(filename, info[i].shortname);
						switch(GetFileType(info[i].shortname))
						{
						case TEXT:
							ReadText(filename);
							redraw = TRUE;
							break;
						default:;
						}
					}
				}
				if(idx != list->GetSel())
				{
					list->SetSel(idx);
					idx = list->GetSel();
					redraw = TRUE;
				}
			}
			if(count > 0)
				delete[] info;
		}
	}
	INT ReadText(CONST CHAR * filename)
	{
		Layout l((*g_skinmgr)["text"]);
		Wnd * list = l.Add("main", WT_STATIC);
		l.AddScrollBar("scrollbar", "main");
		Text text;
		text.Open(filename);
		text.Format(0, list->GetWidth());
		list->SetCharsets(text.GetEncode());
		for(DWORD i = 0; i < text.GetRowCount(); i ++)
		{
			INT size;
			CONST CHAR * t = text.GetRow(i, size);
			list->AddItem(t, size);
		}
		list->SetPos(text.LocateRow(g_bookmark->Load(filename)) * (g_Display.GetFontSize() + list->GetRowSpace()));
		BOOL redraw = TRUE;
		INT lastpos = list->GetPos();
		while(TRUE)
		{
			if(redraw)
			{
				l.Show();
				redraw = FALSE;
			}
			DWORD key = g_Ctrl.ReadDelay();
			if(key == Ctrl::START)
				g_musicman->Pause();
			if(key == Ctrl::UP)
				list->SetPos(list->GetPos() - g_Display.GetFontSize() - list->GetRowSpace());
			if(key == Ctrl::DOWN)
				list->SetPos(list->GetPos() + g_Display.GetFontSize() + list->GetRowSpace());
			if(key == Ctrl::LEFT || key == Ctrl::LTRIGGER)
				list->SetPos(list->GetPos() - list->GetPageItemCount() * (g_Display.GetFontSize() + list->GetRowSpace()));
			if(key == Ctrl::RIGHT || key == Ctrl::RTRIGGER)
				list->SetPos(list->GetPos() + list->GetPageItemCount() * (g_Display.GetFontSize() + list->GetRowSpace()));
			if(key == Ctrl::CROSS)
				break;
			if(lastpos != list->GetPos())
			{
				lastpos = list->GetPos();
				redraw = TRUE;
			}
		}
		INT count;
		CONST CHAR * start = text.GetRow(list->GetPos() / (g_Display.GetFontSize() + list->GetRowSpace()), count);
		if(start != NULL)
			g_bookmark->Save(filename, (BYTE *)start - text.GetBuffer());
		return 0;
	}
APP_END(eReader2)
