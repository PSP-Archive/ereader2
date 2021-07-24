/*
 * $Id: Wnd.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Wnd.h"
#include <stdlib.h>
#include <string.h>
#include "Display.h"
#include "Log.h"
#include "Utils.h"
#include "Log.h"

#define GET_STYLE(d) ((m_style & (d)) > 0)
#define SET_STYLE(v,d) if(v) m_style |= (d); else m_style &= ~(d);

Wnd * Wnd::CreateWnd(WndType wtype, Wnd * parent, DWORD style)
{
	switch(wtype)
	{
	case WT_STATIC:
		return new Static(parent, style);
	case WT_LISTBOX:
		return new ListBox(parent, style);
	case WT_SCROLLBAR:
		return new ScrollBar(parent, style);
	default:
		return new Wnd(parent, style);
	}
}

Wnd::Wnd(Wnd * parent, DWORD style):m_encode(&UCS),m_img(NULL)
{
	m_forecolor = COLOR_WHITE;
	m_backcolor = 0;
	m_style = style;
	m_parent = parent;
	m_left = -1;
	if(parent == NULL)
	{
		SetRect(0, 0, 479, 271);
	}
	else
	{
		parent->AddClient(this);
		SetRect(0, 0, 0, 0);
	}
	m_clients.clear();
}

Wnd::~Wnd()
{
	if(m_parent != NULL)
		m_parent->DelClient(this);
	for(DWORD i = 0; i < m_clients.size(); i ++)
		delete m_clients[i];
	m_clients.clear();
}

VOID Wnd::SetStyle(DWORD newstyle)
{
	if(m_style != newstyle)
	{
		DWORD stylech = m_style ^ newstyle;
		m_style = newstyle;
		StyleChange(stylech);
	}
}

DWORD Wnd::GetStyle() CONST
{
	return m_style;
}

VOID Wnd::ApplySkin(SkinElem * skin)
{
	if(skin == NULL)
		return;
	if(m_parent != NULL)
	{
		m_style |= skin->style;
		SetRect(skin->X1, skin->Y1, skin->X2 + 1, skin->Y2 + 1);
		SetForeColor(skin->FC);
		SetBackColor(skin->BC);
	}
	else
		SetBackColor(skin->BC | ALPHA_MASK);
	SetBackImage(skin->BI);
}

VOID Wnd::StyleChange(DWORD changed)
{
/*
	if((changed & (WS_BORDER | WS_VISIBLE)) > 0)
	{
	}
*/
}

Wnd * Wnd::SetRect(INT left, INT top, INT right, INT bottom)
{
	if(m_left != left || m_top != top || m_right != right || m_bottom != bottom)
	{
		m_left = left;
		m_top = top;
		m_right = right;
		m_bottom = bottom;
		m_width = m_right - m_left + 1;
		m_height = m_bottom - m_top + 1;
		GetGlobalRect(m_gleft, m_gtop, m_gright, m_gbottom);
	}
	return this;
}

VOID Wnd::GetRect(INT &left, INT &top, INT &right, INT &bottom)
{
	left = m_left;
	top = m_top;
	right = m_right;
	bottom = m_bottom;
}

VOID Wnd::GetGlobalRect(INT &left, INT &top, INT &right, INT &bottom)
{
	if(m_parent != NULL)
	{
		INT l, t, r, b;
		m_parent->GetRect(l, t, r, b);
		left = m_left + l;
		top = m_top + t;
		right = m_right + l;
		bottom = m_bottom + t;
	}
	else
		GetRect(left, top, right, bottom);
}

INT Wnd::GetClientCount() CONST
{
	return m_clients.size();
}

Wnd * Wnd::GetClient(INT index) CONST
{
	return m_clients.at(index);
}

Wnd * Wnd::GetParent() CONST
{
	return m_parent;
}

BOOL Wnd::AddClient(Wnd * client)
{
	m_clients.push_back(client);
	client->m_parent = this;
	m_clients[m_clients.size() - 1]->m_zorder = m_clients.size() - 1;
	return TRUE;
}

BOOL Wnd::DelClient(Wnd * client)
{
	for(DWORD i = 0; i < m_clients.size(); i ++)
		if(m_clients[i] == client)
		{
			m_clients.erase(m_clients.begin() + i);
			return TRUE;
		}
	return FALSE;
}

