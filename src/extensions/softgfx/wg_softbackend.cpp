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


	//____ () _____________________________________________

	void SoftBackend::beginRender()
	{
	}

	//____ () _____________________________________________

	void SoftBackend::endRender()
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setCanvas(Surface* pSurface)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setCanvas(CanvasRef ref)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setObjects(Object** pBeg, Object** pEnd)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setCoords(spx* pBeg, spx* pEnd)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::setTransforms(Transform* pBeg, Transform* pEnd)
	{
	}

	//____ () _____________________________________________

	void SoftBackend::processCommands(int32_t* pBeg, int32_t* pEnd)
	{
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


	//____ () _____________________________________________

	int SoftBackend::maxEdges() const
	{

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




} // namespace wg
