/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_softbackend.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_gfxbase.h>



namespace wg
{
	const TypeInfo SoftBackend::TYPEINFO = { "SoftBackend", &GfxBackend::TYPEINFO };

	int		SoftBackend::s_lineThicknessTable[17];

	int 	SoftBackend::s_mulTab[256];

	int16_t	SoftBackend::s_limit4096Tab[4097 * 3];

	bool	SoftBackend::s_bTablesInitialized = false;

	const int16_t SoftBackend::s_channel_4_1[256] = { 0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
											0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15 };

	const int16_t SoftBackend::s_channel_4_2[256] = { 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15,
											4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15, 4096 * 1 / 15,
											4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15, 4096 * 2 / 15,
											4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15, 4096 * 3 / 15,
											4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15, 4096 * 4 / 15,
											4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15, 4096 * 5 / 15,
											4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15, 4096 * 6 / 15,
											4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15, 4096 * 7 / 15,
											4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15, 4096 * 8 / 15,
											4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15, 4096 * 9 / 15,
											4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15, 4096 * 10 / 15,
											4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15, 4096 * 11 / 15,
											4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15, 4096 * 12 / 15,
											4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15, 4096 * 13 / 15,
											4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15, 4096 * 14 / 15,
											4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15, 4096 * 15 / 15 };


	const int16_t SoftBackend::s_channel_5[32] = { 4096 * 0 / 31, 4096 * 1 / 31, 4096 * 2 / 31, 4096 * 3 / 31, 4096 * 4 / 31, 4096 * 5 / 31, 4096 * 6 / 31, 4096 * 7 / 31, 4096 * 8 / 31, 4096 * 9 / 31, 4096 * 10 / 31, 4096 * 11 / 31, 4096 * 12 / 31, 4096 * 13 / 31, 4096 * 14 / 31, 4096 * 15 / 31,
											4096 * 16 / 31, 4096 * 17 / 31, 4096 * 18 / 31, 4096 * 19 / 31, 4096 * 20 / 31, 4096 * 21 / 31, 4096 * 22 / 31, 4096 * 23 / 31, 4096 * 24 / 31, 4096 * 25 / 31, 4096 * 26 / 31, 4096 * 27 / 31, 4096 * 28 / 31, 4096 * 29 / 31, 4096 * 30 / 31, 4096 * 31 / 31 };

	const int16_t SoftBackend::s_channel_6[64] = { 4096 * 0 / 63, 4096 * 1 / 63, 4096 * 2 / 63, 4096 * 3 / 63, 4096 * 4 / 63, 4096 * 5 / 63, 4096 * 6 / 63, 4096 * 7 / 63, 4096 * 8 / 63, 4096 * 9 / 63, 4096 * 10 / 63, 4096 * 11 / 63, 4096 * 12 / 63, 4096 * 13 / 63, 4096 * 14 / 63, 4096 * 15 / 63,
											4096 * 16 / 63, 4096 * 17 / 63, 4096 * 18 / 63, 4096 * 19 / 63, 4096 * 20 / 63, 4096 * 21 / 63, 4096 * 22 / 63, 4096 * 23 / 63, 4096 * 24 / 63, 4096 * 25 / 63, 4096 * 26 / 63, 4096 * 27 / 63, 4096 * 28 / 63, 4096 * 29 / 63, 4096 * 30 / 63, 4096 * 31 / 63,
											4096 * 32 / 63, 4096 * 33 / 63, 4096 * 34 / 63, 4096 * 35 / 63, 4096 * 36 / 63, 4096 * 37 / 63, 4096 * 38 / 63, 4096 * 39 / 63, 4096 * 40 / 63, 4096 * 41 / 63, 4096 * 42 / 63, 4096 * 43 / 63, 4096 * 44 / 63, 4096 * 45 / 63, 4096 * 46 / 63, 4096 * 47 / 63,
											4096 * 48 / 63, 4096 * 49 / 63, 4096 * 50 / 63, 4096 * 51 / 63, 4096 * 52 / 63, 4096 * 53 / 63, 4096 * 54 / 63, 4096 * 55 / 63, 4096 * 56 / 63, 4096 * 57 / 63, 4096 * 58 / 63, 4096 * 59 / 63, 4096 * 60 / 63, 4096 * 61 / 63, 4096 * 62 / 63, 4096 * 63 / 63 };



	const uint8_t SoftBackend::s_fast8_channel_4_1[256] = { 0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
											0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

	const uint8_t SoftBackend::s_fast8_channel_4_2[256] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
											0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
											0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
											0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
											0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
											0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
											0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
											0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
											0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
											0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
											0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
											0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
											0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
											0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	const uint8_t SoftBackend::s_fast8_channel_5[32] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x29, 0x31, 0x39, 0x41, 0x4a, 0x52, 0x5a, 0x62, 0x6a, 0x73, 0x7b,
											0x83, 0x8b, 0x94, 0x9c, 0xa4, 0xac, 0xb4, 0xbd, 0xc5, 0xcd, 0xd5, 0xde, 0xe6, 0xee, 0xf6, 0xff };

	const uint8_t SoftBackend::s_fast8_channel_6[64] = { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
											0x40, 0x44, 0x48, 0x4c, 0x50, 0x55, 0x59, 0x5d, 0x61, 0x65, 0x69, 0x6d, 0x71, 0x75, 0x79, 0x7d,
											0x81, 0x85, 0x89, 0x8d, 0x91, 0x95, 0x99, 0x9d, 0xa1, 0xa5, 0xaa, 0xae, 0xb2, 0xb6, 0xba, 0xbe,
											0xc2, 0xc6, 0xca, 0xce, 0xd2, 0xd6, 0xda, 0xde, 0xe2, 0xe6, 0xea, 0xee, 0xf2, 0xf6, 0xfa, 0xff };




