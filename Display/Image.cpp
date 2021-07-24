/*
 * $Id: Image.cpp 81 2007-11-08 18:33:18Z soarchin $

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

#include "Image.h"
#include "IORead.h"
#include "Utils.h"
#include <stdlib.h>
#include <malloc.h>
#include "png.h"
#include "zlib.h"

Image::Image()
{
	m_swizzled = FALSE;
	m_Buffer = NULL;
}

Image::~Image()
{
	if(m_Buffer != NULL)
	{
		free(m_Buffer);
		m_Buffer = NULL;
	}
}

VOID Image::Swizzle()
{
	if(m_swizzled || m_Pitch <= 16)
		return;
	PIXEL * out;
	if((out = (PIXEL *)memalign(16, m_Pitch * ((m_Height + 7) & ~0x7))) == NULL)
		return;
	UINT blockx, blocky;
	UINT j;

	UINT width_blocks = m_Pitch / 16;
	UINT height_blocks = (m_Height + 7) / 8;

	UINT src_pitch = (m_Pitch-16)/4;
	UINT src_row = m_Pitch * 8;

	CONST BYTE * ysrc = (CONST BYTE *)m_Buffer;
	DWORD* dst = (DWORD*)out;

	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		CONST BYTE* xsrc = ysrc;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			CONST DWORD* src = (DWORD*)xsrc;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				src += src_pitch;
			}
			xsrc += 16;
		}
		ysrc += src_row;
	}
	free(m_Buffer);
	m_Buffer = out;
	m_swizzled = TRUE;
}

Image * Image::AutoLoad(CONST CHAR * filename, CONST CHAR * archive)
{
	CONST CHAR * strExt = GetFileExt(filename);
	Image * res = NULL;
	if(stricmp(strExt, ".png") == 0)
	{
		res = new PNG;
		if(!res->LoadFile(filename, archive))
		{
			delete res;
			res = NULL;
		}
	}
	return res;
}

PNG::PNG()
{
}

PNG::~PNG()
{
}

STATIC VOID PNGRead(png_structp png, png_bytep buf, png_size_t size)
{
	png_size_t check = ((IOReadStruct *)png->io_ptr)->iorc->Read(buf, size);
	if (check != size)
		png_error(png, "Read Error");
}

BOOL PNG::LoadFile(CONST CHAR * filename, CONST CHAR * archive)
{
	IOReadStruct iors;
	iors.iorc = IOReadBase::AutoOpen(filename, archive);
	if(!iors.iorc->Open(filename, archive))
	{
		delete iors.iorc;
		return FALSE;
	}

	// BEGIN: Read png
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	BYTE sig[8];

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return FALSE;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return FALSE;
	}


	if (setjmp(png_ptr->jmpbuf)) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return FALSE;
	}


	png_set_read_fn(png_ptr, (VOID *)&iors, PNGRead);

	PNGRead(png_ptr, sig, 8);
	if (!png_check_sig(sig, 8))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return FALSE;
	}
	png_set_sig_bytes(png_ptr, 8);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_BGR, NULL);

	m_Width = info_ptr->width;
	m_Height = info_ptr->height;

	if (info_ptr->bit_depth == 16) {
		m_backColor = RGB(info_ptr->background.red >> 8, info_ptr->background.green >> 8, info_ptr->background.blue >> 8);
	} else if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY && info_ptr->bit_depth < 8) {
		if (info_ptr->bit_depth == 1)
			m_backColor = info_ptr->background.gray ? 0xFFFFFFFF : 0;
		else if (info_ptr->bit_depth == 2)
			m_backColor = RGB((255/3) * info_ptr->background.gray, (255/3) * info_ptr->background.gray, (255/3) * info_ptr->background.gray);
		else
			m_backColor = RGB((255/15) * info_ptr->background.gray, (255/15) * info_ptr->background.gray, (255/15) * info_ptr->background.gray);
	} else {
		m_backColor = RGB(info_ptr->background.red, info_ptr->background.green, info_ptr->background.blue);
	}

	m_Pitch = (PIXEL_BYTES * info_ptr->width + 15) & ~0xF;
	if((m_Buffer = (PIXEL *)memalign(16, m_Pitch * ((m_Height + 7) & ~0x7))) == NULL)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return FALSE;
	}

	png_byte **prowtable = info_ptr->row_pointers;
	DWORD x, y;
	BYTE r=0, g=0, b=0, a=0;
	PIXEL * imgdata = m_Buffer;
	switch(info_ptr->color_type){
	case PNG_COLOR_TYPE_GRAY:
		for (y = 0; y < info_ptr->height; y ++){
			png_byte *prow = prowtable[y];
			for (x = 0; x < info_ptr->width; x ++){
				r = g = b = *prow++;
				*imgdata++ = RGB(r,g,b);
			}
			imgdata +=  m_Pitch / PIXEL_BYTES - info_ptr->width;
		}
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		for (y = 0; y < info_ptr->height; y ++){
			png_byte *prow = prowtable[y];
			for (x = 0; x < info_ptr->width; x ++){
				r = g = b = *prow++;
				a = *prow++;
				*imgdata++ = RGBA(r,g,b,a);
			}
			imgdata +=  m_Pitch / PIXEL_BYTES - info_ptr->width;
		}
		break;
	case PNG_COLOR_TYPE_RGB:
		for (y = 0; y < info_ptr->height; y ++){
			png_byte *prow = prowtable[y];
			for (x = 0; x < info_ptr->width; x ++){
				b = *prow++;
				g = *prow++;
				r = *prow++;
				*imgdata++ = RGB(r,g,b);
			}
			imgdata +=  m_Pitch / PIXEL_BYTES - info_ptr->width;
		}
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		for (y = 0; y < info_ptr->height; y ++){
			png_byte *prow = prowtable[y];
			for (x = 0; x < info_ptr->width; x ++){
				b = *prow++;
				g = *prow++;
				r = *prow++;
				a = *prow++;
				*imgdata++ = RGBA(r,g,b,a);
			}
			imgdata +=  m_Pitch / PIXEL_BYTES - info_ptr->width;
		}
		break;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	png_ptr = NULL;
	info_ptr = NULL;
	// END: Read png

	delete iors.iorc;
	Swizzle();
	return TRUE;
}
