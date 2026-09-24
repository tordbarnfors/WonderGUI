/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <wg_softbackend.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_softedgemap.h>
#include <algorithm>
#include <wg_gfxbase.h>
#include <cstring>


using namespace std;

namespace wg
{
	const TypeInfo SoftBackend::TYPEINFO = { "SoftBackend", &GfxBackend::TYPEINFO };

	int		SoftBackend::s_lineThicknessTable[17];

	alignas(kCacheLineSize) int SoftBackend::s_mulTab[256];

	alignas(kCacheLineSize) int16_t SoftBackend::s_limit4096Tab[4097 * 3];

	bool SoftBackend::s_bTablesInitialized = false;

	alignas(kCacheLineSize) const int16_t SoftBackend::s_channel_4_1[256] = { 0, 4096 * 1 / 15, 4096 * 2 / 15, 4096 * 3 / 15, 4096 * 4 / 15, 4096 * 5 / 15, 4096 * 6 / 15, 4096 * 7 / 15, 4096 * 8 / 15, 4096 * 9 / 15, 4096 * 10 / 15, 4096 * 11 / 15, 4096 * 12 / 15, 4096 * 13 / 15, 4096 * 14 / 15, 4096 * 15 / 15,
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

	alignas(kCacheLineSize) const int16_t SoftBackend::s_channel_4_2[256] = { 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15, 4096 * 0 / 15,
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


	alignas(kCacheLineSize) const int16_t SoftBackend::s_channel_5_linear[32] = { 4096 * 0 / 31, 4096 * 1 / 31, 4096 * 2 / 31, 4096 * 3 / 31, 4096 * 4 / 31, 4096 * 5 / 31, 4096 * 6 / 31, 4096 * 7 / 31, 4096 * 8 / 31, 4096 * 9 / 31, 4096 * 10 / 31, 4096 * 11 / 31, 4096 * 12 / 31, 4096 * 13 / 31, 4096 * 14 / 31, 4096 * 15 / 31,
											4096 * 16 / 31, 4096 * 17 / 31, 4096 * 18 / 31, 4096 * 19 / 31, 4096 * 20 / 31, 4096 * 21 / 31, 4096 * 22 / 31, 4096 * 23 / 31, 4096 * 24 / 31, 4096 * 25 / 31, 4096 * 26 / 31, 4096 * 27 / 31, 4096 * 28 / 31, 4096 * 29 / 31, 4096 * 30 / 31, 4096 * 31 / 31 };

	alignas(kCacheLineSize) const int16_t SoftBackend::s_channel_6_linear[64] = { 4096 * 0 / 63, 4096 * 1 / 63, 4096 * 2 / 63, 4096 * 3 / 63, 4096 * 4 / 63, 4096 * 5 / 63, 4096 * 6 / 63, 4096 * 7 / 63, 4096 * 8 / 63, 4096 * 9 / 63, 4096 * 10 / 63, 4096 * 11 / 63, 4096 * 12 / 63, 4096 * 13 / 63, 4096 * 14 / 63, 4096 * 15 / 63,
											4096 * 16 / 63, 4096 * 17 / 63, 4096 * 18 / 63, 4096 * 19 / 63, 4096 * 20 / 63, 4096 * 21 / 63, 4096 * 22 / 63, 4096 * 23 / 63, 4096 * 24 / 63, 4096 * 25 / 63, 4096 * 26 / 63, 4096 * 27 / 63, 4096 * 28 / 63, 4096 * 29 / 63, 4096 * 30 / 63, 4096 * 31 / 63,
											4096 * 32 / 63, 4096 * 33 / 63, 4096 * 34 / 63, 4096 * 35 / 63, 4096 * 36 / 63, 4096 * 37 / 63, 4096 * 38 / 63, 4096 * 39 / 63, 4096 * 40 / 63, 4096 * 41 / 63, 4096 * 42 / 63, 4096 * 43 / 63, 4096 * 44 / 63, 4096 * 45 / 63, 4096 * 46 / 63, 4096 * 47 / 63,
											4096 * 48 / 63, 4096 * 49 / 63, 4096 * 50 / 63, 4096 * 51 / 63, 4096 * 52 / 63, 4096 * 53 / 63, 4096 * 54 / 63, 4096 * 55 / 63, 4096 * 56 / 63, 4096 * 57 / 63, 4096 * 58 / 63, 4096 * 59 / 63, 4096 * 60 / 63, 4096 * 61 / 63, 4096 * 62 / 63, 4096 * 63 / 63 };

	alignas(kCacheLineSize) const int16_t SoftBackend::s_channel_5_sRGB[32] = { 0, 2, 10, 24, 45, 74, 110, 155, 208, 270, 340, 419, 508, 605, 713, 829,
														956, 1092, 1239, 1395, 1562, 1739, 1926, 2124, 2333, 2552, 2782, 3022, 3274, 3537, 3811, 4096 };

	alignas(kCacheLineSize) const int16_t SoftBackend::s_channel_6_sRGB[64] = { 0, 0, 2, 5, 10, 16, 23, 33, 44, 57, 71, 88, 107, 127, 150, 174,
														201, 230, 260, 293, 328, 365, 405, 446, 490, 536, 584, 635, 688, 743, 801, 861,
														923, 988, 1055, 1124, 1196, 1270, 1347, 1426, 1508, 1592, 1679, 1768, 1860, 1954, 2051, 2150,
														2252, 2356, 2463, 2573, 2685, 2800, 2918, 3038, 3161, 3287, 3415, 3546, 3679, 3815, 3954, 4096 };


	alignas(kCacheLineSize) const uint8_t SoftBackend::s_fast8_channel_4_1[256] = { 0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
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

	alignas(kCacheLineSize) const uint8_t SoftBackend::s_fast8_channel_4_2[256] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

	alignas(kCacheLineSize) const uint8_t SoftBackend::s_fast8_channel_5[32] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x29, 0x31, 0x39, 0x41, 0x4a, 0x52, 0x5a, 0x62, 0x6a, 0x73, 0x7b,
											0x83, 0x8b, 0x94, 0x9c, 0xa4, 0xac, 0xb4, 0xbd, 0xc5, 0xcd, 0xd5, 0xde, 0xe6, 0xee, 0xf6, 0xff };

	alignas(kCacheLineSize) const uint8_t SoftBackend::s_fast8_channel_6[64] = { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
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
			m_pCanvas->pullPixels(m_buffer,false);
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
		// WonderGFX should be able to run without RTTI.

		auto pSurface = _pSurface->isInstanceOf(SoftSurface::TYPEINFO) ? static_cast<SoftSurface*>(_pSurface) : nullptr;

		if (m_pCanvas)
		{
			m_pCanvas->pullPixels(m_buffer,false);
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

		m_tintColor = HiColor::White;
		m_tint.nLayers = 0;
		_updateTint();
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

	void SoftBackend::setObjects(Object* const * pBeg, Object* const * pEnd)
	{
		m_pObjectsBeg = pBeg;
		m_pObjectsEnd = pEnd;
		m_pObjectsPtr = pBeg;
	}

	//____ setRects() _____________________________________________

	void SoftBackend::setRects(const RectSPX* pBeg, const RectSPX* pEnd)
	{
		m_pRectsBeg = pBeg;
		m_pRectsEnd = pEnd;
		m_pRectsPtr = pBeg;
	}

	//____ setColors() _____________________________________________

	void SoftBackend::setColors(const HiColor* pBeg, const HiColor* pEnd)
	{
		m_pColorsBeg = pBeg;
		m_pColorsEnd = pEnd;
		m_pColorsPtr = pBeg;
	}

	//____ setTransforms() _____________________________________________

	void SoftBackend::setTransforms(const Transform* pBeg, const Transform* pEnd)
	{
		m_pTransformsBeg = pBeg;
		m_pTransformsEnd = pEnd;
	}

	//____ processCommands() _____________________________________________

	void SoftBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version)
	{
		const RectSPX *	pRects = m_pRectsPtr;
		const HiColor*	pColors = m_pColorsPtr;
		Object* const *	pObjects = m_pObjectsPtr;

		int 		customTransformStart = version == 1 ? GfxFlip_size : NbStandardTransforms;

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
					auto pBlitSource = static_cast<SoftSurface*>(*pObjects++);

					if (!pBlitSource || !m_pBlitSource || pBlitSource->pixelFormat() != m_pBlitSource->pixelFormat() ||
						pBlitSource->sampleMethod() != m_pBlitSource->sampleMethod())
						m_bBlitFunctionNeedsUpdate = true;

					m_pBlitSource = pBlitSource;
				}

				if (statesChanged & uint8_t(StateChange::TintColor))
				{
					m_tintColor = *pColors++;
					if (!(statesChanged & uint8_t(StateChange::Tint)))
						_updateTint();
				}

				if (statesChanged & uint8_t(StateChange::Tint))
					p = _setTint(p, pColors);

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					m_blendMode = (BlendMode)*p++;

					m_bBlitFunctionNeedsUpdate = true;
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

					const uint16_t* pRed = p;
					const uint16_t* pGreen = p + 9;
					const uint16_t* pBlue = p + 18;
					p += 27;

					_updateBlurRadius(radius);

					for (int i = 0; i < 9; i++)
					{
						m_colTrans.blurMtxR[i] = int(pRed[i]) * 2;
						m_colTrans.blurMtxG[i] = int(pGreen[i]) * 2;
						m_colTrans.blurMtxB[i] = int(pBlue[i]) * 2;
					}
				}

				// Take care of alignment

				if( (uintptr_t(p) & 0x2) == 2 )
					p++;

				break;
			}

			case Command::Fill:
			{
				int32_t nRects = *p++;

				const HiColor&  col = * pColors++;

				// Optimize calls

				BlendMode blendMode = m_blendMode;
				if (blendMode == BlendMode::Blend && col.a == 4096 && m_colTrans.bTintOpaque )
				{
					blendMode = BlendMode::Replace;
				}

				auto pKernels = m_pKernels[(int)m_pCanvas->pixelFormat()];

				// Get kernel, fall back to Flat kernels drawn in runs if GradientX kernels are missing.

				auto getKernel = [&](BlendMode mode, bool& bRuns) -> FillOp_p
				{
					bRuns = false;
					if (!pKernels)
						return nullptr;

					FillOp_p pKernel = pKernels->pFillKernels[(int)m_colTrans.mode][(int)mode];
					if (!pKernel && m_colTrans.mode == TintMode::GradientX)
					{
						pKernel = pKernels->pFillKernels[(int)TintMode::Flat][(int)mode];
						bRuns = (pKernel != nullptr);
					}
					return pKernel;
				};

				auto reportMissingKernel = [&](BlendMode mode)
				{
					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed fill operation. SoftBackend is missing fill kernel for TintMode::%s, BlendMode::%s onto surface of PixelFormat:%s.",
						toString(m_colTrans.mode),
						toString(mode),
						toString(m_pCanvas->pixelFormat()));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				};

				// Fills a rectangle of whole pixels with given kernel, taking tint into account.

				auto fillPixels = [&](FillOp_p pOp, bool bRuns, const RectI& rect, HiColor color)
				{
					_forTintSpans(rect, bRuns, [&](const RectI& sub)
					{
						uint8_t* pDst = m_pCanvasPixels + sub.y * m_canvasPitch + sub.x * m_canvasPixelBytes;
						pOp(pDst, m_canvasPixelBytes, m_canvasPitch - sub.w * m_canvasPixelBytes, sub.h, sub.w, color, m_colTrans, sub.pos());
					});
				};

				bool bRuns;
				FillOp_p pFunc = getKernel(blendMode, bRuns);

				if (!pFunc)
				{
					pRects += nRects;

					if (blendMode == BlendMode::Ignore)
						break;

					reportMissingKernel(blendMode);
					break;
				}

				for (int i = 0; i < nRects; i++)
				{
					RectI patch = * pRects++;

					if (((patch.x | patch.y | patch.w | patch.h) & 63) == 0)
					{
						// Pixel aligned fill

						fillPixels(pFunc, bRuns, patch / 64, col);
					}
					else
					{
						// Subpixel fill


						// Fill all but anti-aliased edges

						int x1 = ((patch.x + 63) >> 6);
						int y1 = ((patch.y + 63) >> 6);
						int x2 = ((patch.x + patch.w) >> 6);
						int y2 = ((patch.y + patch.h) >> 6);

						fillPixels(pFunc, bRuns, RectI(x1, y1, x2 - x1, y2 - y1), col);

						//

						BlendMode	edgeBlendMode = (blendMode == BlendMode::Replace) ? BlendMode::Blend : blendMode; // Need to blend edges and corners even if fill is replace

						bool bEdgeRuns;
						FillOp_p pEdgeFunc = getKernel(edgeBlendMode, bEdgeRuns);

						if (pEdgeFunc == nullptr)
						{
							pRects += (nRects - i - 1);

							if (blendMode == BlendMode::Ignore)
								break;

							reportMissingKernel(edgeBlendMode);
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

						auto edge = [&](int alpha, const RectI& rect)
						{
							if (alpha != 0)
							{
								HiColor color = col;
								color.a = alpha;
								fillPixels(pEdgeFunc, bEdgeRuns, rect, color);
							}
						};

						edge(aaTop, RectI(x1, pixelPatch.y, x2 - x1, 1));
						edge(aaBottom, RectI(x1, y2, x2 - x1, 1));
						edge(aaLeft, RectI(pixelPatch.x, y1, 1, y2 - y1));
						edge(aaRight, RectI(x2, y1, 1, y2 - y1));

						// Draw corner pieces

						edge(aaTopLeft, RectI(pixelPatch.x, pixelPatch.y, 1, 1));
						edge(aaTopRight, RectI(x2, pixelPatch.y, 1, 1));
						edge(aaBottomLeft, RectI(pixelPatch.x, y2, 1, 1));
						edge(aaBottomRight, RectI(x2, y2, 1, 1));
					}
				}
				break;
			}

			case Command::Line:
			{
				int32_t nClipRects = * p++;
				int32_t nLines = *p++;
				p++;

				const RectSPX * pClipRects = pRects;
				pRects += nClipRects;

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

					snprintf(errorMsg, 1024, "Failed drawLine operation. SoftBackend is missing clipLine kernel for BlendMode::%s onto surface of PixelFormat:%s.",
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
					HiColor color = *pColors++;

					HiColor fillColor = color;

					// Lines are only tinted by the flat tint color, not by Tints.

					if( m_tintColor != HiColor::White )
						fillColor = fillColor * m_tintColor;

					CoordSPX beg, end;

					auto p32 = (const spx *) p;
					beg.x = *p32++;
					beg.y = *p32++;
					end.x = *p32++;
					end.y = *p32++;
					p = (const uint16_t*) p32;

					spx thickness = * p++;
					p++;

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
							continue;								// TODO: Should stil draw the caps!

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
				auto pEdgemap = static_cast<SoftEdgemap*>(*pObjects++);

				int32_t nRects = *p++;
				int32_t	flip = *p++;
				p++;						// padding

				auto p32 = (const spx *) p;
				int32_t	destX = *p32++;
				int32_t	destY = *p32++;
				p = (const uint16_t*) p32;


				const RectSPX * pMyRects = pRects;
				pRects += nRects;

				int32_t nSegments = pEdgemap->renderSegments();		// Segments to draw. Last one extends to the end.
				
				int nEdgeStrips = pEdgemap->m_size.w + 1;

				int edgeStripPitch = pEdgemap->m_nbSegments - 1;

				//TODO: Proper 26:6 support


				auto& mtx = s_standardTransforms[flip];

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

				EdgemapTinting tinting;
				_beginEdgemapTinting(pEdgemap, nSegments, tinting);

				bool* transparentSegments = tinting.transparent;
				bool* opaqueSegments = tinting.opaque;

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

				
				StripSource stripSource = tinting.bPerPixel ? StripSource::Tintmaps : StripSource::Colors;

				SegmentOp_p	pOp = nullptr;
				auto pKernels = m_pKernels[(int)m_pCanvas->pixelFormat()];
				if (pKernels)
					pOp = pKernels->pSegmentKernels[(int)stripSource][(int)m_blendMode];

				if (pOp == nullptr)
				{
					_endEdgemapTinting(tinting);

					if (m_blendMode == BlendMode::Ignore)
						break;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed draw segments operation. SoftBackend is missing segments kernel %s tint for BlendMode::%s onto surface of PixelFormat:%s.",
						tinting.bPerPixel ? "with" : "without",
						toString(m_blendMode),
						toString(m_pCanvas->pixelFormat()));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					break;
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

						// Generate colors for column if tint varies over edgemap

						_tintEdgemapColumn(pEdgemap, nSegments, tinting, columnOfs, rowOfs, rowOfs + rows, pEdgeStrips, start, simpleTransform);

						const int16_t* pColors = &tinting.colors[skippedSegments][0];

						//

						pOp(clipBeg, clipEnd, pStripStart, rowPitch, nEdges, edges, pColors, tinting.pColumns + skippedSegments * tinting.pitch, tinting.pitch, transparentSegments + skippedSegments, opaqueSegments + skippedSegments, m_colTrans);
						pEdgeStrips += edgeStripPitch;
						pStripStart += colPitch;
						columnOfs++;
					}
				}

				// Free what we have reserved on the memStack.

				_endEdgemapTinting(tinting);

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


				for (int i = 0; i < nRects; i++)
				{
					auto p32 = (const spx *) p;
					int srcX = *p32++;
					int srcY = *p32++;
					spx dstX = *p32++;
					spx dstY = *p32++;
					p = (const uint16_t*) p32;

					int32_t transform = *p++;
					p++;							// padding

					RectI	patch = (*pRects++) / 64;

					if (transform < customTransformStart )
					{
						const Transform& mtx = s_standardTransforms[transform];

						// Step forward _src by half a pixel, so we start from correct pixel.

						srcX += (mtx.xx + mtx.yx) * 512;
						srcY += (mtx.xy + mtx.yy) * 512;

						//

						CoordI src0 = { srcX / 1024, srcY / 1024 };
						CoordI dest = { dstX / 64, dstY / 64 };

						_forTintSpans(patch, m_bBlitTintRuns, [&](const RectI& sub)
						{
							CoordI	patchOfs = sub.pos() - dest;
							CoordI	src = src0;

							src.x += patchOfs.x * mtx.xx + patchOfs.y * mtx.yx;
							src.y += patchOfs.x * mtx.xy + patchOfs.y * mtx.yy;

							if (cmd == Command::Blit)
								(this->*m_pStraightBlitOp)(sub, src, mtx, sub.pos(), m_pStraightBlitFirstPassOp);
							else if (cmd == Command::Tile)
								(this->*m_pStraightTileOp)(sub, src, mtx, sub.pos(), m_pStraightTileFirstPassOp);
							else
								(this->*m_pStraightBlurOp)(sub, src, mtx, sub.pos(), m_pStraightBlurFirstPassOp);
						});
					}
					else
					{
						binalInt mtx[2][2];

						const Transform* pTransform = &m_pTransformsBeg[transform - customTransformStart];

						mtx[0][0] = binalInt(pTransform->xx * BINAL_MUL);
						mtx[0][1] = binalInt(pTransform->xy * BINAL_MUL);
						mtx[1][0] = binalInt(pTransform->yx * BINAL_MUL);
						mtx[1][1] = binalInt(pTransform->yy * BINAL_MUL);

						//

						BinalCoord src0 = { srcX * (BINAL_MUL / 1024), srcY * (BINAL_MUL / 1024) };
						CoordI dest = { dstX / 64, dstY / 64 };

						_forTintSpans(patch, m_bBlitTintRuns, [&](const RectI& sub)
						{
							CoordI	patchOfs = sub.pos() - dest;
							BinalCoord src = src0;

							src.x += patchOfs.x * mtx[0][0] + patchOfs.y * mtx[1][0];
							src.y += patchOfs.x * mtx[0][1] + patchOfs.y * mtx[1][1];

							if( cmd == Command::Blit)
								(this->*m_pTransformBlitOp)(sub, src, mtx, sub.pos(), m_pTransformBlitFirstPassOp, cmd);
							else if (cmd == Command::ClipBlit)
								(this->*m_pTransformClipBlitOp)(sub, src, mtx, sub.pos(), m_pTransformClipBlitFirstPassOp, cmd);
							else if (cmd == Command::Tile)
								(this->*m_pTransformTileOp)(sub, src, mtx, sub.pos(), m_pTransformTileFirstPassOp, cmd);
							else
								(this->*m_pTransformBlurOp)(sub, src, mtx, sub.pos(), m_pTransformBlurFirstPassOp, cmd);
						});
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
		m_pObjectsPtr = pObjects;

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

	//____ canBeBlitSource() ___________________________________________

	bool SoftBackend::canBeBlitSource(const TypeInfo& type) const
	{
		return (type == SoftSurface::TYPEINFO);
	}

	//____ canBeCanvas() ______________________________________________

	bool SoftBackend::canBeCanvas(const TypeInfo& type) const
	{
		// Any type of surface can be a backend for software rendering as long as pixelbuffer can be reserved.

		return true;
	}

	//____ waitForCompletion() ___________________________________________________

	void SoftBackend::waitForCompletion()
	{
		return;
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

	void SoftBackend::_onePassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp, Command cmd)
	{
		const SoftSurface* pSource = m_pBlitSource;

		int dstPixelBytes = m_canvasPixelBytes;

		uint8_t* pDst = m_pCanvasPixels + dest.y * m_canvasPitch + dest.x * dstPixelBytes;

		pPassOneOp(pSource, pos, transformMatrix, pDst, dstPixelBytes, m_canvasPitch - dstPixelBytes * dest.w, dest.h, dest.w, m_colTrans, patchPos);
	}


	//____ _twoPassTransformBlit() ____________________________________________

	void SoftBackend::_twoPassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2],
		CoordI patchPos, TransformBlitOp_p pPassOneOp, Command cmd)
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
			snprintf(errorMsg, 1024, "Failed blit operation. SoftBackend is missing straight blit kernel for:\n source format = %s\n tile = %s\n tint mode = %s\n blend mode = %s\n, dest format = %s\n", toString(m_pBlitSource->pixelFormat()),
				m_pBlitSource->isTiling() ? "true" : "false",
				toString(m_colTrans.mode),
				toString(m_blendMode),
				toString(m_canvasPixelFormat));
		}

		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	//____ _dummyTransformBlit() ________________________________________________