Wnd * Wnd::SetZOrder(INT order)
{
	if(order < 0)
		order = 0;
	else if(order >= m_parent->GetClientCount())
		order = m_parent->GetClientCount() - 1;
	if(order == m_zorder)
		return this;
	if(order < m_zorder)
	{
		Wnd * orgwnd = m_parent->m_clients[m_zorder];
		for(INT i = order; i < m_zorder; i ++)
		{
			m_parent->m_clients[i + 1] = m_parent->m_clients[i];
			m_parent->m_clients[i + 1]->m_zorder = i + 1;
		}
		m_parent->m_clients[order] = orgwnd;
		m_parent->m_clients[order]->m_zorder = order;
	}
	else
	{
		Wnd * orgwnd = m_parent->m_clients[m_zorder];
		for(INT i = m_zorder; i < order; i ++)
		{
			m_parent->m_clients[i] = m_parent->m_clients[i + 1];
			m_parent->m_clients[i]->m_zorder = i;
		}
		m_parent->m_clients[order] = orgwnd;
		m_parent->m_clients[order]->m_zorder = order;
	}
	return this;
}

INT Wnd::GetZOrder() CONST
{
	return m_zorder;
}

VOID Wnd::Refresh(BOOL swapBuf, BOOL copy)
{
	if(swapBuf)
		g_Display.BeginPaint(copy);
	Repaint(FALSE);
	Invalidate(FALSE);
	if(swapBuf)
		g_Display.EndPaint();
}

VOID Wnd::Invalidate(BOOL swapBuf)
{
	if(swapBuf)
		g_Display.BeginPaint();
	if(m_parent != NULL)
	{
		for(INT i = m_zorder + 1; i < m_parent->GetClientCount(); i ++)
			m_parent->GetClient(i)->Repaint(FALSE);
		m_parent->Invalidate(FALSE);
	}
	if(swapBuf)
		g_Display.EndPaint();
}

VOID Wnd::Repaint(BOOL swapBuf)
{
	if(GET_STYLE(WS_VISIBLE))
	{
		if(swapBuf)
			g_Display.BeginPaint();
		Predraw();
		Draw();
		Postdraw();
		for(DWORD i = 0; i < m_clients.size(); i ++)
			m_clients[i]->Repaint(FALSE);
		if(swapBuf)
			g_Display.EndPaint();
	}
}

VOID Wnd::Draw()
{
	g_Display.SetForeColor(m_forecolor);
	g_Display.SetBackColor(m_backcolor);

	if(GET_STYLE(WS_BORDER))
	{
		g_Display.Rectangle(m_gleft, m_gtop, m_gright, m_gbottom);
		if(m_img == NULL)
			g_Display.FillRect(m_gleft + 1, m_gtop + 1, m_gright - 1, m_gbottom - 1);
		else
			m_img->Draw(m_gleft + 1, m_gtop + 1, m_width - 2, m_height - 2, 0, 0, m_img->GetWidth(), m_img->GetHeight());
	}
	else
	{
		if(m_img == NULL)
			g_Display.FillRect(m_gleft, m_gtop, m_gright, m_gbottom);
		else
			m_img->Draw(m_gleft, m_gtop, m_width, m_height, 0, 0, m_img->GetWidth(), m_img->GetHeight());
	}
}

Wnd * Wnd::SetForeColor(DWORD color)
{
	if(color != m_forecolor)
	{
		m_forecolor = color;
	}
	return this;
}

Wnd * Wnd::SetBackColor(DWORD color)
{
	if(color != m_backcolor)
	{
		m_backcolor = color;
	}
	return this;
}

Wnd * Wnd::SetBackImage(Image * img)
{
	m_img = img;
	return this;
}

Wnd * Wnd::SetCharsets(Charsets * encode)
{
	if(m_encode != encode)
	{
		m_encode = encode;
	}
	return this;
}

Static::Static(Wnd * parent, DWORD style, DWORD capacity, DWORD rowspace, DWORD charspace): Wnd(parent, style)
{
	m_items.clear();
	m_capacity = capacity;
	m_pos = 0;
	m_rowspace = rowspace;
	m_charspace = charspace;
	m_scrollbar = NULL;
}

Static::~Static()
{
	RemoveAll();
}

VOID Static::ApplySkin(SkinElem * skin)
{
	if(skin == NULL)
		return;
	Wnd::ApplySkin(skin);
	m_charspace = skin->data1;
	m_rowspace = skin->data2;
}

INT Static::GetItemCount() CONST
{
	return m_items.size();
}

INT Static::GetPageItemCount() CONST
{
	return (m_height + m_rowspace) / (g_Display.GetFontSize() + m_rowspace);
}

