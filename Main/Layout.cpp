/*
 * $Id: Layout.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Layout.h"
#include "Charsets.h"

Layout::Layout(Skin * skin)
{
	m_desktop = new Wnd(NULL, Wnd::WS_VISIBLE);
	m_skin = skin;
	if(m_skin != NULL)
		m_desktop->ApplySkin((*m_skin)["desktop"]);
	m_wndlist.clear();
}

Layout::~Layout()
{
	delete m_desktop;
}

Wnd * Layout::Add(CONST CHAR * name, WndType wtype)
{
	Wnd * wnd = Wnd::CreateWnd(wtype, m_desktop, Wnd::WS_VISIBLE);
	m_wndlist[ASC.Dup(name)] = wnd;
	if(m_skin != NULL)
		wnd->ApplySkin((*m_skin)[name]);
	return wnd;
}

Wnd * Layout::AddScrollBar(CONST CHAR * name, CONST CHAR * parent)
{
	Wnd * wnd = Add(name, WT_SCROLLBAR);
	Wnd * awnd = m_wndlist[parent];
	if(awnd != NULL)
		awnd->SetScrollBar(wnd);
	return wnd;
}

VOID Layout::SetSkin(Skin * skin)
{
	if(skin != NULL)
	{
		m_skin = skin;
		m_desktop->ApplySkin((*m_skin)["desktop"]);
		_WndList::iterator iter = m_wndlist.begin();
		while(iter != m_wndlist.end())
			(*iter).second->ApplySkin((*m_skin)[(*iter).first]);
	}
}

VOID Layout::Show()
{
	m_desktop->Refresh();
}

VOID Layout::RemoveAll()
{
	_WndList::iterator iter = m_wndlist.begin();
	while(iter != m_wndlist.end())
	{
		delete[] (*iter).first;
		delete (*iter).second;
		iter ++;
	}
	m_wndlist.clear();
}

Wnd * Layout::operator [](CONST CHAR * name)
{
	if(m_skin != NULL)
		return m_wndlist[name];
	else
		return NULL;
}
