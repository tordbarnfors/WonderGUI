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
#include <wg_softedgemap.h>
#include <wg_gfxbase.h>


using namespace std;

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
			m_pCanvas->pullPixels(m_buffer);
			m_pCanvas->freePixelBuffer(m_buffer);
			m_pCanvas = nullptr;
		}
	}

	//____ _beginSession() _______________________________________________________

	void SoftBackend::beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo )
	{
		if( canvasRef != CanvasRef::None )
			setCanvas(canvasRef);
		else
			setCanvas(pCanvas);
	}

	//____ _endSession() _______________________________________________________

	void SoftBackend::endSession()
	{

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
			m_pCanvas->pullPixels(m_buffer);
			m_pCanvas->freePixelBuffer(m_buffer);
		}

		m_pCanvas = pSurface;

		m_buffer = m_pCanvas->allocPixelBuffer();
		m_pCanvasPixels		= m_buffer.pixels;
		m_canvasPixelFormat = m_buffer.format;
		m_canvasPitch		= m_buffer.pitch;
		m_canvasPixelBytes	= m_pCanvas->pixelDescription()->bits/8;

		_resetStates();
	}


	//____ _resetStates() ________________________________________________________

	void SoftBackend::_resetStates()
	{
		m_blendMode = BlendMode::Blend;

		m_pBlitSource = nullptr;
		m_bBlitFunctionNeedsUpdate = true;

		m_colTrans.mode = TintMode::None;
		m_colTrans.flatTintColor = HiColor::White;
		m_colTrans.bTintOpaque = true;
		m_colTrans.tintRect.clear();
		m_colTrans.pTintAxisX = nullptr;
		m_colTrans.pTintAxisY = nullptr;
		m_colTrans.morphFactor = 2048;
		m_colTrans.fixedBlendColor = HiColor::White;

		_updateBlurRadius(s_defaultBlurRadius);

		for (int i = 0; i < 9; i++)
		{
			m_colTrans.blurMtxR[i] = s_defaultBlur[i];
			m_colTrans.blurMtxG[i] = s_defaultBlur[i];
			m_colTrans.blurMtxB[i] = s_defaultBlur[i];
		}

	}


	//____ setObjects() _____________________________________________

	void SoftBackend::setObjects(Object** pBeg, Object** pEnd)
	{
		m_pObjectsBeg = pBeg;
		m_pObjectsEnd = pEnd;
	}

	//____ setRects() _____________________________________________

	void SoftBackend::setRects(RectSPX* pBeg, RectSPX* pEnd)
	{
		m_pRectsBeg = pBeg;
		m_pRectsEnd = pEnd;
		m_pRectsPtr = pBeg;
	}

	//____ setColors() _____________________________________________

	void SoftBackend::setColors(HiColor* pBeg, HiColor* pEnd)
	{
		m_pColorsBeg = pBeg;
		m_pColorsEnd = pEnd;
		m_pColorsPtr = pBeg;
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
		RectSPX *	pRects = m_pRectsPtr;
		HiColor*	pColors = m_pColorsPtr;


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
					m_colTrans.flatTintColor = *pColors++;
					m_colTrans.pTintAxisX = nullptr;
					m_colTrans.pTintAxisY = nullptr;

					m_colTrans.bTintOpaque = (m_colTrans.flatTintColor.a == 4096);
					_updateTintMode();
				}

				if (statesChanged & uint8_t(StateChange::TintMap))
				{
					int32_t	x = *p++ / 64;
					int32_t	y = *p++ / 64;
					int32_t	w = *p++ / 64;
					int32_t	h = *p++ / 64;

					m_colTrans.tintRect = RectI(x, y, w, h);
					
					int32_t nHorrColors = *p++;
					int32_t nVertColors = *p++;

					auto pOurColors = pColors;
					
					if( nHorrColors > 0 )
					{
						m_colTrans.pTintAxisX = pColors;
						pColors += nHorrColors;
					}
					else
						m_colTrans.pTintAxisX = nullptr;

					if( nVertColors > 0 )
					{
						m_colTrans.pTintAxisY = pColors;
						pColors += nVertColors;
					}
					else
						m_colTrans.pTintAxisY = nullptr;

					// Check if whole tint is opaque.
					
					int totalAlpha = 0;
					while( pOurColors < pColors )
					{
						totalAlpha += pOurColors->a;
						pOurColors++;
					}
					
					m_colTrans.bTintOpaque = (totalAlpha == 4096 * (nHorrColors + nVertColors));

					_updateTintMode();
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					m_colTrans.morphFactor = *p++;
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
					m_colTrans.fixedBlendColor = *pColors++;
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					spx		radius = *p++;

					const spx* pRed = p;
					const spx* pGreen = p + 9;
					const spx* pBlue = p + 18;
					p += 27;

					_updateBlurRadius(radius);

					for (int i = 0; i < 9; i++)
					{
						m_colTrans.blurMtxR[i] = pRed[i] * 65536;
						m_colTrans.blurMtxG[i] = pGreen[i] * 65536;
						m_colTrans.blurMtxB[i] = pBlue[i] * 65536;
					}
				}

				break;
			}

			case Command::Fill:
			{
				int32_t nRects = *p++;

				HiColor&  col = * pColors++;

				FillOp_p pFunc = nullptr;

				// Optimize calls

				BlendMode blendMode = m_blendMode;
				if (blendMode == BlendMode::Blend && col.a == 4096 && (m_colTrans.mode == TintMode::None || (m_colTrans.mode == TintMode::Flat && m_colTrans.flatTintColor.a == 4096)) )
				{
					blendMode = BlendMode::Replace;
				}

				auto pKernels = m_pKernels[(int)m_pCanvas->pixelFormat()];
				if (pKernels)
					pFunc = pKernels->pFillKernels[(int)m_colTrans.mode][(int)blendMode];

				if (!pFunc)
				{
					pRects += nRects;

					if (blendMode == BlendMode::Ignore)
						break;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed fill operation. SoftGfxDevice is missing fill kernel for TintMode::%s, BlendMode::%s onto surface of PixelFormat:%s.",
						toString(m_colTrans.mode),
						toString(blendMode),
						toString(m_pCanvas->pixelFormat()));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					break;
				}

				for (int i = 0; i < nRects; i++)
				{
					RectI patch = * pRects++;

					if (((patch.x | patch.y | patch.w | patch.h) & 63) == 0)
					{
						// Pixel aligned fill

						patch /= 64;

						uint8_t* pDst = m_pCanvasPixels + patch.y * m_canvasPitch + patch.x * m_canvasPixelBytes;
						pFunc(pDst, m_canvasPixelBytes, m_canvasPitch - patch.w * m_canvasPixelBytes, patch.h, patch.w, col, m_colTrans, patch.pos());
					}
					else
					{
						// Subpixel fill


						// Fill all but anti-aliased edges

						int x1 = ((patch.x + 63) >> 6);
						int y1 = ((patch.y + 63) >> 6);
						int x2 = ((patch.x + patch.w) >> 6);
						int y2 = ((patch.y + patch.h) >> 6);

						uint8_t* pDst = m_pCanvasPixels + y1 * m_canvasPitch + x1 * m_canvasPixelBytes;
						pFunc(pDst, m_canvasPixelBytes, m_canvasPitch - (x2 - x1) * m_canvasPixelBytes, y2 - y1, x2 - x1, col, m_colTrans, { x1,y1 });

						//

						BlendMode	edgeBlendMode = (blendMode == BlendMode::Replace) ? BlendMode::Blend : blendMode; // Need to blend edges and corners even if fill is replace

						FillOp_p pEdgeFunc = pKernels->pFillKernels[(int)m_colTrans.mode][(int)edgeBlendMode];

						if (pEdgeFunc == nullptr)
						{
							pRects += (nRects - i - 1);

							if (blendMode == BlendMode::Ignore)
								break;

							char errorMsg[1024];

							snprintf(errorMsg, 1024, "Failed fill operation. SoftGfxDevice is missing fill kernel for TintMode::%s, BlendMode::%s onto surface of PixelFormat:%s.",
								toString(m_colTrans.mode),
								toString(blendMode),
								toString(m_pCanvas->pixelFormat()));

							GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
							break;
						}

						// Draw the sides

						int aaLeft = (4096 - (patch.x & 0x3F) * 64) & 4095;
						int aaTop = (4096 - (patch.y & 0x3F) * 64) & 4095;
						int aaRight = ((patch.x + patch.w) & 0x3F) * 64;
						int aaBottom = ((patch.y + patch.h) & 0x3F) * 64;

						int aaTopLeft = aaTop * aaLeft / 4096;
						int aaTopRight = aaTop * aaRight / 4096;
						int aaBottomLeft = aaBottom * aaLeft / 4096;
						int aaBottomRight = aaBottom * aaRight / 4096;


						if (blendMode != BlendMode::Replace)
						{
							int alpha = col.a;

							aaLeft = aaLeft * alpha >> 12;
							aaTop = aaTop * alpha >> 12;
							aaRight = aaRight * alpha >> 12;
							aaBottom = aaBottom * alpha >> 12;

							aaTopLeft = aaTopLeft * alpha >> 12;
							aaTopRight = aaTopRight * alpha >> 12;
							aaBottomLeft = aaBottomLeft * alpha >> 12;
							aaBottomRight = aaBottomRight * alpha >> 12;
						}

						RectI pixelPatch = patch / 64;
						HiColor color = col;

						if (aaTop != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + pixelPatch.y * m_canvasPitch + x1 * m_canvasPixelBytes;
							int length = x2 - x1;
							color.a = aaTop;
							pEdgeFunc(pDst, m_canvasPixelBytes, 0, 1, length, color, m_colTrans, { x1,pixelPatch.y });
						}

						if (aaBottom != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + y2 * m_canvasPitch + x1 * m_canvasPixelBytes;
							int length = x2 - x1;
							color.a = aaBottom;
							pEdgeFunc(pDst, m_canvasPixelBytes, 0, 1, length, color, m_colTrans, { x1,y2 });
						}

						if (aaLeft != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + y1 * m_canvasPitch + pixelPatch.x * m_canvasPixelBytes;
							int length = y2 - y1;
							color.a = aaLeft;
							pEdgeFunc(pDst, m_canvasPitch, 0, 1, length, color, m_colTrans, { pixelPatch.x, y1 });
						}

						if (aaRight != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + y1 * m_canvasPitch + x2 * m_canvasPixelBytes;
							int length = y2 - y1;
							color.a = aaRight;
							pEdgeFunc(pDst, m_canvasPitch, 0, 1, length, color, m_colTrans, { x2, y1 });
						}

						// Draw corner pieces


						if (aaTopLeft != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + pixelPatch.y * m_canvasPitch + pixelPatch.x * m_canvasPixelBytes;
							color.a = aaTopLeft;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { pixelPatch.x, pixelPatch.y });
						}

						if (aaTopRight != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + pixelPatch.y * m_canvasPitch + x2 * m_canvasPixelBytes;
							color.a = aaTopRight;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { x2, pixelPatch.y });
						}

						if (aaBottomLeft != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + y2 * m_canvasPitch + pixelPatch.x * m_canvasPixelBytes;
							color.a = aaBottomLeft;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { pixelPatch.x, y2 });
						}

						if (aaBottomRight != 0)
						{
							uint8_t* pDst = m_pCanvasPixels + y2 * m_canvasPitch + x2 * m_canvasPixelBytes;
							color.a = aaBottomRight;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { x2, y2 });
						}

					}
				}
				break;
			}

			case Command::Line:
			{
				spx thickness = * p++;
				int32_t nClipRects = * p++;
				int32_t nLines = *p++;

				HiColor color = *pColors++;

				const RectSPX * pClipRects = pRects;
				pRects += nClipRects;

				HiColor fillColor = color;
				
				if( m_colTrans.mode == TintMode::Flat )
				fillColor = fillColor * m_colTrans.flatTintColor;

				//

				ClipLineOp_p pOp = nullptr;

				auto pKernels = m_pKernels[(int)m_pCanvas->pixelFormat()];
				if (pKernels)
					pOp = pKernels->pClipLineKernels[(int)m_blendMode];

				if (pOp == nullptr)
				{
					if (m_blendMode == BlendMode::Ignore)
						return;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed drawLine operation. SoftGfxDevice is missing clipLine kernel for BlendMode::%s onto surface of PixelFormat:%s.",
						toString(m_blendMode),
						toString(m_pCanvas->pixelFormat()));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					return;
				}


				uint8_t* pRow;
				int		rowInc, pixelInc;
				int 	length, width;
				int		pos, slope;
				int		clipStart, clipEnd;

				for (int line = 0; line < nLines; line++)
				{
					CoordSPX beg = { *p++, *p++ };
					CoordSPX end = { *p++, *p++ };

					//TODO: Proper 26:6 support
					beg = Util::roundToPixels(beg);
					end = Util::roundToPixels(end);


					if (std::abs(beg.x - end.x) > std::abs(beg.y - end.y))
					{
						// Prepare mainly horizontal line segment

						if (beg.x > end.x)
							swap(beg, end);

						length = end.x - beg.x;
						slope = ((end.y - beg.y) * 65536) / length;

						width = _scaleLineThickness(thickness / 64.f, slope);
						pos = (beg.y << 16) - width / 2 + 32768;

						rowInc = m_canvasPixelBytes;
						pixelInc = m_canvasPitch;

						pRow = m_pCanvasPixels + beg.x * rowInc;

						// Loop through patches

						for (int i = 0; i < nClipRects; i++)
						{
							// Do clipping

							const RectI clip = pClipRects[i] / 64;

							int _length = length;
							int _pos = pos;
							uint8_t* _pRow = pRow;

							if (beg.x < clip.x)
							{
								int cut = clip.x - beg.x;
								_length -= cut;
								_pRow += rowInc * cut;
								_pos += slope * cut;
							}

							if (end.x > clip.x + clip.w)
								_length -= end.x - (clip.x + clip.w);

							clipStart = clip.y << 16;
							clipEnd = (clip.y + clip.h) << 16;

							//  Draw

							pOp(clipStart, clipEnd, _pRow, rowInc, pixelInc, _length, width, _pos, slope, fillColor, m_colTrans, { 0,0 });
						}
					}
					else
					{
						// Prepare mainly vertical line segment

						if (beg.y > end.y)
							swap(beg, end);

						length = end.y - beg.y;
						if (length == 0)
							return;											// TODO: Should stil draw the caps!

						// Need multiplication instead of shift as operand might be negative
						slope = ((end.x - beg.x) * 65536) / length;
						width = _scaleLineThickness(thickness / 64.f, slope);
						pos = (beg.x << 16) - width / 2 + 32768;

						rowInc = m_canvasPitch;
						pixelInc = m_canvasPixelBytes;

						pRow = m_pCanvasPixels + beg.y * rowInc;

						// Loop through patches

						for (int i = 0; i < nClipRects; i++)
						{
							// Do clipping

							const RectI clip = pClipRects[i] / 64;

							int _length = length;
							int _pos = pos;
							uint8_t* _pRow = pRow;

							if (beg.y < clip.y)
							{
								int cut = clip.y - beg.y;
								_length -= cut;
								_pRow += rowInc * cut;
								_pos += slope * cut;
							}

							if (end.y > clip.y + clip.h)
								_length -= end.y - (clip.y + clip.h);

							clipStart = clip.x << 16;
							clipEnd = (clip.x + clip.w) << 16;

							//  Draw

							pOp(clipStart, clipEnd, _pRow, rowInc, pixelInc, _length, width, _pos, slope, fillColor, m_colTrans, { 0,0 });
						}
					}
				}

				break;
			}


			case Command::DrawEdgemap:
			{
				int32_t objectOfs = *p++;
				auto pEdgemap = static_cast<SoftEdgemap*>(m_pObjectsBeg[objectOfs]);

				int32_t	destX = *p++;
				int32_t	destY = *p++;

				int32_t	flip = *p++;

				int32_t nRects = *p++;
				RectSPX * pMyRects = pRects;
				pRects += nRects;

				int32_t nSegments = pEdgemap->segments();
				const HiColor * pSegmentColors = pEdgemap->m_pFlatColors;
				
				int nEdgeStrips = pEdgemap->m_size.w + 1;

				int edgeStripPitch = pEdgemap->m_nbSegments - 1;

				//TODO: Proper 26:6 support


				auto& mtx = s_blitFlipTransforms[flip];

				RectI _dest = { 
					Util::roundToPixels(destX), 
					Util::roundToPixels(destY), 
					pEdgemap->m_size.w * int(abs(mtx.xx)) + pEdgemap->m_size.h * int(abs(mtx.yx)),
					pEdgemap->m_size.w * int(abs(mtx.xy)) + pEdgemap->m_size.h * int(abs(mtx.yy)),
				};

				RectI dest = _dest;

				SegmentEdge edges[c_maxSegments - 1];

				int xPitch = m_canvasPixelBytes;
				int yPitch = m_canvasPitch;

				// We need to modify our transform since we are moving the destination pointer, not the source pointer, according to the transform.

				int simpleTransform[2][2];

				simpleTransform[0][0] = mtx.xx;
				simpleTransform[1][1] = mtx.yy;

				if (mtx.xx == 0 || mtx.yy == 0)
				{
					simpleTransform[0][1] = mtx.yx;
					simpleTransform[1][0] = mtx.xy;
				}
				else
				{
					simpleTransform[0][1] = mtx.xy;
					simpleTransform[1][0] = mtx.yx;
				}

				// Calculate pitches

				int colPitch = simpleTransform[0][0] * xPitch + simpleTransform[0][1] * yPitch;
				int rowPitch = simpleTransform[1][0] * xPitch + simpleTransform[1][1] * yPitch;

				// Calculate start coordinate

				CoordI start = dest.pos();

				if (simpleTransform[0][0] + simpleTransform[1][0] < 0)
					start.x += dest.w - 1;

				if (simpleTransform[0][1] + simpleTransform[1][1] < 0)
					start.y += dest.h - 1;

				// Detect if strip columns are lined horizontally or verically

				bool bHorizontalColumns = (abs(colPitch) == xPitch);

				// Limit size of destination rect by number of edgestrips.

				if (bHorizontalColumns)
				{
					if (dest.w > nEdgeStrips - 1)
					{
						if (colPitch < 0)
							dest.x += dest.w - nEdgeStrips - 1;

						dest.w = nEdgeStrips - 1;
					}
				}
				else
				{
					if (dest.h > nEdgeStrips - 1)
					{
						if (colPitch < 0)
							dest.y += dest.h - nEdgeStrips - 1;

						dest.h = nEdgeStrips - 1;
					}
				}

				// Apply tinting

				int16_t	colors[c_maxSegments * 4][4];				// RGBA order of elements
				bool	transparentSegments[c_maxSegments];
				bool	opaqueSegments[c_maxSegments];


				// Determine combined tint-mode, start with global m_colTrans...

				bool	bTintX = false;
				bool	bTintY = false;

				if (m_colTrans.mode != TintMode::None)
				{
					if (m_colTrans.mode == TintMode::GradientXY)
					{
						bTintX = bTintY = true;
					}
					else
					{
						if (m_colTrans.mode == TintMode::GradientY)
						{
							if (mtx.xy == 0 && mtx.yx == 0)
								bTintY = true;
							else
								bTintX = true;
						}
						if (m_colTrans.mode == TintMode::GradientX)
						{
							if (mtx.xy == 0 && mtx.yx == 0)
								bTintX = true;
							else
								bTintY = true;
						}

					}
				}

				// ... add in tintmaps for segments
								
				if( pEdgemap->colorstripsX() )
					bTintX = true;
				
				if( pEdgemap->colorstripsY() )
					bTintY = true;				

				// Unpack input colors and fill in transparentSegments

				if (!bTintX && !bTintY)
				{
					// If we just use flat tinting (or no tint at all), we tint our segment colors right away

					auto pColor = pSegmentColors;
					
					for (int i = 0; i < nSegments; i++)
					{
						colors[i][0] = (pSegmentColors[i].r * m_colTrans.flatTintColor.r) >> 12;
						colors[i][1] = (pSegmentColors[i].g * m_colTrans.flatTintColor.g) >> 12;
						colors[i][2] = (pSegmentColors[i].b * m_colTrans.flatTintColor.b) >> 12;
						colors[i][3] = (pSegmentColors[i].a * m_colTrans.flatTintColor.a) >> 12;
						
						transparentSegments[i] = (colors[i][3] == 0);
						opaqueSegments[i] = (colors[i][3] == 4096);						
					}
				}


				HiColor * pTintColorsX = nullptr;
				HiColor * pTintColorsY = nullptr;

				int		tintBufferSizeX = 0;
				int		tintBufferSizeY = 0;
				
				int		segmentPitchTintmapY = 0;
				int		segmentPitchTintmapX = 0;

				
				// If we instead have gradients we have things to take care of...

				if (bTintX || bTintY)
				{
					if (bTintX)
					{
						int length = pEdgemap->m_size.w;

						segmentPitchTintmapX = length;

						// Generate the buffer that we will need

						tintBufferSizeX = sizeof(HiColor) * nSegments * length;
						pTintColorsX = (HiColor*)GfxBase::memStackAlloc(tintBufferSizeX);
						
						// export segment colorstrip or flat colors into our buffer
						
						if (pEdgemap->m_pColorstripsX)
							memcpy(pTintColorsX, pEdgemap->m_pColorstripsX, tintBufferSizeX);
						else if (pEdgemap->m_pFlatColors)
						{
							HiColor* pOutput = pTintColorsX;
							HiColor* pInput = pEdgemap->m_pFlatColors;

							for (int seg = 0; seg < nSegments; seg++)
							{
								for (int i = 0; i < length; i++)
									*pOutput++ = *pInput;

								pInput++;
							}
						}
					}
					
					if (bTintY)
					{
						int length = pEdgemap->m_size.h;

						segmentPitchTintmapY = length;
						
						// Generate the buffer that we will need

						tintBufferSizeY = sizeof(HiColor) * nSegments * length;
						pTintColorsY = (HiColor*)GfxBase::memStackAlloc(tintBufferSizeY);
						
						// Export segment colorstrip into our buffer if we have them.
						// Otherwise, if we haven't already exported our flat colors we do it now.
						// If flat colors already have been exported, we just fill with white.

						if (pEdgemap->m_pColorstripsY)
							memcpy(pTintColorsY, pEdgemap->m_pColorstripsY, tintBufferSizeY);
						else if (pEdgemap->m_pFlatColors && !bTintX)
						{
							HiColor* pOutput = pTintColorsX;
							HiColor* pInput = pEdgemap->m_pFlatColors;

							for (int seg = 0; seg < nSegments; seg++)
							{
								for (int i = 0; i < length; i++)
									*pOutput++ = *pInput;

								pInput++;
							}
						}
						else
						{
							for (int i = 0; i < tintBufferSizeY; i++)
								pTintColorsY[i] = HiColor::White;
						}
					}

					// Possibly add in global tint, which might need to be rotated, offset and reversed

					if (m_colTrans.mode == TintMode::Flat)
					{
						if (m_colTrans.flatTintColor != HiColor::White)
						{
							// We only apply tintColor once, so we only modify one of the color lists.

							if (pTintColorsX)
							{
								for (int i = 0; i < nSegments * pEdgemap->m_size.w; i++)
									pTintColorsX[i] *= m_colTrans.flatTintColor;
							}
							else if (pTintColorsY)
							{
								for (int i = 0; i < nSegments * pEdgemap->m_size.h; i++)
									pTintColorsY[i] *= m_colTrans.flatTintColor;
							}
						}
					}
					else if (m_colTrans.mode == TintMode::GradientX || m_colTrans.mode == TintMode::GradientY || m_colTrans.mode == TintMode::GradientXY)
					{
						HiColor* pGlobalsX = m_colTrans.pTintAxisX ? m_colTrans.pTintAxisX + _dest.x - m_colTrans.tintRect.x : nullptr;
						HiColor* pGlobalsY = m_colTrans.pTintAxisY ? m_colTrans.pTintAxisY + _dest.y - m_colTrans.tintRect.y : nullptr;

						int width = _dest.w;
						int height = _dest.h;

						int pitchX = mtx.xx + mtx.xy;
						int pitchY = mtx.yx + mtx.yy;

						if (mtx.xy != 0 || mtx.yx != 0)
						{
							std::swap(pGlobalsX, pGlobalsY);
							std::swap(width, height);
							std::swap(pitchX, pitchY);
						}


						if (pitchX < 0 && pGlobalsX)
							pGlobalsX += width - 1;

						if (pitchY < 0 && pGlobalsY)
							pGlobalsY += height - 1;


						if (pGlobalsX)
						{
							HiColor* pDest = pTintColorsX;
							for (int seg = 0; seg < nSegments; seg++)
							{
								HiColor* pSrc = pGlobalsX;

								for (int i = 0; i < width; i++)
								{
									*pDest++ *= *pSrc;
									pSrc += pitchX;
								}
							}
						}

						if (pGlobalsY)
						{
							HiColor* pDest = pTintColorsY;
							for (int seg = 0; seg < nSegments; seg++)
							{
								HiColor* pSrc = pGlobalsY;

								for (int i = 0; i < height; i++)
								{
									*pDest++ *= *pSrc;
									pSrc += pitchY;
								}
							}
						}
					}


					// Mark transparent and opaque segments
					
					if (m_colTrans.bTintOpaque)
					{
						for (int i = 0; i < nSegments; i++)
						{
							opaqueSegments[i] = pEdgemap->m_opaqueSegments.test(i);
							transparentSegments[i] = pEdgemap->m_transparentSegments.test(i);
						}

					}
					else
					{
						for (int i = 0; i < nSegments; i++)
						{
							opaqueSegments[i] = false;
							transparentSegments[i] = pEdgemap->m_transparentSegments.test(i);
						}
					}
				}


				// Modify opaqueSegments if our state isn't blend

				if (m_blendMode != BlendMode::Blend && m_blendMode != BlendMode::BlendFixedColor)
				{
					bool val = (m_blendMode == BlendMode::Replace);

					for (int seg = 0; seg < nSegments; seg++)
						opaqueSegments[seg] = val;
				}

				// Modify transparentSegments if our state is BlendFixedColor

				if (m_blendMode == BlendMode::BlendFixedColor)
				{
					for (int seg = 0; seg < nSegments; seg++)
						transparentSegments[seg] = false;
				}

				// Set start position and clip dest

				uint8_t* pOrigo = m_pCanvasPixels + start.y * yPitch + start.x * xPitch;

				
				StripSource stripSource = StripSource::Colors;
				if( bTintY )
				{
					if(bTintX)
						stripSource = StripSource::ColorsAndTintmaps;
					else
						stripSource = StripSource::Tintmaps;
				}
				
				SegmentOp_p	pOp = nullptr;
				auto pKernels = m_pKernels[(int)m_pCanvas->pixelFormat()];
				if (pKernels)
					pOp = pKernels->pSegmentKernels[(int)stripSource][(int)m_blendMode];

				if (pOp == nullptr)
				{
					if (m_blendMode == BlendMode::Ignore)
						return;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed draw segments operation. SoftGfxDevice is missing segments kernel %s Y-tint for BlendMode::%s onto surface of PixelFormat:%s.",
						bTintY ? "with" : "without",
						toString(m_blendMode),
						toString(m_pCanvas->pixelFormat()));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					return;
				}

				// Loop through patches

				for (int patchIdx = 0; patchIdx < nRects; patchIdx++)
				{
					// Clip patch

					RectI patch = RectI::overlap(dest, pMyRects[patchIdx] / 64);
					if (patch.w == 0 || patch.h == 0)
						continue;

					// Calculate stripstart, clipBeg/clipEnd and first edge for patch

					int columnOfs;
					int rowOfs;

					int columns;
					int rows;

					if (bHorizontalColumns)
					{
						columnOfs = colPitch > 0 ? patch.x - dest.x : dest.right() - patch.right();
						rowOfs = rowPitch > 0 ? patch.y - dest.y : dest.bottom() - patch.bottom();

						columns = patch.w;
						rows = patch.h;
					}
					else
					{
						columnOfs = colPitch > 0 ? patch.y - dest.y : dest.bottom() - patch.bottom();
						rowOfs = rowPitch > 0 ? patch.x - dest.x : dest.right() - patch.right();

						columns = patch.h;
						rows = patch.w;
					}

					
					pEdgemap->samples();

					const int* pEdgeStrips = pEdgemap->m_pSamples + columnOfs * edgeStripPitch;
					uint8_t* pStripStart = pOrigo + columnOfs * colPitch;

					int clipBeg = rowOfs * 256;
					int clipEnd = clipBeg + (rows * 256);

					for (int x = 0; x < columns; x++)
					{
						int nEdges = 0;
						int skippedSegments = 0;

						for (int y = 0; y < nSegments - 1; y++)
						{
							int beg = pEdgeStrips[y] * 4;
							int end = pEdgeStrips[y + edgeStripPitch] * 4;

							if (beg > end)
								swap(beg, end);

							if (beg >= clipEnd)
								break;

							if (end > clipBeg)
							{
								int coverageInc = (end == beg) ? 0 : (65536 * 256) / (end - beg);
								int coverage = 0;

								if (beg < clipBeg)
								{
									int cut = clipBeg - beg;
									beg = clipBeg;
									coverage += (coverageInc * cut) >> 8;
								}

								if (end > clipEnd)
									end = clipEnd;


								edges[nEdges].begin = beg;
								edges[nEdges].end = end;
								edges[nEdges].coverage = coverage;
								edges[nEdges].coverageInc = coverageInc;
								nEdges++;
							}
							else
								skippedSegments++;
						}

						// Update tinting if we have X-gradient

						const int16_t* pColors = &colors[skippedSegments][0];

						if (bTintX)
						{
							for( int i = 0 ; i < nSegments ; i++ )
							{
								HiColor& col = pTintColorsX[i*segmentPitchTintmapX+columnOfs];

								colors[i][0] = col.r;
								colors[i][1] = col.g;
								colors[i][2] = col.b;
								colors[i][3] = col.a;
							}
							
							pColors = &colors[0][0];
						}

						//

 						pOp(clipBeg, clipEnd, pStripStart, rowPitch, nEdges, edges, pColors, pTintColorsY + skippedSegments * segmentPitchTintmapY, segmentPitchTintmapY , transparentSegments + skippedSegments, opaqueSegments + skippedSegments, m_colTrans);
						pEdgeStrips += edgeStripPitch;
						pStripStart += colPitch;
						columnOfs++;
					}
				}

				// Free what we have reservhed on the memStack.

				if (tintBufferSizeY > 0)
					GfxBase::memStackFree(tintBufferSizeY);

				if (tintBufferSizeX > 0)
					GfxBase::memStackFree(tintBufferSizeX);

				break;
			}

			case Command::Blur:
			case Command::Blit:
			case Command::ClipBlit:
			case Command::Tile:
			{
				if (m_bBlitFunctionNeedsUpdate)
				{
					_updateBlitFunctions();
					m_bBlitFunctionNeedsUpdate = false;
				}

				int32_t nRects = *p++;
				int32_t transform = *p++;

				int srcX = *p++;
				int srcY = *p++;
				spx dstX = *p++;
				spx dstY = *p++;


				if (transform <= int(GfxFlip_max) )
				{
					const Transform& mtx = s_blitFlipTransforms[transform];

					// Step forward _src by half a pixel, so we start from correct pixel.

					srcX += (mtx.xx + mtx.yx) * 512;
					srcY += (mtx.xy + mtx.yy) * 512;

					//

					for (int i = 0; i < nRects; i++)
					{
						RectI	patch = (*pRects++) / 64;

						CoordI src = { srcX / 1024, srcY / 1024 };
						CoordI dest = { dstX / 64, dstY / 64 };

						CoordI	patchOfs = patch.pos() - dest;


						//

						src.x += patchOfs.x * mtx.xx + patchOfs.y * mtx.yx;
						src.y += patchOfs.x * mtx.xy + patchOfs.y * mtx.yy;

						if (cmd == Command::Blit)
							(this->*m_pStraightBlitOp)(patch, src, mtx, patch.pos(), m_pStraightBlitFirstPassOp);
						else if (cmd == Command::Tile)
							(this->*m_pStraightTileOp)(patch, src, mtx, patch.pos(), m_pStraightTileFirstPassOp);
						else
							(this->*m_pStraightBlurOp)(patch, src, mtx, patch.pos(), m_pStraightBlurFirstPassOp);
					}
				}
				else
				{
					binalInt mtx[2][2];

					Transform* pTransform = &m_pTransformsBeg[transform - GfxFlip_size];

					mtx[0][0] = binalInt(pTransform->xx * BINAL_MUL);
					mtx[0][1] = binalInt(pTransform->xy * BINAL_MUL);
					mtx[1][0] = binalInt(pTransform->yx * BINAL_MUL);
					mtx[1][1] = binalInt(pTransform->yy * BINAL_MUL);


					//

					for (int i = 0; i < nRects; i++)
					{
						RectI	patch = (*pRects++) / 64;

						BinalCoord src = { srcX * (BINAL_MUL / 1024), srcY * (BINAL_MUL / 1024) };
						CoordI dest = { dstX / 64, dstY / 64 };

						CoordI	patchOfs = patch.pos() - dest;

						//

						src.x += patchOfs.x * mtx[0][0] + patchOfs.y * mtx[1][0];
						src.y += patchOfs.x * mtx[0][1] + patchOfs.y * mtx[1][1];

						//

						if( cmd == Command::Blit)
							(this->*m_pTransformBlitOp)(patch, src, mtx, patch.pos(), m_pTransformBlitFirstPassOp);
						else if (cmd == Command::ClipBlit)
							(this->*m_pTransformClipBlitOp)(patch, src, mtx, patch.pos(), m_pTransformClipBlitFirstPassOp);
						else if (cmd == Command::Tile)
							(this->*m_pTransformTileOp)(patch, src, mtx, patch.pos(), m_pTransformTileFirstPassOp);
						else
							(this->*m_pTransformBlurOp)(patch, src, mtx, patch.pos(), m_pTransformBlurFirstPassOp);
					}
				}

				break;
			}

			default:
				break;
			}
		}

		// Save progress.

		m_pRectsPtr = pRects;
		m_pColorsPtr = pColors;

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
		return c_maxSegments-1;
	}

	//____ surfaceType() ______________________________________________________

	const TypeInfo& SoftBackend::surfaceType(void) const
	{
		return SoftSurface::TYPEINFO;
	}

	//____ setLineKernel() ____________________________________________________

	bool SoftBackend::setLineKernel(BlendMode blendMode, PixelFormat destFormat, LineOp_p pKernel)
	{
		if (!_setupDestFormatKernels(destFormat))
			return false;

		m_pKernels[(int)destFormat]->pLineKernels[(int)blendMode] = pKernel;
		return true;
	}

	//____ setClipLineKernel() ________________________________________________

	bool SoftBackend::setClipLineKernel(BlendMode blendMode, PixelFormat destFormat, ClipLineOp_p pKernel)
	{
		if (!_setupDestFormatKernels(destFormat))
			return false;

		m_pKernels[(int)destFormat]->pClipLineKernels[(int)blendMode] = pKernel;
		return true;
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

	//____ setTransformBlitKernel() ___________________________________________

	bool SoftBackend::setTransformBlitKernel(PixelFormat sourceFormat, SampleMethod sampleMethod, SoftBackend::ReadOp edgeOp, TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, TransformBlitOp_p pKernel)
	{
		bool success = false;
		
		if( _setupDestFormatKernels(destFormat) )
		{
			int singlePassKernelsIdx = m_pKernels[(int)destFormat]->singlePassBlitKernels[(int)sourceFormat];
			if( singlePassKernelsIdx == 0 )
			{
				m_singlePassBlitKernels.emplace_back();
				singlePassKernelsIdx = (int) m_singlePassBlitKernels.size();
				m_pKernels[(int)destFormat]->singlePassBlitKernels[(int)sourceFormat] = (uint16_t) singlePassKernelsIdx;
			}
			singlePassKernelsIdx--;
			
			int transformBlitKernelsIdx = m_singlePassBlitKernels[singlePassKernelsIdx].transformBlitKernels[(int)blendMode];
			if( transformBlitKernelsIdx == 0 )
			{
				m_singlePassTransformBlitKernels.emplace_back();
				transformBlitKernelsIdx = (int) m_singlePassTransformBlitKernels.size();
				m_singlePassBlitKernels[singlePassKernelsIdx].transformBlitKernels[(int)blendMode] = transformBlitKernelsIdx;
			}
			transformBlitKernelsIdx--;
			
			m_singlePassTransformBlitKernels[transformBlitKernelsIdx].pKernels[(int)sampleMethod][(int)edgeOp][(int)tintMode] = pKernel;
			success = true;
		}
		
		if( destFormat == PixelFormat::Undefined && blendMode == BlendMode::Replace && tintMode == TintMode::None )			// Special case for HiColor destination.
		{
			m_pTransformMoveToHiColorKernels[(int)sourceFormat][(int)sampleMethod][(int)edgeOp] = pKernel;
			success = true;
		}
			
		if( destFormat == PixelFormat::BGRA_8_linear && blendMode == BlendMode::Replace && tintMode == TintMode::None )		// Special case for HiColor destination.
		{
			m_pTransformMoveToBGRA8Kernels[(int)sourceFormat][(int)sampleMethod][(int)edgeOp] = pKernel;
			success = true;
		}
				
		return success;
	}

	//____ setSegmentStripKernel() ____________________________________________

	bool SoftBackend::setSegmentStripKernel(SoftBackend::StripSource stripSource, BlendMode blendMode, PixelFormat destFormat, SegmentOp_p pKernel)
	{
		if (!_setupDestFormatKernels(destFormat))
			return false;

		m_pKernels[(int)destFormat]->pSegmentKernels[(int)stripSource][(int)blendMode] = pKernel;
		return true;
	}

	//____ _onePassStraightBlit() _____________________________________________

	void SoftBackend::_onePassStraightBlit(const RectI& dest, CoordI src, const Transform& mtx, CoordI patchPos, StraightBlitOp_p pPassOneOp)
	{
		const SoftSurface* pSource = m_pBlitSource;

		int srcPixelBytes = pSource->m_pPixelDescription->bits / 8;
		int dstPixelBytes = m_canvasPixelBytes;

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
		int dstPixelBytes = m_canvasPixelBytes;

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

	void SoftBackend::_onePassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp)
	{
		const SoftSurface* pSource = m_pBlitSource;

		int dstPixelBytes = m_canvasPixelBytes;

		uint8_t* pDst = m_pCanvasPixels + dest.y * m_canvasPitch + dest.x * dstPixelBytes;

		pPassOneOp(pSource, pos, transformMatrix, pDst, dstPixelBytes, m_canvasPitch - dstPixelBytes * dest.w, dest.h, dest.w, m_colTrans, patchPos);
	}


	//____ _twoPassTransformBlit() ____________________________________________

	void SoftBackend::_twoPassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2],
		CoordI patchPos, TransformBlitOp_p pPassOneOp)
	{
		const SoftSurface* pSource = m_pBlitSource;

		int dstPixelBytes = m_canvasPixelBytes;

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

			pPassOneOp(pSource, pos, transformMatrix, pChunkBuffer, 8, 0, thisChunkLines, dest.w, m_colTrans, { 0,0 });
			m_pBlitSecondPassOp(pChunkBuffer, pDst, pSource, pitchesPass2, thisChunkLines, dest.w, m_colTrans, patchPos, nullptr);

			pos.x += transformMatrix[1][0] * thisChunkLines;
			pos.y += transformMatrix[1][1] * thisChunkLines;

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

	void SoftBackend::_dummyTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp)
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

	//____ _updateBlurRadius() _____________________________________________________

	void SoftBackend::_updateBlurRadius(spx radius)
	{
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

		for (int i = 0; i < 9; i++)
		{
			m_colTrans.blurOfsPixel[i].x = (m_colTrans.blurOfsSPX[i].x) / 64;
			m_colTrans.blurOfsPixel[i].y = (m_colTrans.blurOfsSPX[i].y) / 64;
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

	//____ _scaleLineThickness() ___________________________________________________

	int SoftBackend::_scaleLineThickness(float thickness, int slope)
	{
		slope = std::abs(slope);

		int scale = s_lineThicknessTable[slope >> 12];

		if (slope < (1 << 16))
		{
			int scale2 = s_lineThicknessTable[(slope >> 12) + 1];
			scale += ((scale2 - scale) * (slope & 0xFFF)) >> 12;
		}

		return (int)(thickness * scale);
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