INT Static::AddItem(CONST CHAR * text, DWORD len)
{
	_ListItem li;
	li.data = 0;
	li.len = len;
	if(GET_STYLE(SS_COPYTEXT))
		li.str = m_encode->Dup(text);
	else
		li.str = (CHAR *)text;
	m_items.push_back(li);
	UpdateScrollBar();
	return m_items.size() - 1;
}

VOID Static::RemoveItem(INT index)
{
	if(index < 0 || index >= (INT)m_items.size())
		return;
	if(GET_STYLE(SS_COPYTEXT))
		delete[] m_items[index].str;
	m_items.erase(m_items.begin() + index);
	SetPos(m_pos);
	UpdateScrollBar();
}

VOID Static::RemoveAll()
{
	if(GET_STYLE(SS_COPYTEXT))
		for(DWORD i = 0; i < m_items.size(); i ++)
			delete[] m_items[i].str;
	m_items.clear();
	SetPos(0);
	UpdateScrollBar();
}

CONST CHAR * Static::GetItem(INT index, INT &len) CONST
{
	if(index < 0 || index >= (INT)m_items.size())
		return NULL;
	len = m_items[index].len;
	return m_items[index].str;
}

Wnd * Static::SetItem(INT index, CONST CHAR * text, DWORD len)
{
	if(index < 0 || index >= (INT)m_items.size())
		return this;

	m_items[index].data = 0;
	m_items[index].len = len;
	if(GET_STYLE(SS_COPYTEXT))
	{
		delete[] m_items[index].str;
		m_items[index].str = m_encode->Dup(text);
	}
	else
		m_items[index].str = (CHAR *)text;
	return this;
}

DWORD Static::GetItemData(INT index) CONST
{
	if(index < 0 || index >= (INT)m_items.size())
		return 0;
	return m_items[index].data;
}

Wnd * Static::SetItemData(INT index, DWORD data)
{
	if(index < 0 || index >= (INT)m_items.size())
		return this;
	m_items[index].data = data;
	return this;
}

INT Static::GetPos() CONST
{
	return m_pos;
}

Wnd * Static::SetPos(INT newpos)
{
	if(newpos + m_height - 2 > g_Display.GetFontSize() + ((INT)m_items.size() - 1) * (g_Display.GetFontSize() + m_rowspace))
		newpos = g_Display.GetFontSize() + (m_items.size() - 1) * (g_Display.GetFontSize() + m_rowspace) - m_height + 2;
	if(newpos < 0)
		newpos = 0;
	if(newpos != m_pos)
	{
		m_pos = newpos;
		if(m_scrollbar != NULL)
			m_scrollbar->SetPos(m_pos);
	}
	return this;
}

INT Static::GetRowSpace() CONST
{
	return m_rowspace;
}

Wnd * Static::SetRowSpace(INT rowspace)
{
	if(m_rowspace != rowspace)
	{
		INT idx = (m_pos + m_rowspace) / (g_Display.GetFontSize() + m_rowspace);
		INT ext = m_pos + m_rowspace - (g_Display.GetFontSize() + m_rowspace) * idx;
		m_rowspace = rowspace;
		m_pos = (g_Display.GetFontSize() + m_rowspace) * idx - ext - m_rowspace;
		UpdateScrollBar();
	}
	return this;
}

INT Static::GetCharSpace() CONST
{
	return m_charspace;
}

Wnd * Static::SetCharSpace(INT charspace)
{
	if(m_charspace != charspace)
	{
		m_charspace = charspace;
	}
	return this;
}

Wnd * Static::GetScrollBar()
{
	return m_scrollbar;
}

VOID Static::UpdateScrollBar()
{
	if(m_scrollbar != NULL)
	{
		INT max = g_Display.GetFontSize() + ((INT)m_items.size() - 1) * (g_Display.GetFontSize() + m_rowspace) - m_height + 2;
		if(max <= 0)
			max = 1;
		m_scrollbar->SetRange(0, max);
		m_scrollbar->SetPos(m_pos);
	}
}

Wnd * Static::SetScrollBar(Wnd * sb)
{
	m_scrollbar = sb;
	UpdateScrollBar();
	return this;
}