	void SoftBackend::_dummyTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp, Command cmd)
	{
		if (m_blendMode == BlendMode::Ignore)
			return;

		const static char * commandText[4] = { "Blit", "ClipBlit", "Tile", "Blur" };

		const char * pCmd = nullptr;

		switch( cmd )
		{
			case Command::Blit:
				pCmd = commandText[0];
				break;
			case Command::ClipBlit:
				pCmd = commandText[1];
				break;
			case Command::Tile:
				pCmd = commandText[2];
				break;
			case Command::Blur:
				pCmd = commandText[3];
				break;
			default:
				assert(false);						// Should never get here.
				break;
		}

		char errorMsg[1024];
		snprintf(errorMsg, 1024, "Failed %s operation. SoftBackend is missing transform blit kernel for:\n source format = %s\n sample method = %s\n tint mode = %s\n blend mode = %s\n, dest format = %s\n",
		    pCmd,
			toString(m_pBlitSource->pixelFormat()),
			toString(m_pBlitSource->sampleMethod()),
			toString(m_colTrans.mode),
			toString(m_blendMode),
			toString(m_canvasPixelFormat));

		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);

	}

	//____ _updateBlitFunctions() _____________________________________________

	void SoftBackend::_updateBlitFunctions()
	{
		m_bBlitTintRuns = false;
		_selectBlitFunctions(m_colTrans.mode);

		// Kernel sets without GradientX kernels: use Flat kernels and draw tint in runs of same color.

		if (m_colTrans.mode == TintMode::GradientX && m_pStraightBlitOp == &SoftBackend::_dummyStraightBlit &&
			m_pBlitSource && m_pBlitSource->m_pData && m_blendMode != BlendMode::Ignore )
		{
			_selectBlitFunctions(TintMode::Flat);
			m_bBlitTintRuns = (m_pStraightBlitOp != &SoftBackend::_dummyStraightBlit);

			if (!m_bBlitTintRuns)
				_selectBlitFunctions(m_colTrans.mode);		// So that error messages report the right tint mode.
		}
	}

	//____ _selectBlitFunctions() _____________________________________________

	void SoftBackend::_selectBlitFunctions(TintMode tintMode)
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

		if (m_colTrans.bTintOpaque && (tintMode == TintMode::None || tintMode == TintMode::Flat))
		{
			// TODO: Optimize by using a lookup table.

			if (blendMode == BlendMode::Blend &&
                (srcFormat == PixelFormat::RGB_565_bigendian ||
				 srcFormat == PixelFormat::RGB_555_bigendian ||
                 srcFormat == PixelFormat::BGR_8_sRGB ||
				 srcFormat == PixelFormat::BGR_8_linear ||
                 srcFormat == PixelFormat::BGR_565_linear ||
                 srcFormat == PixelFormat::BGR_565_sRGB ||
				 srcFormat == PixelFormat::BGRX_8_sRGB ||
                 srcFormat == PixelFormat::BGRX_8_linear))
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

			m_pBlitSecondPassOp = m_pKernels[(int)dstFormat]->pStraightBlitFromBGRA8Kernels[(int)tintMode][(int)blendMode];
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

			m_pBlitSecondPassOp = m_pKernels[(int)dstFormat]->pStraightBlitFromHiColorKernels[(int)tintMode][(int)blendMode];
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

				pStraightBlitSinglePassKernel = pStraightBlitKernels[int(ReadOp::Normal)][int(tintMode)];
				pStraightTileSinglePassKernel = pStraightBlitKernels[int(ReadOp::Tile)][int(tintMode)];
				pStraightBlurSinglePassKernel = pStraightBlitKernels[int(ReadOp::Blur)][int(tintMode)];
			}

			if (transformBlitKernelsIdx > 0)
			{
				auto pTransformBlitKernels = m_singlePassTransformBlitKernels[transformBlitKernelsIdx - 1].pKernels;

				pTransformBlitSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Normal)][int(tintMode)];
				pTransformTileSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Tile)][int(tintMode)];
				pTransformClipBlitSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Clip)][int(tintMode)];
				pTransformBlurSinglePassKernel = pTransformBlitKernels[(int)sampleMethod][int(ReadOp::Blur)][int(tintMode)];
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

	//____ _setTintColor() ___________________________________________________

	void SoftBackend::_setTintColor(HiColor color)
	{
		m_tintColor = color;
		_updateTint();
	}

	//____ _setTint() __________________________________________________________

	const uint16_t* SoftBackend::_setTint(const uint16_t* p, const HiColor*& pColors)
	{
		p = TintTools::decodeTint(p, pColors, m_tint);
		_updateTint();
		return p;
	}

	//____ _updateTint() _______________________________________________________
	/*
		Recalculates everything derived from m_tintColor and m_tint: layout, TintMode,
		opacity and lookup tables. The tint color is included in the lookup tables.
	*/

	void SoftBackend::_updateTint()
	{
		m_softTint.set(m_tint, m_tintColor);

		m_colTrans.pTintAxisX = nullptr;
		m_colTrans.pTintAxisY = nullptr;
		m_colTrans.tintRect.clear();
		m_colTrans.bTintOpaque = m_softTint.isOpaque();

		if (m_softTint.isFlat())
		{
			HiColor color = m_softTint.flatColor();

			m_tintLayout = TintLayout::None;
			m_colTrans.flatTintColor = color;
			m_colTrans.mode = color.isOpaqueWhite() ? TintMode::None : TintMode::Flat;
		}
		else
		{
			m_colTrans.flatTintColor = m_tintColor;			// Overwritten per span in Vertical layout.

			if (!m_softTint.variesAlongX())
			{
				m_tintLayout = TintLayout::Vertical;
				m_colTrans.mode = TintMode::Flat;
			}
			else if (!m_softTint.variesAlongY())
			{
				m_tintLayout = TintLayout::Horizontal;
				m_colTrans.mode = TintMode::GradientX;
			}
			else
			{
				m_tintLayout = TintLayout::General;
				m_colTrans.mode = TintMode::GradientX;
			}
		}

		// Blit functions depend on tint mode and opacity.

		m_bBlitFunctionNeedsUpdate = true;
	}

	//____ _beginEdgemapTinting() _____________________________________________
	/*
		Prepares colors for drawing an edgemap: flat colors per segment if neither segments
		nor device tint vary over the edgemap, otherwise a buffer for colors per pixel that is
		filled in column by column by _tintEdgemapColumn().
	*/

	void SoftBackend::_beginEdgemapTinting(SoftEdgemap* pEdgemap, int nSegments, EdgemapTinting& t)
	{
		const SoftTint& global = m_softTint;

		t.bPerPixel = pEdgemap->hasTints() || !global.isFlat();
		t.pColumns = nullptr;
		t.pGlobal = nullptr;
		t.pitch = 0;
		t.bufferBytes = 0;

		for (int seg = 0; seg < nSegments; seg++)
		{
			const SoftTint& segTint = pEdgemap->m_segmentTints[seg];

			t.transparent[seg] = segTint.isTransparent() || global.isTransparent();
			t.opaque[seg] = segTint.isOpaque() && global.isOpaque();

			if (!t.bPerPixel)
			{
				HiColor col = pEdgemap->m_pFlatColors[seg] * global.flatColor();

				t.colors[seg][0] = col.b;
				t.colors[seg][1] = col.g;
				t.colors[seg][2] = col.r;
				t.colors[seg][3] = col.a;
			}
			else
			{
				t.colors[seg][0] = 4096;
				t.colors[seg][1] = 4096;
				t.colors[seg][2] = 4096;
				t.colors[seg][3] = 4096;
			}
		}

		if (t.bPerPixel)
		{
			int length = pEdgemap->m_size.h;

			t.pitch = length;
			t.bufferBytes = sizeof(HiColor) * (nSegments + 1) * length;
			t.pColumns = (HiColor*)GfxBase::memStackAlloc(t.bufferBytes);
			t.pGlobal = t.pColumns + nSegments * length;
		}
	}

	//____ _tintEdgemapColumn() ________________________________________________
	/*
		Generates colors for rows rowBeg -> rowEnd of one column of the edgemap, for the
		segments visible there. Segment tints are evaluated in edgemap space, the device
		tint in canvas space.

		canvasStart and simpleTransform give canvas pixel of column c, row r as:
		canvasStart + c * (simpleTransform[0][0], simpleTransform[0][1]) + r * (simpleTransform[1][0], simpleTransform[1][1])
	*/

	void SoftBackend::_tintEdgemapColumn(SoftEdgemap* pEdgemap, int nSegments, EdgemapTinting& t, int column, int rowBeg, int rowEnd,
										 const int* pEdgeStrips, CoordI canvasStart, const int simpleTransform[2][2])
	{
		if (!t.bPerPixel || rowEnd <= rowBeg)
			return;

		const SoftTint& global = m_softTint;
		int edgeStripPitch = pEdgemap->m_nbSegments - 1;

		// Device tint along the column

		bool bGlobalFlat = global.isFlat();
		HiColor globalFlat = global.flatColor();

		if (!bGlobalFlat)
		{
			int dx = simpleTransform[1][0];
			int dy = simpleTransform[1][1];
			int x = canvasStart.x + column * simpleTransform[0][0] + rowBeg * dx;
			int y = canvasStart.y + column * simpleTransform[0][1] + rowBeg * dy;

			global.generate(x, y, dx, dy, rowEnd - rowBeg, t.pGlobal + rowBeg);
		}

		// Segments

		for (int seg = 0; seg < nSegments; seg++)
		{
			if (t.transparent[seg])
				continue;

			// Rows covered by segment in this column, from edge above (if any) to edge below (if any),
			// sampled at both sides of the column. Edges are in spx, e.g. 26.6.

			int top = rowBeg;
			int bottom = rowEnd;

			if (seg > 0)
			{
				int e1 = pEdgeStrips[seg - 1];
				int e2 = pEdgeStrips[seg - 1 + edgeStripPitch];
				top = std::max(top, (std::min(e1, e2) >> 6) - 1);
			}

			if (seg < nSegments - 1)
			{
				int e1 = pEdgeStrips[seg];
				int e2 = pEdgeStrips[seg + edgeStripPitch];
				bottom = std::min(bottom, (std::max(e1, e2) >> 6) + 2);
			}

			if (top >= bottom)
				continue;

			HiColor* pOut = t.pColumns + seg * t.pitch + top;
			int length = bottom - top;

			const SoftTint& segTint = pEdgemap->m_segmentTints[seg];

			if (segTint.isFlat())
			{
				HiColor col = segTint.flatColor();

				if (bGlobalFlat)
				{
					col = col * globalFlat;
					for (int i = 0; i < length; i++)
						pOut[i] = col;
				}
				else
				{
					const HiColor* pGlobal = t.pGlobal + top;
					for (int i = 0; i < length; i++)
						pOut[i] = pGlobal[i] * col;
				}
			}
			else
			{
				segTint.generate(column, top, 0, 1, length, pOut);

				if (!bGlobalFlat)
				{
					const HiColor* pGlobal = t.pGlobal + top;
					for (int i = 0; i < length; i++)
						pOut[i] = pOut[i] * pGlobal[i];
				}
				else if (globalFlat != HiColor::White)
				{
					for (int i = 0; i < length; i++)
						pOut[i] = pOut[i] * globalFlat;
				}
			}
		}
	}

	//____ _endEdgemapTinting() ________________________________________________

	void SoftBackend::_endEdgemapTinting(EdgemapTinting& t)
	{
		if (t.bufferBytes > 0)
			GfxBase::memStackFree(t.bufferBytes);

		t.bufferBytes = 0;
	}

	//____ _tintRowBuffer() ____________________________________________________

	HiColor* SoftBackend::_tintRowBuffer(int length)
	{
		if ((int)m_tintRow.size() < length)
			m_tintRow.resize(length);

		return m_tintRow.data();
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
			format != PixelFormat::BGR_565_sRGB && format != PixelFormat::BGR_565_linear &&
			format != PixelFormat::Alpha_8 && format != PixelFormat::BGRA_4_linear &&
			format != PixelFormat::RGB_565_bigendian && format != PixelFormat::RGB_555_bigendian)
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
