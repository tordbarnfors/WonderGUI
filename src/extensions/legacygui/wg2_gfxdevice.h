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
#ifndef	WG2_GFXDEVICE_DOT_H
#define WG2_GFXDEVICE_DOT_H

#include <climits>

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef	WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#include <wg_textstyle.h>
#include <wg_char.h>


#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif


#include <wg_gfxdevice.h>
#include <wg2_geo.h>

//____ WgWaveLine ___________________________________________________________

typedef wg::WaveLine        WgWaveLine;

class	WgBlock;
class	WgText;
class	WgCursorInstance;
class 	WgPen;

class WgGfxDevice
{
public:



	// High-level print methods

	static bool		PrintText( wg::GfxDevice * pDevice, const WgText * pText, const WgRect& dest );

	// Low-level print methods

	static void		PrintLine( wg::GfxDevice * pDevice, WgPen& pen, const wg::TextAttr& baseAttr, const wg::Char * _pLine, int maxChars = INT_MAX, wg::State statem = wg::State::Default );

	static void     BlitBlock( wg::GfxDevice * pDevice, const WgBlock& block, const WgRect& dest, int scale, bool bTriLinear = false, float mipmapbias = 0.f );


protected:

	static void _printTextSpan(wg::GfxDevice * pDevice, WgPen& pen, const WgText * pText, int ofs, int len, bool bLineEnding);
	static void _printEllipsisTextSpan(wg::GfxDevice * pDevice, WgPen& pen, const WgText * pText, int ofs, int len, int endX);
	static void _drawTextBg(wg::GfxDevice * pDevice, const WgText * pText, const WgRect& dest);
	static void _drawTextSectionBg(wg::GfxDevice * pDevice, const WgText * pText, const WgRect& dstRect, int iStartOfs, int iEndOfs, WgColor color);
	static void _drawUnderline(wg::GfxDevice * pDevice, const WgText * pText, int _x, int _y, int ofs, int maxChars);

};

#endif	// WG2_GFXDEVICE_DOT_H