	//____ create() _____________________________________________

	SoftBackend_p SoftBackend::create()
	{
		return SoftBackend_p(new SoftBackend());
	}

	//____ constructor _____________________________________________

	SoftBackend::SoftBackend()
	{
		_initTables();

		// clear kernel tables

		for (int i = 0; i < PixelFormat_size; i++)
		{
			m_pKernels[i] = nullptr;

			m_pStraightMoveToBGRA8Kernels[i][0] = nullptr;
			m_pStraightMoveToBGRA8Kernels[i][1] = nullptr;

			m_pTransformMoveToBGRA8Kernels[i][0][0] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][0][1] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][0][2] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][1][0] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][1][1] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][1][2] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][2][0] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][2][1] = nullptr;
			m_pTransformMoveToBGRA8Kernels[i][2][2] = nullptr;


			m_pStraightMoveToHiColorKernels[i][0] = nullptr;
			m_pStraightMoveToHiColorKernels[i][1] = nullptr;
			m_pTransformMoveToHiColorKernels[i][0][0] = nullptr;
			m_pTransformMoveToHiColorKernels[i][0][1] = nullptr;
			m_pTransformMoveToHiColorKernels[i][0][2] = nullptr;
			m_pTransformMoveToHiColorKernels[i][1][0] = nullptr;
			m_pTransformMoveToHiColorKernels[i][1][1] = nullptr;
			m_pTransformMoveToHiColorKernels[i][1][2] = nullptr;
			m_pTransformMoveToHiColorKernels[i][2][0] = nullptr;
			m_pTransformMoveToHiColorKernels[i][2][1] = nullptr;
			m_pTransformMoveToHiColorKernels[i][2][2] = nullptr;
		}

	}

	//____ destructor _____________________________________________

	SoftBackend::~SoftBackend()
	{
		for (int i = 0; i < PixelFormat_size; i++)
		{
			if (m_pKernels[i])
			{
				delete m_pKernels[i];
				m_pKernels[i] = nullptr;
			}
		}
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ beginRender() _____________________________________________

	void SoftBackend::beginRender()
	{
	}

	//____ endRender() _____________________________________________

	void SoftBackend::endRender()
	{
		if (m_pCanvas)
		{
			m_pCanvas->freePixelBuffer(m_buffer);
			m_pCanvas = nullptr;
		}
	}

	//____ setCanvas() _____________________________________________

	void SoftBackend::setCanvas(CanvasRef ref)
	{
		auto pInfo = canvasInfo(ref);

		if (!pInfo)
		{
			//TODO: Error handling!

			return;
		}

		setCanvas(pInfo->pSurface);
	}

	void SoftBackend::setCanvas(Surface* _pSurface)
	{
		auto pSurface = dynamic_cast<SoftSurface*>(_pSurface);

		if (m_pCanvas)
		{
			m_pCanvas->freePixelBuffer(m_buffer);
		}

		m_pCanvas = pSurface;

		m_buffer = m_pCanvas->allocPixelBuffer();
		m_pCanvasPixels		= m_buffer.pixels;
		m_canvasPixelFormat = m_buffer.format;
		m_canvasPitch		= m_buffer.pitch;
		m_canvasPixelBits	= m_pCanvas->pixelDescription()->bits;

		// Reset states

		m_blendMode = BlendMode::Blend;
		m_tintMode = TintMode::None;

		m_pBlitSource = nullptr;
		m_bBlitFunctionNeedsUpdate = true;

		m_colTrans.mode = TintMode::None;
		m_colTrans.flatTintColor = HiColor::White;
		m_colTrans.tintRect.clear();
		m_colTrans.pTintAxisX = nullptr;
		m_colTrans.pTintAxisY = nullptr;
		m_colTrans.morphFactor = 2048;
		m_colTrans.fixedBlendColor = HiColor::White;

		_updateBlur(nullptr);
	}



	//____ setObjects() _____________________________________________

	void SoftBackend::setObjects(Object** pBeg, Object** pEnd)
	{
		m_pObjectsBeg = pBeg;
		m_pObjectsEnd = pEnd;
	}

	//____ setCoords() _____________________________________________

	void SoftBackend::setCoords(spx* pBeg, spx* pEnd)
	{
		m_pCoordsBeg = pBeg;
		m_pCoordsEnd = pEnd;
	}

	//____ setTransforms() _____________________________________________

	void SoftBackend::setTransforms(Transform* pBeg, Transform* pEnd)
	{
		m_pTransformsBeg = pBeg;
		m_pTransformsEnd = pEnd;
	}

	//____ processCommands() _____________________________________________

	void SoftBackend::processCommands(int32_t* pBeg, int32_t* pEnd)
	{
		spx *		pCoords = m_pCoordsBeg;
		Transform*	pTransforms = m_pTransformsBeg;


		auto p = pBeg;
		while (p < pEnd)
		{
			auto cmd = Command(*p++);
			switch (cmd)
			{
			case Command::None:
				break;

			case Command::StateChange:
			{
				int32_t statesChanged = *p++;

				if (statesChanged & uint8_t(StateChange::BlitSource))
				{
					int32_t objectOfs = *p++;

					auto pBlitSource = static_cast<SoftSurface*>(m_pObjectsBeg[objectOfs]);

					if (!pBlitSource || !m_pBlitSource || pBlitSource->pixelFormat() != m_pBlitSource->pixelFormat() ||
						pBlitSource->sampleMethod() != m_pBlitSource->sampleMethod())
						m_bBlitFunctionNeedsUpdate = true;

					m_pBlitSource = pBlitSource;
				}

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					m_blendMode = (BlendMode)*p++;

					m_bBlitFunctionNeedsUpdate = true;
				}

				if (statesChanged & uint8_t(StateChange::TintColor))
				{
					m_colTrans.flatTintColor.r = *p++;
					m_colTrans.flatTintColor.g = *p++;
					m_colTrans.flatTintColor.b = *p++;
					m_colTrans.flatTintColor.a = *p++;

					_updateTintMode();
				}

				if (statesChanged & uint8_t(StateChange::TintMap))
				{
					int32_t objectOfs = *p++;
					int32_t	x = *p++ / 64;
					int32_t	y = *p++ / 64;
					int32_t	w = *p++ / 64;
					int32_t	h = *p++ / 64;

					m_colTrans.tintRect = RectI(x, y, w, h);

					auto pTintmap = static_cast<Tintmap*>(m_pObjectsBeg[objectOfs]);

					if( pTintmap->isHorizontal() )
					{
						if( m_tintmapXBufferSize < w )
						{
							delete m_pTintmapXBuffer;
							m_pTintmapXBuffer = new HiColor[w];
							m_tintmapXBufferSize = w;
						}
						m_colTrans.pTintAxisX = m_pTintmapXBuffer;
					}
					else
						m_colTrans.pTintAxisX = nullptr;

					if( pTintmap->isVertical() )
					{
						if( m_tintmapYBufferSize < h )
						{
							delete m_pTintmapYBuffer;
							m_pTintmapYBuffer = new HiColor[h];
							m_tintmapYBufferSize = h;
						}
						m_colTrans.pTintAxisY = m_pTintmapYBuffer;
					}
					else
						m_colTrans.pTintAxisY = nullptr;

					pTintmap->exportAxisColors({w*64,h*64}, m_pTintmapXBuffer, m_pTintmapYBuffer);
					
					_updateTintMode();
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					m_colTrans.morphFactor = *p++;
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
					m_colTrans.fixedBlendColor.r = *p++;
					m_colTrans.fixedBlendColor.g = *p++;
					m_colTrans.fixedBlendColor.b = *p++;
					m_colTrans.fixedBlendColor.a = *p++;
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					int32_t objectOfs = *p++;

					auto pBlurbrush = static_cast<Blurbrush*>(m_pObjectsBeg[objectOfs]);
					
					_updateBlur(pBlurbrush);

					assert(false);
				}

				break;
			}

			case Command::Fill:
			{
				int32_t nRects = *p++;

				int32_t r = *p++;
				int32_t g = *p++;
				int32_t b = *p++;
				int32_t a = *p++;

				HiColor col(r, g, b, a);

				FillOp_p pFunc = nullptr;

				auto pKernels = m_pKernels[(int)m_pCanvas->pixelFormat()];
				if (pKernels)
					pFunc = pKernels->pFillKernels[(int)m_tintMode][(int)m_blendMode];

				if (!pFunc)
				{
					//TODO: Error handling!!!

					break;
				}

				for (int i = 0; i < nRects; i++)
				{
					RectSPX* pPatch = reinterpret_cast<RectSPX*>(pCoords);
					pCoords += 4;

					uint8_t* pDst = m_pCanvasPixels + (pPatch->y/64) *m_canvasPitch + (pPatch->x/64) * m_canvasPixelBits/8;
					pFunc(pDst, m_canvasPixelBits/8, m_canvasPitch - (pPatch->w/64) * (m_canvasPixelBits/8), (pPatch->h/64), (pPatch->w/64), col, m_colTrans, pPatch->pos());
				}
				break;
			}

			case Command::FillSubPixel:
			case Command::Plot:
			case Command::Line:
			case Command::Blur:
			case Command::DrawEdgemap:
			{
				//TODO: Implement!!!	

				assert(false);
				break;
			}

			case Command::Blit:
			{
				if (m_bBlitFunctionNeedsUpdate)
				{
					_updateBlitFunctions();
					m_bBlitFunctionNeedsUpdate = false;
				}

				int32_t nRects = *p++;
				int32_t transform = *p++;

				int32_t srcX = *p++;
				int32_t srcY = *p++;
				int32_t dstX = *p++;
				int32_t dstY = *p++;

				bool bTiling = m_pCanvas->isTiling();

				if (transform <= int(GfxFlip_max) )
				{
					const Transform& mtx = s_blitFlipTransforms[transform];

					// Step forward _src by half a pixel, so we start from correct pixel.

					srcX += (mtx.xx + mtx.yx) * 32;
					srcY += (mtx.xy + mtx.yy) * 32;

					//

					for (int i = 0; i < nRects; i++)
					{
						RectI	patch;

						patch.x = *pCoords++ / 64;
						patch.y = *pCoords++ / 64;
						patch.w = *pCoords++ / 64;
						patch.h = *pCoords++ / 64;

						CoordI src = { srcX / 64, srcY / 64 };
						CoordI dest = { dstX / 64, dstY / 64 };

						CoordI	patchOfs = patch.pos() - dest;


						//

						src.x += patchOfs.x * mtx.xx + patchOfs.y * mtx.yx;
						src.y += patchOfs.x * mtx.xy + patchOfs.y * mtx.yy;

						if (bTiling)
							(this->*m_pStraightTileOp)(patch, src, mtx, patch.pos(), m_pStraightTileFirstPassOp);
						else
							(this->*m_pStraightBlitOp)(patch, src, mtx, patch.pos(), m_pStraightBlitFirstPassOp);
					}
				}
				else
				{

				}




				break;
			}

			default:
				break;
			}
		}
	}

	//____ defineCanvas() _____________________________________________

	bool SoftBackend::defineCanvas(CanvasRef ref, SoftSurface* pSurface)
	{

		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](CanvasInfo& entry) { return (ref == entry.ref); });

		if (it == m_definedCanvases.end())
		{
			if (pSurface)
				m_definedCanvases.push_back(CanvasInfo(ref, pSurface, pSurface->pixelSize() * 64, pSurface->pixelFormat(), pSurface->scale()));
		}
		else
		{
			if (pSurface)
			{
				it->pSurface = pSurface;
				it->size = pSurface->pixelSize() * 64;
				it->scale = pSurface->scale();
				it->format = pSurface->pixelFormat();
			}
			else
			{
				m_definedCanvases.erase(it);
			}
		}

		return true;
	}

	//____ canvasInfo() _____________________________________________

	const CanvasInfo* SoftBackend::canvasInfo(CanvasRef ref) const
	{
		auto it = std::find_if(m_definedCanvases.begin(), m_definedCanvases.end(), [ref](const CanvasInfo& entry) { return (ref == entry.ref); });

		if (it != m_definedCanvases.end())
			return &*it;

		return nullptr;
	}


	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p SoftBackend::surfaceFactory()
	{
		if (!m_pSurfaceFactory)
			m_pSurfaceFactory = SoftSurfaceFactory::create();

		return m_pSurfaceFactory;
	}


	//____ edgemapFactory() ___________________________________________________

	EdgemapFactory_p SoftBackend::edgemapFactory()
	{
		if (!m_pEdgemapFactory)
			m_pEdgemapFactory = SoftEdgemapFactory::create();

		return m_pEdgemapFactory;
	}


	//____ maxEdges() _____________________________________________

	int SoftBackend::maxEdges() const
	{
		return c_maxSegments;
	}

	//____ surfaceType() ______________________________________________________

	const TypeInfo& SoftBackend::surfaceType(void) const
	{
		return SoftSurface::TYPEINFO;
	}


	//____ setFillKernel() ____________________________________________________

	bool SoftBackend::setFillKernel(TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, FillOp_p pKernel)
	{
		if (!_setupDestFormatKernels(destFormat))
			return false;

		m_pKernels[(int)destFormat]->pFillKernels[(int)tintMode][(int)blendMode] = pKernel;
		return true;
	}

	//____ setStraightBlitKernel() ____________________________________________

	bool SoftBackend::setStraightBlitKernel(PixelFormat sourceFormat, SoftBackend::ReadOp readOp, TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, StraightBlitOp_p pKernel)
	{
		bool success = false;

		if (_setupDestFormatKernels(destFormat))
		{
			int singlePassKernelsIdx = m_pKernels[(int)destFormat]->singlePassBlitKernels[(int)sourceFormat];
			if (singlePassKernelsIdx == 0)
			{
				m_singlePassBlitKernels.emplace_back();
				singlePassKernelsIdx = (int)m_singlePassBlitKernels.size();
				m_pKernels[(int)destFormat]->singlePassBlitKernels[(int)sourceFormat] = (uint16_t)singlePassKernelsIdx;
			}
			singlePassKernelsIdx--;

			int straightBlitKernelsIdx = m_singlePassBlitKernels[singlePassKernelsIdx].straightBlitKernels[(int)blendMode];
			if (straightBlitKernelsIdx == 0)
			{
				m_singlePassStraightBlitKernels.emplace_back();
				straightBlitKernelsIdx = (int)m_singlePassStraightBlitKernels.size();
				m_singlePassBlitKernels[singlePassKernelsIdx].straightBlitKernels[(int)blendMode] = straightBlitKernelsIdx;
			}
			straightBlitKernelsIdx--;

			m_singlePassStraightBlitKernels[straightBlitKernelsIdx].pKernels[(int)readOp][(int)tintMode] = pKernel;
			success = true;

			if (sourceFormat == PixelFormat::Undefined && readOp == ReadOp::Normal)
				m_pKernels[(int)destFormat]->pStraightBlitFromHiColorKernels[(int)tintMode][(int)blendMode] = pKernel;

			if (sourceFormat == PixelFormat::BGRA_8_linear && readOp == ReadOp::Normal)
				m_pKernels[(int)destFormat]->pStraightBlitFromBGRA8Kernels[(int)tintMode][(int)blendMode] = pKernel;
		}

		if (destFormat == PixelFormat::Undefined && blendMode == BlendMode::Replace && tintMode == TintMode::None)			// Special case for HiColor destination.
		{
			m_pStraightMoveToHiColorKernels[(int)sourceFormat][(int)readOp] = pKernel;
			success = true;
		}

		if (destFormat == PixelFormat::BGRA_8_linear && blendMode == BlendMode::Replace && tintMode == TintMode::None)		// Special case for HiColor destination.
		{
			m_pStraightMoveToBGRA8Kernels[(int)sourceFormat][(int)readOp] = pKernel;
			success = true;
		}

		return success;
	}

	//____ _onePassStraightBlit() _____________________________________________

	void SoftBackend::_onePassStraightBlit(const RectI& dest, CoordI src, const Transform& mtx, CoordI patchPos, StraightBlitOp_p pPassOneOp)
	{
		const SoftSurface* pSource = m_pBlitSource;

		int srcPixelBytes = pSource->m_pPixelDescription->bits / 8;
		int dstPixelBytes = m_canvasPixelBits / 8;

		Pitches pitches;

		pitches.srcX = srcPixelBytes * mtx.xx + pSource->m_pitch * mtx.xy;
		pitches.dstX = dstPixelBytes;
		pitches.srcY = srcPixelBytes * mtx.yx + pSource->m_pitch * mtx.yy - pitches.srcX * dest.w;
		pitches.dstY = m_canvasPitch - dstPixelBytes * dest.w;

		uint8_t* pDst = m_pCanvasPixels + dest.y * m_canvasPitch + dest.x * dstPixelBytes;
		uint8_t* pSrc = pSource->m_pData + src.y * pSource->m_pitch + src.x * srcPixelBytes;

		pPassOneOp(pSrc, pDst, pSource, pitches, dest.h, dest.w, m_colTrans, patchPos, &mtx);
	}

	//____ _twoPassStraightBlit() _____________________________________________

	void SoftBackend::_twoPassStraightBlit(const RectI& dest, CoordI src, const Transform& mtx, CoordI patchPos, StraightBlitOp_p pPassOneOp)
	{
		SoftSurface* pSource = m_pBlitSource;

		int srcPixelBytes = pSource->m_pPixelDescription->bits / 8;
		int dstPixelBytes = m_canvasPixelBits / 8;

		Pitches pitchesPass1, pitchesPass2;

		pitchesPass1.srcX = srcPixelBytes * mtx.xx + pSource->m_pitch * mtx.xy;
		pitchesPass1.dstX = 8;
		pitchesPass1.srcY = srcPixelBytes * mtx.yx + pSource->m_pitch * mtx.yy - pitchesPass1.srcX * dest.w;
		pitchesPass1.dstY = 0;

		pitchesPass2.srcX = 8;
		pitchesPass2.dstX = dstPixelBytes;
		pitchesPass2.srcY = 0;
		pitchesPass2.dstY = m_canvasPitch - dstPixelBytes * dest.w;

		int chunkLines;

		if (dest.w >= 2048)
			chunkLines = 1;
		else if (dest.w * dest.h <= 2048)
			chunkLines = dest.h;
		else
			chunkLines = 2048 / dest.w;

		int memBufferSize = chunkLines * dest.w * 8;

		uint8_t* pChunkBuffer = (uint8_t*)GfxBase::memStackAlloc(memBufferSize);

		int line = 0;

		while (line < dest.h)
		{
			int thisChunkLines = std::min(dest.h - line, chunkLines);

			uint8_t* pDst = m_pCanvasPixels + (dest.y + line) * m_canvasPitch + dest.x * dstPixelBytes;
			uint8_t* pSrc = pSource->m_pData + src.y * pSource->m_pitch + line * int(srcPixelBytes * mtx.yx + pSource->m_pitch * mtx.yy) + src.x * srcPixelBytes;
			//			uint8_t * pSrc = pSource->m_pData + (src.y+line) * pSource->m_pitch + src.x * srcPixelBytes;

			pPassOneOp(pSrc, pChunkBuffer, pSource, pitchesPass1, thisChunkLines, dest.w, m_colTrans, { 0,0 }, &mtx);
			m_pBlitSecondPassOp(pChunkBuffer, pDst, pSource, pitchesPass2, thisChunkLines, dest.w, m_colTrans, patchPos, nullptr);

			patchPos.y += thisChunkLines;
			line += thisChunkLines;
		}

		GfxBase::memStackFree(memBufferSize);
	}

	//____ _onePassTransformBlit() ____________________________________________

	void SoftBackend::_onePassTransformBlit(const RectI& dest, BinalCoord pos, const Transform& mtx, CoordI patchPos, TransformBlitOp_p pPassOneOp)
	{
		const SoftSurface* pSource = m_pBlitSource;

		int dstPixelBytes = m_canvasPixelBits / 8;

		uint8_t* pDst = m_pCanvasPixels + dest.y * m_canvasPitch + dest.x * dstPixelBytes;

		pPassOneOp(pSource, pos, &mtx, pDst, dstPixelBytes, m_canvasPitch - dstPixelBytes * dest.w, dest.h, dest.w, m_colTrans, patchPos);
	}


	//____ _twoPassTransformBlit() ____________________________________________

	void SoftBackend::_twoPassTransformBlit(const RectI& dest, BinalCoord pos, const Transform& mtx,
		CoordI patchPos, TransformBlitOp_p pPassOneOp)
	{
		const SoftSurface* pSource = m_pBlitSource;

		int dstPixelBytes = m_canvasPixelBits / 8;

		Pitches pitchesPass2;

		pitchesPass2.srcX = 8;
		pitchesPass2.dstX = dstPixelBytes;
		pitchesPass2.srcY = 0;
		pitchesPass2.dstY = m_canvasPitch - dstPixelBytes * dest.w;

		int chunkLines;

		if (dest.w >= 2048)
			chunkLines = 1;
		else if (dest.w * dest.h <= 2048)
			chunkLines = dest.h;
		else
			chunkLines = 2048 / dest.w;

		int memBufferSize = chunkLines * dest.w * 8;

		uint8_t* pChunkBuffer = (uint8_t*)GfxBase::memStackAlloc(memBufferSize);

		int line = 0;

		while (line < dest.h)
		{
			int thisChunkLines = std::min(dest.h - line, chunkLines);

			uint8_t* pDst = m_pCanvasPixels + (dest.y + line) * m_canvasPitch + dest.x * dstPixelBytes;

			pPassOneOp(pSource, pos, &mtx, pChunkBuffer, 8, 0, thisChunkLines, dest.w, m_colTrans, { 0,0 });
			m_pBlitSecondPassOp(pChunkBuffer, pDst, pSource, pitchesPass2, thisChunkLines, dest.w, m_colTrans, patchPos, nullptr);

			pos.x += mtx.yx * thisChunkLines;
			pos.y += mtx.yy * thisChunkLines;

			patchPos.y += thisChunkLines;
			line += thisChunkLines;
		}

		GfxBase::memStackFree(memBufferSize);
	}

	//____ _dummyStraightBlit() _________________________________________________

	void SoftBackend::_dummyStraightBlit(const RectI& dest, CoordI pos, const Transform& mtx, CoordI patchPos, StraightBlitOp_p pPassOneOp)
	{
		if (m_blendMode == BlendMode::Ignore)
			return;

		char errorMsg[1024];

		if (m_pBlitSource == nullptr)
		{
			snprintf(errorMsg, 1024, "Failed blit operation. Blit source is not set.");
		}
		else
		{
			snprintf(errorMsg, 1024, "Failed blit operation. SoftGfxDevice is missing straight blit kernel for:\n source format = %s\n tile = %s\n tint mode = %s\n blend mode = %s\n, dest format = %s\n", toString(m_pBlitSource->pixelFormat()),
				m_pBlitSource->isTiling() ? "true" : "false",
				toString(m_colTrans.mode),
				toString(m_blendMode),
				toString(m_canvasPixelFormat));
		}

		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	//____ _dummyTransformBlit() ________________________________________________

	void SoftBackend::_dummyTransformBlit(const RectI& dest, BinalCoord pos, const Transform& mtx, CoordI patchPos, TransformBlitOp_p pPassOneOp)
	{
/*
		if (m_blendMode == BlendMode::Ignore)
			return;

		char errorMsg[1024];
		snprintf(errorMsg, 1024, "Failed blit operation. SoftGfxDevice is missing transform blit kernel for:\n source format = %s\n sample method = %s\n tile = %s\n clip = %s\n tint mode = %s\n blend mode = %s\n, dest format = %s\n", toString(m_pBlitSource->pixelFormat()),
			toString(m_pBlitSource->sampleMethod()),
			m_pBlitSource->isTiling() ? "true" : "false",
			m_bClipSource ? "true" : "false",
			toString(m_colTrans.mode),
			toString(m_blendMode),
			toString(m_canvasPixelFormat));

		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
*/
	}

	//____ _updateBlitFunctions() _____________________________________________

	void SoftBackend::_updateBlitFunctions()
	{
		// Start with dummy kernels.

		m_pStraightBlitOp = &SoftBackend::_dummyStraightBlit;
		m_pStraightTileOp = &SoftBackend::_dummyStraightBlit;
		m_pStraightBlurOp = &SoftBackend::_dummyStraightBlit;

		m_pTransformBlitOp = &SoftBackend::_dummyTransformBlit;
		m_pTransformClipBlitOp = &SoftBackend::_dummyTransformBlit;
		m_pTransformTileOp = &SoftBackend::_dummyTransformBlit;
		m_pTransformBlurOp = &SoftBackend::_dummyTransformBlit;

		// Sanity checking...

		if (/*!m_pRenderLayerSurface ||*/ !m_pBlitSource /*|| !m_pCanvasPixels*/ || !m_pBlitSource->m_pData || m_blendMode == BlendMode::Ignore)
			return;

		//

		SampleMethod	sampleMethod = m_pBlitSource->sampleMethod();
		PixelFormat		srcFormat = m_pBlitSource->m_pixelFormat;
		PixelFormat		dstFormat = m_canvasPixelFormat;

		BlendMode		blendMode = m_blendMode;

		if (m_pKernels[(int)dstFormat] == nullptr)
			return;

		// Optimize BlendMode

		// TODO: Optimize by having flag for alpha in m_colTrans, which also is calculated on gradient tints.

		if (m_colTrans.mode == TintMode::None || (m_colTrans.mode == TintMode::Flat && m_colTrans.flatTintColor.a == 4096))
		{
			// TODO: Optimize by using a lookup table.

			if (blendMode == BlendMode::Blend && (srcFormat == PixelFormat::RGB_565_bigendian ||
				srcFormat == PixelFormat::RGB_555_bigendian || srcFormat == PixelFormat::BGR_8_sRGB ||
				srcFormat == PixelFormat::BGR_8_linear || srcFormat == PixelFormat::BGR_565_linear ||
				srcFormat == PixelFormat::BGRX_8_sRGB || srcFormat == PixelFormat::BGRX_8_linear))
			{
				blendMode = BlendMode::Replace;
			}
		}

		// Add two-pass rendering fallback.

		auto pixelDescSource = Util::pixelFormatToDescription(srcFormat);
		auto pixelDescDest = Util::pixelFormatToDescription(dstFormat);

		if ((pixelDescDest.colorSpace == ColorSpace::Linear || dstFormat == PixelFormat::Alpha_8) && (pixelDescSource.colorSpace == ColorSpace::Linear || srcFormat == PixelFormat::Alpha_8))
		{
			m_pStraightBlitFirstPassOp = m_pStraightMoveToBGRA8Kernels[(int)srcFormat][int(ReadOp::Normal)];
			m_pStraightTileFirstPassOp = m_pStraightMoveToBGRA8Kernels[(int)srcFormat][int(ReadOp::Tile)];
			m_pStraightBlurFirstPassOp = m_pStraightMoveToBGRA8Kernels[(int)srcFormat][int(ReadOp::Blur)];
			m_pTransformBlitFirstPassOp = m_pTransformMoveToBGRA8Kernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Normal)];
			m_pTransformTileFirstPassOp = m_pTransformMoveToBGRA8Kernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Tile)];
			m_pTransformClipBlitFirstPassOp = m_pTransformMoveToBGRA8Kernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Clip)];
			m_pTransformBlurFirstPassOp = m_pTransformMoveToBGRA8Kernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Blur)];

			m_pBlitSecondPassOp = m_pKernels[(int)dstFormat]->pStraightBlitFromBGRA8Kernels[(int)m_colTrans.mode][(int)blendMode];
		}
		else
		{
			m_pStraightBlitFirstPassOp = m_pStraightMoveToHiColorKernels[(int)srcFormat][int(ReadOp::Normal)];
			m_pStraightTileFirstPassOp = m_pStraightMoveToHiColorKernels[(int)srcFormat][int(ReadOp::Tile)];
			m_pStraightBlurFirstPassOp = m_pStraightMoveToHiColorKernels[(int)srcFormat][int(ReadOp::Blur)];
			m_pTransformBlitFirstPassOp = m_pTransformMoveToHiColorKernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Normal)];
			m_pTransformTileFirstPassOp = m_pTransformMoveToHiColorKernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Tile)];
			m_pTransformClipBlitFirstPassOp = m_pTransformMoveToHiColorKernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Clip)];
			m_pTransformBlurFirstPassOp = m_pTransformMoveToHiColorKernels[(int)srcFormat][(int)sampleMethod][int(ReadOp::Blur)];

			m_pBlitSecondPassOp = m_pKernels[(int)dstFormat]->pStraightBlitFromHiColorKernels[(int)m_colTrans.mode][(int)blendMode];
		}


		// Try to find suitable one-pass kernels.

		StraightBlitOp_p	pStraightBlitSinglePassKernel = nullptr;
		StraightBlitOp_p	pStraightTileSinglePassKernel = nullptr;
		StraightBlitOp_p	pStraightBlurSinglePassKernel = nullptr;
		TransformBlitOp_p	pTransformBlitSinglePassKernel = nullptr;
		TransformBlitOp_p	pTransformTileSinglePassKernel = nullptr;
		TransformBlitOp_p	pTransformClipBlitSinglePassKernel = nullptr;
		TransformBlitOp_p	pTransformBlurSinglePassKernel = nullptr;


		int singleBlitKernelsIdx = m_pKernels[(int)dstFormat]->singlePassBlitKernels[(int)srcFormat];
		if (singleBlitKernelsIdx > 0)
		{
			auto pSingleBlitKernels = &m_singlePassBlitKernels[singleBlitKernelsIdx - 1];

			int straightBlitKernelsIdx = pSingleBlitKernels->straightBlitKernels[(int)blendMode];
			int transformBlitKernelsIdx = pSingleBlitKernels->transformBlitKernels[(int)blendMode];

			if (straightBlitKernelsIdx > 0)
			{
				auto pStraightBlitKernels = m_singlePassStraightBlitKernels[straightBlitKernelsIdx - 1].pKernels;

				pStraightBlitSinglePassKernel = pStraightBlitKernels[int(ReadOp::Normal)][int(m_colTrans.mode)];
				pStraightTileSinglePassKernel = pStraightBlitKernels[int(ReadOp::Tile)][int(m_colTrans.mode)];
				pStraightBlurSinglePassKernel = pStraightBlitKernels[int(ReadOp::Blur)][int(m_colTrans.mode)];
			}

			if (transformBlitKernelsIdx > 0)
			{
				auto pTransformBlitKernels = m_singlePassTransformBlitKernels[transformBlitKernelsIdx - 1].pKernels;

				pTransformBlitSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Normal)][int(m_colTrans.mode)];
				pTransformTileSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Tile)][int(m_colTrans.mode)];
				pTransformClipBlitSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Clip)][int(m_colTrans.mode)];
				pTransformBlurSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Blur)][int(m_colTrans.mode)];
			}
		}

		// Set kernels to use

		if (pStraightBlitSinglePassKernel)
		{
			m_pStraightBlitOp = &SoftBackend::_onePassStraightBlit;
			m_pStraightBlitFirstPassOp = pStraightBlitSinglePassKernel;
		}
		else if (m_pStraightBlitFirstPassOp && m_pBlitSecondPassOp)
			m_pStraightBlitOp = &SoftBackend::_twoPassStraightBlit;


		if (pStraightTileSinglePassKernel)
		{
			m_pStraightTileOp = &SoftBackend::_onePassStraightBlit;
			m_pStraightTileFirstPassOp = pStraightTileSinglePassKernel;
		}
		else if (m_pStraightTileFirstPassOp && m_pBlitSecondPassOp)
			m_pStraightTileOp = &SoftBackend::_twoPassStraightBlit;

		if (pStraightBlurSinglePassKernel)
		{
			m_pStraightBlurOp = &SoftBackend::_onePassStraightBlit;
			m_pStraightBlurFirstPassOp = pStraightBlurSinglePassKernel;
		}
		else if (m_pStraightBlurFirstPassOp && m_pBlitSecondPassOp)
			m_pStraightBlurOp = &SoftBackend::_twoPassStraightBlit;



		if (pTransformBlitSinglePassKernel)
		{
			m_pTransformBlitOp = &SoftBackend::_onePassTransformBlit;
			m_pTransformBlitFirstPassOp = pTransformBlitSinglePassKernel;
		}
		else if (m_pTransformBlitFirstPassOp && m_pBlitSecondPassOp)
			m_pTransformBlitOp = &SoftBackend::_twoPassTransformBlit;


		if (pTransformClipBlitSinglePassKernel)
		{
			m_pTransformClipBlitOp = &SoftBackend::_onePassTransformBlit;
			m_pTransformClipBlitFirstPassOp = pTransformClipBlitSinglePassKernel;
		}
		else if (m_pTransformClipBlitFirstPassOp && m_pBlitSecondPassOp)
			m_pTransformClipBlitOp = &SoftBackend::_twoPassTransformBlit;


		if (pTransformTileSinglePassKernel)
		{
			m_pTransformTileOp = &SoftBackend::_onePassTransformBlit;
			m_pTransformTileFirstPassOp = pTransformTileSinglePassKernel;
		}
		else if (m_pTransformTileFirstPassOp && m_pBlitSecondPassOp)
			m_pTransformTileOp = &SoftBackend::_twoPassTransformBlit;

		if (pTransformBlurSinglePassKernel)
		{
			m_pTransformBlurOp = &SoftBackend::_onePassTransformBlit;
			m_pTransformBlurFirstPassOp = pTransformBlurSinglePassKernel;
		}
		else if (m_pTransformBlurFirstPassOp && m_pBlitSecondPassOp)
			m_pTransformBlurOp = &SoftBackend::_twoPassTransformBlit;


		return;
	}

	//____ _updateTintMode() ___________________________________________________

	void SoftBackend::_updateTintMode()
	{
		TintMode mode;

		if (!m_colTrans.pTintAxisX && !m_colTrans.pTintAxisY)
			mode = m_colTrans.flatTintColor.isOpaqueWhite() ? TintMode::None : TintMode::Flat;
		else if (m_colTrans.pTintAxisX && m_colTrans.pTintAxisY)
			mode = TintMode::GradientXY;
		else if (m_colTrans.pTintAxisX)
			mode = TintMode::GradientX;
		else
			mode = TintMode::GradientY;

		if (mode != m_colTrans.mode)
		{
			m_colTrans.mode = mode;
			m_bBlitFunctionNeedsUpdate = true;
		}
	}

	//____ _updateBlur() _____________________________________________________

	void SoftBackend::_updateBlur(Blurbrush * pBrush)
	{
		spx radius = pBrush ? pBrush->size() : s_defaultBlurRadius;
		spx cornerRadius = radius * 724 / 1024;

		m_colTrans.blurOfsSPX[0] = { -cornerRadius, -cornerRadius };
		m_colTrans.blurOfsSPX[1] = { 0, -radius };
		m_colTrans.blurOfsSPX[2] = { cornerRadius, -cornerRadius };
		m_colTrans.blurOfsSPX[3] = { -radius, 0 };
		m_colTrans.blurOfsSPX[4] = { 0, 0 };
		m_colTrans.blurOfsSPX[5] = { radius, 0 };
		m_colTrans.blurOfsSPX[6] = { -cornerRadius, cornerRadius };
		m_colTrans.blurOfsSPX[7] = { 0, radius };
		m_colTrans.blurOfsSPX[8] = { cornerRadius, cornerRadius };

		if (pBrush)
		{
			for (int i = 0; i < 9; i++)
			{
				m_colTrans.blurOfsPixel[i].x = (m_colTrans.blurOfsSPX[i].x) / 64;
				m_colTrans.blurOfsPixel[i].y = (m_colTrans.blurOfsSPX[i].y) / 64;

				m_colTrans.blurMtxR[i] = pBrush->red()[i];
				m_colTrans.blurMtxG[i] = pBrush->green()[i];
				m_colTrans.blurMtxB[i] = pBrush->blue()[i];
			}

		}
		else
		{
			for (int i = 0; i < 9; i++)
			{
				m_colTrans.blurOfsPixel[i].x = (m_colTrans.blurOfsSPX[i].x) / 64;
				m_colTrans.blurOfsPixel[i].y = (m_colTrans.blurOfsSPX[i].y) / 64;

				m_colTrans.blurMtxR[i] = s_defaultBlur[i];
				m_colTrans.blurMtxG[i] = s_defaultBlur[i];
				m_colTrans.blurMtxB[i] = s_defaultBlur[i];
			}

		}


	}


	//____ _setupDestFormatKernels() _____________________________________________

	bool SoftBackend::_setupDestFormatKernels(PixelFormat format)
	{
		if (format != PixelFormat::BGR_8_sRGB && format != PixelFormat::BGR_8_linear &&
			format != PixelFormat::BGRX_8_sRGB && format != PixelFormat::BGRX_8_linear &&
			format != PixelFormat::BGRA_8_sRGB && format != PixelFormat::BGRA_8_linear &&
			format != PixelFormat::BGRA_4_linear && format != PixelFormat::BGR_565_linear &&
			format != PixelFormat::Alpha_8 && format != PixelFormat::RGB_565_bigendian &&
			format != PixelFormat::RGB_555_bigendian)
		{
			return false;
		}

		if (!m_pKernels[(int)format])
			m_pKernels[(int)format] = new DestFormatKernels();

		return true;
	}

	//____ _initTables() ____________________________________________________

	void SoftBackend::_initTables()
	{
		if (!s_bTablesInitialized)
		{
			// Init sRGBtoLinearTab

			for (int i = 0; i <= 4096; i++)
				s_limit4096Tab[i] = 0;

			for (int i = 0; i <= 4096; i++)
				s_limit4096Tab[4097 + i] = i;

			for (int i = 0; i <= 4096; i++)
				s_limit4096Tab[4097 * 2 + i] = 4096;

			// Init mulTab

			for (int i = 0; i < 256; i++)
				s_mulTab[i] = 65536 * i / 255;

			// Init line thickness table

			for (int i = 0; i < 17; i++)
			{
				double b = i / 16.0;
				s_lineThicknessTable[i] = (int)(Util::squareRoot(1.0 + b * b) * 65536);
			}

			s_bTablesInitialized = true;
		}
	}

	//____ DestFormatKernels::constructor ________________________________________

	SoftBackend::DestFormatKernels::DestFormatKernels()
	{
		std::memset(this, 0, sizeof(DestFormatKernels));
	}

	//____ SinglePassBlitKernels::constructor ____________________________________

	SoftBackend::SinglePassBlitKernels::SinglePassBlitKernels()
	{
		std::memset(this, 0, sizeof(SinglePassBlitKernels));
	}


} // namespace wg
