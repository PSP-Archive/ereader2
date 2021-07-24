/*
 * $Id: Wnd.h 74 2007-10-20 10:46:39Z soarchin $

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
#include "Charsets.h"
#include "Image.h"
#include "Skin.h"
#include <vector>
using namespace std;

enum WndType
{
	WT_WND = 0,
	WT_STATIC = 1,
	WT_LISTBOX = 2,
	WT_SCROLLBAR = 10
};

class ScrollBar;

class Wnd
{
public:
	enum WndStyle {
		WS_BORDER = 1,
		WS_VISIBLE = 0x80000000,
	};
	STATIC Wnd * CreateWnd(WndType wtype, Wnd * parent, DWORD style);
	Wnd(Wnd * parent, DWORD style);
	virtual ~Wnd();
	virtual WndType GetType() CONST { return WT_WND; }
	virtual VOID ApplySkin(SkinElem * skin);
	virtual VOID SetStyle(DWORD newstyle);
	virtual DWORD GetStyle() CONST;
	virtual Wnd * SetRect(INT left, INT top, INT right, INT bottom);
	VOID GetRect(INT &left, INT &top, INT &right, INT &bottom);
	VOID GetGlobalRect(INT &left, INT &top, INT &right, INT &bottom);
	INLINE INT GetWidth() {return m_width - 1;}
	INLINE INT GetHeight() {return m_height - 1;}
	INLINE Wnd * SetData(DWORD data) {m_data = data; return this;}
	INLINE DWORD GetData() {return m_data;}
	INT GetClientCount() CONST;
	Wnd * GetClient(INT index) CONST;
	Wnd * GetParent() CONST;
	BOOL AddClient(Wnd * client);
	BOOL DelClient(Wnd * client);
	Wnd * SetZOrder(INT order);
	INT GetZOrder() CONST;
	VOID Refresh(BOOL swapBuf = TRUE, BOOL copy = FALSE);
	VOID Invalidate(BOOL swapBuf = TRUE);
	VOID Repaint(BOOL swapBuf = TRUE);
	Wnd * SetForeColor(DWORD color);
	Wnd * SetBackColor(DWORD color);
	Wnd * SetBackImage(Image * img);
	Wnd * SetCharsets(Charsets * encode);

// for Static
	virtual INT GetItemCount() CONST { return 0; }
	virtual INT GetPageItemCount() CONST { return 0; }
	virtual INT AddItem(CONST CHAR * text, DWORD len = 0x7FFF) { return 0; }
	virtual VOID RemoveItem(INT index) {}
	virtual VOID RemoveAll() {}
	virtual CONST CHAR * GetItem(INT index, INT &len) CONST { return NULL; }
	virtual Wnd * SetItem(INT index, CONST CHAR * text, DWORD len = 0x7FFF)  { return NULL; }
	virtual DWORD GetItemData(INT index) CONST { return 0; }
	virtual Wnd * SetItemData(INT index, DWORD data) { return NULL; }
	virtual INT GetPos() CONST { return 0; }
	virtual Wnd * SetPos(INT newpos) { return NULL; }
	virtual INT GetRowSpace() CONST { return 0; }
	virtual Wnd * SetRowSpace(INT rowspace) { return NULL; }
	virtual INT GetCharSpace() CONST { return 0; }
	virtual Wnd * SetCharSpace(INT charspace) { return NULL; }
	virtual Wnd * GetScrollBar(){ return NULL; }
	virtual Wnd * SetScrollBar(Wnd * sb) { return NULL; }
// for ListBox
	virtual INT GetSel() CONST { return 0; }
	virtual Wnd * SetSel(INT selidx) { return NULL; }
	virtual Wnd * SetSelForeColor(DWORD color) { return NULL; }
	virtual DWORD GetSelForeColor() CONST { return 0; }
	virtual Wnd * SetSelBackColor(DWORD color) { return NULL; }
	virtual DWORD GetSelBackColor() CONST { return 0; }
// for ScrollBar
	virtual Wnd * SetBarWidth(INT width) { return NULL; }
	virtual INT GetBarWidth() CONST { return 0; }
	virtual Wnd * SetRange(INT min, INT max) { return NULL; }
	virtual VOID GetRange(INT &min, INT &max) {}
	virtual Wnd * SetBarImage(Image * img) { return NULL; }
	virtual Wnd * SetBarColor(DWORD color) { return NULL; }
	virtual DWORD GetBarColor() { return 0; }
protected:
	typedef vector<Wnd *> _ClientList;
	_ClientList m_clients;
	DWORD m_style;
	Wnd * m_parent;
	INT m_left, m_top, m_right, m_bottom, m_width, m_height;
	INT m_gleft, m_gtop, m_gright, m_gbottom;
	INT m_zorder;
	DWORD m_forecolor, m_backcolor;
	Charsets * m_encode;
	Image * m_img;
	DWORD m_data;
	virtual VOID Draw();
	virtual VOID Predraw() {}
	virtual VOID Postdraw() {}
	virtual VOID StyleChange(DWORD changed);
};

class Static: public Wnd
{
public:
	enum StaticStyle {
		SS_CENTER = 0x100,
		SS_RIGHT = 0x200,
		SS_COPYTEXT = 0x800
	};
	Static(Wnd * parent, DWORD style, DWORD capacity = 0, DWORD rowspace = 0, DWORD charspace = 0);
	virtual ~Static();
	virtual WndType GetType() CONST { return WT_STATIC; }
	virtual VOID ApplySkin(SkinElem * skin);
	virtual INT GetItemCount() CONST;
	virtual INT GetPageItemCount() CONST;
	virtual INT AddItem(CONST CHAR * text, DWORD len = 0x7FFF);
	virtual VOID RemoveItem(INT index);
	virtual VOID RemoveAll();
	virtual CONST CHAR * GetItem(INT index, INT &len) CONST;
	virtual Wnd * SetItem(INT index, CONST CHAR * text, DWORD len = 0x7FFF);
	virtual DWORD GetItemData(INT index) CONST;
	virtual Wnd * SetItemData(INT index, DWORD data);
	virtual INT GetPos() CONST;
	virtual Wnd * SetPos(INT newpos);
	virtual INT GetRowSpace() CONST;
	virtual Wnd * SetRowSpace(INT rowspace);
	virtual INT GetCharSpace() CONST;
	virtual Wnd * SetCharSpace(INT charspace);
	virtual Wnd * GetScrollBar();
	virtual Wnd * SetScrollBar(Wnd * sb);
protected:
	struct _ListItem
	{
		CHAR * str;
		DWORD len;
		DWORD data;
	};
	vector<_ListItem> m_items;
	INT m_capacity;
	INT m_pos;
	INT m_rowspace;
	INT m_charspace;
	Wnd * m_scrollbar;
	VOID UpdateScrollBar();
	virtual VOID Draw();
};

class ListBox: public Static
{
public:
	ListBox(Wnd * parent, DWORD style, DWORD capacity = 0, DWORD rowspace = 0, DWORD charspace = 0);
	virtual ~ListBox();
	virtual WndType GetType() CONST { return WT_LISTBOX; }
	virtual VOID ApplySkin(SkinElem * skin);
	virtual VOID RemoveAll();
	virtual INT GetSel() CONST;
	virtual Wnd * SetSel(INT selidx);
	virtual Wnd * SetSelForeColor(DWORD color);
	virtual DWORD GetSelForeColor() CONST;
	virtual Wnd * SetSelBackColor(DWORD color);
	virtual DWORD GetSelBackColor() CONST;
protected:
	INT m_selidx;
	DWORD m_selforecolor, m_selbackcolor;
	virtual VOID Draw();
};

class ScrollBar: public Wnd
{
public:
	enum ScrollBarDirection {
		SBS_HORZ = 0x1000
	};
	ScrollBar(Wnd * parent, DWORD style);
	virtual ~ScrollBar();
	virtual WndType GetType() CONST { return WT_SCROLLBAR; }
	virtual VOID ApplySkin(SkinElem * skin);
	virtual Wnd * SetBarWidth(INT width);
	virtual INT GetBarWidth() CONST;
	virtual Wnd * SetRange(INT min, INT max);
	virtual VOID GetRange(INT &min, INT &max);
	virtual Wnd * SetPos(INT pos);
	virtual INT GetPos() CONST;
	virtual Wnd * SetBarImage(Image * img);
	virtual Wnd * SetBarColor(DWORD color);
	virtual DWORD GetBarColor();
protected:
	virtual VOID Draw();
private:
	INT m_barwidth, m_pos, m_min, m_max;
	Image * m_bar;
	DWORD m_barcolor;
};