VOID Static::Draw()
{
	Wnd::Draw();
	INT lidx = (m_pos + m_rowspace) / (m_rowspace + g_Display.GetFontSize());
	INT ftop = m_pos - lidx * (m_rowspace + g_Display.GetFontSize());
	INT ypos = 0;
	if(ftop <= 0)
	{
		ypos = -ftop;
		ftop = 0;
	}
	g_Display.SetForeColor(m_forecolor);
	while(lidx < (INT)m_items.size() && ypos < m_height)
	{
		if(GET_STYLE(SS_CENTER))
		{
			INT w = g_Display.GetStringWidth(m_items[lidx].str, *m_encode, m_charspace, m_items[lidx].len);
			if(w > m_width - 2)
				w = m_width - 2;
			g_Display.PutStringLimit(m_gleft + (m_width - w) / 2, m_gtop + 1 + ypos, m_items[lidx].str, ftop, m_height - ypos - 2, 0, m_gright - m_gleft - 1, *m_encode, m_charspace, m_items[lidx].len);
		}
		else if(GET_STYLE(SS_RIGHT))
		{
			INT w = g_Display.GetStringWidth(m_items[lidx].str, *m_encode, m_charspace, m_items[lidx].len);
			if(w > m_width - 2)
				w = m_width - 2;
			g_Display.PutStringLimit(m_gright - 1 - w, m_gtop + 1 + ypos, m_items[lidx].str, ftop, m_height - ypos - 2, 0, m_gright - m_gleft - 1, *m_encode, m_charspace, m_items[lidx].len);
		}
		else
			g_Display.PutStringLimit(m_gleft + 1, m_gtop + 1 + ypos, m_items[lidx].str, ftop, m_height - ypos - 2, 0, m_gright - m_gleft - 1, *m_encode, m_charspace, m_items[lidx].len);
		ypos = ypos + m_rowspace + g_Display.GetFontSize() - ftop;
		lidx ++;
		ftop = 0;
	}
}

ListBox::ListBox(Wnd * parent, DWORD style, DWORD capacity, DWORD rowspace, DWORD charspace): Static(parent, style, capacity, rowspace, charspace)
{
	m_selforecolor = m_forecolor;
	m_selbackcolor = m_backcolor;
	m_selidx = 0;
}

ListBox::~ListBox()
{
}

VOID ListBox::ApplySkin(SkinElem * skin)
{
	if(skin == NULL)
		return;
	Static::ApplySkin(skin);
	SetSelForeColor(skin->SFC);
	SetSelBackColor(skin->SBC);
}

VOID ListBox::RemoveAll()
{
	SetSel(0);
	Static::RemoveAll();
}

INT ListBox::GetSel() CONST
{
	return m_selidx;
}

Wnd * ListBox::SetSel(INT selidx)
{
	if(selidx == m_selidx)
		return this;
	if(selidx >= (INT)m_items.size())
		m_selidx = m_items.size() - 1;
	else if(selidx < 0)
		m_selidx = 0;
	else
		m_selidx = selidx;
	INT curtop = m_selidx * (g_Display.GetFontSize() + m_rowspace);
	INT curbottom = curtop + g_Display.GetFontSize();
	if(curtop < m_pos)
		SetPos(curtop);
	if(curbottom >= m_pos + m_height - 2)
		SetPos((((curbottom - m_height + m_rowspace + 1) / (g_Display.GetFontSize() + m_rowspace)) + 1) * (g_Display.GetFontSize() + m_rowspace));
	return this;
}

Wnd * ListBox::SetSelForeColor(DWORD color)
{
	m_selforecolor = color;
	return this;
}

DWORD ListBox::GetSelForeColor() CONST
{
	return m_selforecolor;
}

Wnd * ListBox::SetSelBackColor(DWORD color)
{
	m_selbackcolor = color;
	return this;
}

DWORD ListBox::GetSelBackColor() CONST
{
	return m_selbackcolor;
}

