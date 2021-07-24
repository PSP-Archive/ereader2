/*
 * $Id: Display.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Display.h"
#include "IORead.h"
#include "Log.h"
#include <string.h>
#include <malloc.h>
#include <pspkernel.h>
#include <pspgu.h>

Display g_Display;

#define DISP_RSPAN 0

typedef struct _VertexColor {
	PIXEL color;
	WORD x, y, z;
} VertexColor;

typedef struct _Vertex {
	WORD u, v;
	PIXEL color;
	WORD x, y, z;
} Vertex;

static unsigned int __attribute__((aligned(16))) list[65536];

Display::Display()
{
	m_foreColor = COLOR_WHITE;
	m_backColor = 0;
	m_FontBuffer = (BYTE *)NULL;
	sceGuInit();

	sceGuStart(GU_DIRECT, list);
	VRAMStart = (PIXEL *)0;
	VRAMDisp = (PIXEL *)(FRAME_BUFFER_SIZE + (DWORD)sceGeEdramGetAddr());
	DrawStart = (PIXEL *)((FRAME_BUFFER_SIZE * 2 + (DWORD)sceGeEdramGetAddr()) | 0x40000000);
	DrawPointer = DrawStart;
	sceGuDrawBuffer(PSM_FORMAT, (VOID *)0, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (VOID *)FRAME_BUFFER_SIZE, BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH / 2), 2048 - (SCR_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}

Display::~Display()
{
	FreeFont();
	sceGuTerm();
}

BYTE * Display::AllocTexBuffer(DWORD size)
{
	BYTE * result = (BYTE *)DrawPointer;
	memset(result, 0, size * sizeof(PIXEL));
	DrawPointer += size;
	return result;
}

VOID Display::BeginPaint(BOOL copy)
{
	if(copy)
		memmove(GetVRAMStart(), ((BYTE *)VRAMDisp) + 0x40000000, 512 * 272 * PIXEL_BYTES);
	sceGuStart(GU_DIRECT, list);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xFFFFFFFF);
}

VOID Display::EndPaint()
{
	sceGuFinish();
	sceGuSync(0,0);
	WaitVBlank();
	VRAMDisp = GetVRAMStart();
	VRAMStart = (PIXEL *)sceGuSwapBuffers();
	DrawPointer = DrawStart;
}

BOOL Display::HasZippedFont(CONST CHAR * zipfile, CONST CHAR * filename)
{
	unzFile unzf = unzOpen(zipfile);
	if(unzf == NULL)
		return FALSE;

	if(unzLocateFile(unzf, filename, 0) != UNZ_OK || unzOpenCurrentFile(unzf) != UNZ_OK)
	{
		unzClose(unzf);
		return FALSE;
	}
	unzCloseCurrentFile(unzf);

	unzClose(unzf);
	return TRUE;
}

BOOL Display::HasFont(CONST CHAR * filename)
{
	INT fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
	if(fd < 0)
		return FALSE;
	sceIoClose(fd);

	return TRUE;
}

BOOL Display::LoadZippedFont(CONST CHAR * zipfile, CONST CHAR * filename)
{
	FreeFont();
	IOReadZip zipf;
	if(!zipf.Open(filename, zipfile))
		return FALSE;

	DWORD size = zipf.GetSize();

	if((m_FontBuffer = new BYTE[size]) == NULL)
	{
		FreeFont();
		return FALSE;
	}
	zipf.Read(m_FontBuffer, size);
	m_FontSize = (*(DWORD *)m_FontBuffer) & 0xFFFFFF;
	m_FontBits = (*(DWORD *)m_FontBuffer) >> 24;
	switch(m_FontBits)
	{
	case 1:
		for(DWORD i = 0; i < 4; i ++)
			m_FontAlpha[i << 6] = m_FontAlpha[i << 4] = m_FontAlpha[i << 2] = m_FontAlpha[i] = (i * 85) << 24;
		break;
	case 2:
		for(DWORD i = 0; i < 16; i ++)
			m_FontAlpha[i << 4] = m_FontAlpha[i] = (i * 17) << 24;
		break;
	case 3:
		for(DWORD i = 0; i < 256; i ++)
			m_FontAlpha[i] = i << 24;
		break;
	default:
		m_FontAlpha[0] = 0x00000000;
		m_FontAlpha[0x01] = m_FontAlpha[0x02] = m_FontAlpha[0x04] = m_FontAlpha[0x08] = m_FontAlpha[0x10] = m_FontAlpha[0x20] = m_FontAlpha[0x40] = m_FontAlpha[0x80] = 0xFF000000;
		break;
	}
	for(INT i = 0; i < 65535; i ++)
	{
		m_charwidth[i] = m_FontBuffer[i * 4 + 7];
		m_FontBuffer[i * 4 + 7] = 0;
	}
	m_charwidth[65535] = 0;

	return TRUE;
}

BOOL Display::LoadFont(CONST CHAR * filename)
{
	FreeFont();
	INT size;
	INT fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
	if(fd < 0)
		return FALSE;
	size = sceIoLseek(fd, 0, PSP_SEEK_END);
	if((m_FontBuffer = new BYTE[size]) == NULL)
	{
		sceIoClose(fd);
		return FALSE;
	}
	sceIoLseek(fd, 0, PSP_SEEK_SET);
	sceIoRead(fd, m_FontBuffer, size);
	sceIoClose(fd);
	m_FontSize = (*(DWORD *)m_FontBuffer) & 0xFFFFFF;
	m_FontBits = (*(DWORD *)m_FontBuffer) >> 24;
	switch(m_FontBits)
	{
	case 1:
		for(DWORD i = 0; i < 4; i ++)
			m_FontAlpha[i << 6] = m_FontAlpha[i << 4] = m_FontAlpha[i << 2] = m_FontAlpha[i] = (i * 85) << 24;
		break;
	case 2:
		for(DWORD i = 0; i < 16; i ++)
			m_FontAlpha[i << 4] = m_FontAlpha[i] = (i * 17) << 24;
		break;
	case 3:
		for(DWORD i = 0; i < 256; i ++)
			m_FontAlpha[i] = i << 24;
		break;
	default:
		m_FontAlpha[0] = 0x00000000;
		m_FontAlpha[0x01] = m_FontAlpha[0x02] = m_FontAlpha[0x04] = m_FontAlpha[0x08] = m_FontAlpha[0x10] = m_FontAlpha[0x20] = m_FontAlpha[0x40] = m_FontAlpha[0x80] = 0xFF000000;
		break;
	}
	for(INT i = 0; i < 65535; i ++)
	{
		m_charwidth[i] = m_FontBuffer[i * 4 + 7];
		m_FontBuffer[i * 4 + 7] = 0;
	}
	m_charwidth[65535] = 0;

	return TRUE;
}

VOID Display::FreeFont()
{
	if(m_FontBuffer != NULL)
	{
		delete[] m_FontBuffer;
		m_FontBuffer = NULL;
	}
}

VOID Display::GetImage(INT x, INT y, INT w, INT h, PIXEL * buf)
{
	PIXEL * lines = GetVRAMStart() + x + (y << 9), * linesend = lines + (MIN(272 - y, h) << 9);
	DWORD rw = MIN(512 - x, w) * PIXEL_BYTES;
	for(;lines < linesend; lines += 512)
	{
		memcpy(buf, lines, rw);
		buf += w;
	}
}

VOID Display::PutImage(INT x, INT y, INT w, INT h, INT bufw, INT startx, INT starty, INT ow, INT oh, PIXEL * buf, BOOL swizzled)
{
	Vertex* vertices = (Vertex*)sceGuGetMemory(2 * sizeof(Vertex));
	sceGuTexMode(PSM_FORMAT, 0, 0, swizzled ? 1 : 0);
	sceGuTexImage(0, 512, 512, bufw, buf);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	vertices[0].u = startx;
	vertices[0].v = starty;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[0].color = 0;
	vertices[1].u = startx + ((ow == 0) ? w : ow);
	vertices[1].v = starty + ((oh == 0) ? h : oh);
	vertices[1].x = x + w;
	vertices[1].y = y + h;
	vertices[1].z = 0;
	vertices[1].color = 0;
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | COLOR_FORMAT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
}

static BYTE FontMask[4] = {0x80, 0xC0, 0xF0, 0xFF};
static DWORD FontShift[4] = {1, 2, 4, 8};

VOID Display::PutStringLimit(INT x, INT y, CONST CHAR *ostr, INT top, INT bottom, INT left, INT right, Charsets& Encode, DWORD wordspace, INT count)
{
	if(bottom >= m_FontSize)
		bottom = m_FontSize - 1;
	DWORD pixelcolor = m_foreColor & 0x00FFFFFF;
	Vertex* vertices = (Vertex*)sceGuGetMemory(2 * sizeof(Vertex));
	INT height = bottom - top + 1;
	INT blocksize = ((right - left) + m_FontSize + 15) & ~15;
	PIXEL * draw = (PIXEL *)AllocTexBuffer(blocksize * height);
	UINT width = 0;
	CONST CHAR * str = ostr;
	WORD oc = Encode.ProcessChar(str);
	while(str != NULL && str - ostr <= count && (INT)width < right)
	{
		PIXEL * vpoint = draw + width;
		DWORD off = ((DWORD *)m_FontBuffer)[1 + oc];
		if(off == 0)
		{
			oc = Encode.ProcessChar(str);
			continue;
		}
		DWORD w = m_charwidth[oc];
		DWORD mr = FontShift[m_FontBits] * w * top;
		CONST BYTE * ccur = m_FontBuffer + off + mr / 8;
		BYTE b = FontMask[m_FontBits] >> (mr % 8);
		for(INT j = top; j <= bottom; j ++)
		{
			for(DWORD i = 0; i < w; i ++)
			{
				BYTE pi;
				if ((pi = (*ccur) & b) != 0)
					* vpoint = pixelcolor | m_FontAlpha[pi];
				vpoint ++;
				b >>= FontShift[m_FontBits];
				if(b == 0)
				{
					b = FontMask[m_FontBits];
					ccur ++;
				}
			}
			vpoint = vpoint - w + blocksize;
		}
		width += w + wordspace;
		oc = Encode.ProcessChar(str);
	}
	if((INT)width > right)
		width = right;
	sceKernelDcacheWritebackAll();
	sceGuTexMode(PSM_FORMAT, 0, 0, 0);
	sceGuTexImage(0, 512, 512, blocksize, draw);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	vertices[0].u = left;
	vertices[0].v = 0;
	vertices[0].x = x + left;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[0].color = 0;
	vertices[1].u = width;
	vertices[1].v = height;
	vertices[1].x = x + width;
	vertices[1].y = y + height;
	vertices[1].z = 0;
	vertices[1].color = 0;
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | COLOR_FORMAT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
}

VOID Display::PutStringVertLimit(INT x, INT y, CONST CHAR *ostr, INT top, INT bottom, INT left, INT right, Charsets& Encode, DWORD wordspace, INT count)
{
	if(bottom >= m_FontSize)
		bottom = m_FontSize - 1;
	DWORD pixelcolor = m_foreColor & 0x00FFFFFF;
	Vertex* vertices = (Vertex*)sceGuGetMemory(4 * sizeof(Vertex));
	INT height = bottom - top + 1;
	INT blocksize = ((right - left) + m_FontSize + 15) & ~15;
	PIXEL * draw = (PIXEL *)AllocTexBuffer(blocksize * height);
	UINT width = 0;
	CONST CHAR * str = ostr;
	WORD oc = Encode.ProcessChar(str);
	while(str != NULL && str - ostr <= count && (INT)width < right)
	{
		PIXEL * vpoint = draw + width;
		DWORD off = ((DWORD *)m_FontBuffer)[1 + oc];
		if(off == 0)
		{
			oc = Encode.ProcessChar(str);
			continue;
		}
		DWORD w = m_charwidth[oc];
		DWORD mr = FontShift[m_FontBits] * w * top;
		CONST BYTE * ccur = m_FontBuffer + off + mr / 8;
		BYTE b = FontMask[m_FontBits] >> (mr % 8);
		for(INT j = top; j <= bottom; j ++)
		{
			for(DWORD i = 0; i < w; i ++)
			{
				BYTE pi;
				if ((pi = (*ccur) & b) != 0)
					* vpoint = pixelcolor | m_FontAlpha[pi];
				vpoint ++;
				b >>= FontShift[m_FontBits];
				if(b == 0)
				{
					b = FontMask[m_FontBits];
					ccur ++;
				}
			}
			vpoint = vpoint - w + blocksize;
		}
		width += w + wordspace;
		oc = Encode.ProcessChar(str);
	}
	if((INT)width > right)
		width = right;
	sceKernelDcacheWritebackAll();
	sceGuTexMode(PSM_FORMAT, 0, 0, 0);
	sceGuTexImage(0, 512, 512, blocksize, draw);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	vertices[0].u = left;
	vertices[0].v = 0;
	vertices[0].x = x;
	vertices[0].y = y + left;
	vertices[0].z = 0;
	vertices[0].color = 0;
	vertices[1].u = width;
	vertices[1].v = 0;
	vertices[1].x = x;
	vertices[1].y = y + width;
	vertices[1].z = 0;
	vertices[1].color = 0;
	vertices[2].u = left;
	vertices[2].v = height;
	vertices[2].x = x - height;
	vertices[2].y = y + left;
	vertices[2].z = 0;
	vertices[2].color = 0;
	vertices[3].u = width;
	vertices[3].v = height;
	vertices[3].x = x - height;
	vertices[3].y = y + width;
	vertices[3].z = 0;
	vertices[3].color = 0;
	sceGuDrawArray(GU_TRIANGLE_STRIP, GU_TEXTURE_16BIT | COLOR_FORMAT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 4, 0, vertices);
}

DWORD Display::GetStringWidth(CONST CHAR *ostr, Charsets& Encode, DWORD wordspace, INT count)
{
	DWORD width = 0;
	CONST CHAR * str = ostr;
	WORD oc = Encode.ProcessChar(str);
	while(str != NULL && str - ostr < count)
	{
		DWORD w = m_charwidth[oc];
		if(width > 0)
			width += wordspace + w;
		else
			width += w;
		oc = Encode.ProcessChar(str);
	}
	return width;
}

VOID Display::FillVRAM()
{
	sceGuClearColor(m_backColor);
	sceGuClear(GU_COLOR_BUFFER_BIT);
}

VOID Display::FillRect(INT x1, INT y1, INT x2, INT y2)
{
	if((m_backColor & ALPHA_MASK) == 0)
		return;
	VertexColor * vertices = (VertexColor*)sceGuGetMemory(2 * sizeof(VertexColor));

	vertices[0].color = m_backColor;
	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = 0;

	vertices[1].color = m_backColor;
	vertices[1].x = x2 + 1;
	vertices[1].y = y2 + 1;
	vertices[1].z = 0;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuDrawArray(GU_SPRITES, COLOR_FORMAT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

VOID Display::PutPixel(INT x, INT y)
{
	VertexColor * vertices = (VertexColor *)sceGuGetMemory(sizeof(VertexColor));
	vertices[0].color = m_foreColor;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	sceGuDrawArray(GU_POINTS, COLOR_FORMAT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 1, 0, vertices);
}

VOID Display::Rectangle(INT x1, INT y1, INT x2, INT y2)
{
	VertexColor * vertices = (VertexColor *)sceGuGetMemory(5 * sizeof(VertexColor));

	vertices[0].color = m_foreColor;
	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = 0;

	vertices[1].color = m_foreColor;
	vertices[1].x = x2; 
	vertices[1].y = y1; 
	vertices[1].z = 0;

	vertices[2].color = m_foreColor;
	vertices[2].x = x2; 
	vertices[2].y = y2; 
	vertices[2].z = 0;

	vertices[3].color = m_foreColor;
	vertices[3].x = x1; 
	vertices[3].y = y2; 
	vertices[3].z = 0;

	vertices[4].color = m_foreColor;
	vertices[4].x = x1;
	vertices[4].y = y1;
	vertices[4].z = 0;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuDrawArray(GU_LINE_STRIP, COLOR_FORMAT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 5, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

VOID Display::Line(INT x1, INT y1, INT x2, INT y2)
{
	VertexColor * vertices = (VertexColor *)sceGuGetMemory(2 * sizeof(VertexColor));

	vertices[0].color = m_foreColor;
	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = 0;

	vertices[1].color = m_foreColor;
	vertices[1].x = x2; 
	vertices[1].y = y2; 
	vertices[1].z = 0;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuDrawArray(GU_LINES, COLOR_FORMAT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

PIXEL * Display::GetVRAMStart() CONST
{
	return (PIXEL *)((BYTE *)VRAMStart + (DWORD)sceGeEdramGetAddr());
}