VOID ListBox::Draw()
{
	Wnd::Draw();
	INT lidx = (m_pos + m_rowspace) / (m_rowspace + g_Display.GetFontSize());
	INT ftop = m_pos - lidx * (m_rowspace + g_Display.GetFontSize());
	INT ypos = 0;
	if(ftop <= 0)
	{
		ypos = -ftop;
		ftop = 0;
	}
	while(lidx < (INT)m_items.size() && ypos < m_height)
	{
		if(lidx == m_selidx)
		{
			g_Display.SetForeColor(m_selforecolor);
			g_Display.SetBackColor(m_selbackcolor);
			if(m_selbackcolor != m_backcolor)
			{
				INT bottom = m_gtop + 1 + ypos + g_Display.GetFontSize() - ftop;
				if(bottom >= m_gbottom)
					bottom = m_gbottom - 1;
				g_Display.FillRect(m_gleft + 1, m_gtop + 1 + ypos, m_gright - 1, bottom);
			}
		}
		else
		{
			g_Display.SetForeColor(m_forecolor);
			g_Display.SetBackColor(m_backcolor);
		}
		if(GET_STYLE(SS_CENTER))
		{
			INT w = g_Display.GetStringWidth(m_items[lidx].str, *m_encode, m_charspace, m_items[lidx].len); 
			if(w > m_width - 2)
				w = m_width - 2;
			g_Display.PutStringLimit(m_gleft + (m_width - w) / 2, m_gtop + 1 + ypos, m_items[lidx].str, ftop, m_height - ypos - 2, 0, m_gright - m_gleft - 1, *m_encode, m_charspace, m_items[lidx].len);
		}
		else if(GET_STYLE(SS_RIGHT))
		{
			INT w = g_Display.GetStringWidth(m_items[lidx].str, *m_encode, m_charspace, m_items[lidx].len);
			if(w > m_width - 2)
				w = m_width - 2;
			g_Display.PutStringLimit(m_gright - 1 - w, m_gtop + 1 + ypos, m_items[lidx].str, ftop, m_height - ypos - 2, 0, m_gright - m_gleft - 1, *m_encode, m_charspace, m_items[lidx].len);
		}
		else
			g_Display.PutStringLimit(m_gleft + 1, m_gtop + 1 + ypos, m_items[lidx].str, ftop, m_height - ypos - 2, 0, m_gright - m_gleft - 1, *m_encode, m_charspace, m_items[lidx].len);
		ypos = ypos + m_rowspace + g_Display.GetFontSize() - ftop;
		lidx ++;
		ftop = 0;
	}
}

ScrollBar::ScrollBar(Wnd * parent, DWORD style): Wnd(parent, style)
{
	m_bar = NULL;
	m_pos = m_min = 0;
	m_max = 100;
	m_barcolor = m_forecolor;
}

ScrollBar::~ScrollBar()
{
}

VOID ScrollBar::ApplySkin(SkinElem * skin)
{
	Wnd::ApplySkin(skin);
	m_bar = skin->FI;
	m_barcolor = skin->SFC;
	m_barwidth = skin->data2;
}

Wnd * ScrollBar::SetBarWidth(INT width)
{
	if(width < m_height)
		m_width = width;
	else
		width = m_height;
	return this;
}

INT ScrollBar::GetBarWidth() CONST
{
	return m_width;
}

Wnd * ScrollBar::SetRange(INT min, INT max)
{
	m_min = min;
	if(max < min)
		m_max = min;
	else
		m_max = max;
	return this;
}

VOID ScrollBar::GetRange(INT &min, INT &max)
{
	min = m_min;
	max = m_max;
}

Wnd * ScrollBar::SetPos(INT pos)
{
	if(pos < m_min)
		m_pos = m_min;
	else if(pos > m_max)
		m_pos = m_max;
	else
		m_pos = pos;
	return this;
}

INT ScrollBar::GetPos() CONST
{
	return m_pos;
}

Wnd * ScrollBar::SetBarImage(Image * img)
{
	m_bar = img;
	return this;
}

Wnd * ScrollBar::SetBarColor(DWORD color)
{
	m_barcolor = color;
	return this;
}

DWORD ScrollBar::GetBarColor()
{
	return m_barcolor;
}

VOID ScrollBar::Draw()
{
	Wnd::Draw();
	INT l,r,t,b;

	if(GET_STYLE(WS_BORDER))
	{
		if(GET_STYLE(SBS_HORZ))
		{
			t = m_gtop + 1;
			b = m_gbottom - 1;
			l = m_gleft + 1 + (m_gright - m_gleft - 2 - m_barwidth) * (m_pos - m_min) / (m_max - m_min);
			r = l + m_barwidth;
		}
		else
		{
			l = m_gleft + 1;
			r = m_gright - 1;
			t = m_gtop + 1 + (m_gbottom - m_gtop - 2 - m_barwidth) * (m_pos - m_min) / (m_max - m_min);
			b = t + m_barwidth;
		}
	}
	else
	{
		if(GET_STYLE(SBS_HORZ))
		{
			t = m_gtop;
			b = m_gbottom;
			l = m_gleft + (m_gright - m_gleft - m_barwidth) * (m_pos - m_min) / (m_max - m_min);
			r = l + m_barwidth;
		}
		else
		{
			l = m_gleft;
			r = m_gright;
			t = m_gtop + (m_gbottom - m_gtop - m_barwidth) * (m_pos - m_min) / (m_max - m_min);
			b = t + m_barwidth;
		}
	}
	if(m_bar == NULL)
	{
		g_Display.SetBackColor(m_barcolor);
		g_Display.FillRect(l, t, r, b);
	}
	else
		m_bar->Draw(l, t, r - l + 1, b - t + 1, 0, 0, m_img->GetWidth(), m_img->GetHeight());
}
