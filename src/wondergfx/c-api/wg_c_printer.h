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

#ifndef WG_C_PRINTER_DOT_H
#define WG_C_PRINTER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	wg_obj		wg_createPrinter();

	void		wg_setPrinterGfxDevice( wg_obj printer, wg_obj gfxDevice );
	wg_obj		wg_printerGfxDevice( wg_obj printer );

	void		wg_setPrinterFont( wg_obj printer, wg_obj font );
	wg_obj		wg_printerFont( wg_obj printer );

	void		wg_setPrinterOrigo( wg_obj printer, wg_coordSPX coord );
	wg_coordSPX	wg_printerOrigo( wg_obj printer );

	void		wg_setPrinterLineWidth( wg_obj printer, wg_spx width );		// Needed for printJustifiedLine()
	wg_spx		wg_printerLineWidth( wg_obj printer );

	void		wg_setPrinterTabSize( wg_obj printer, int nbWhiteSpace);
	int			wg_printerTabSize(wg_obj printer);

	void		wg_resetPrinterCursor( wg_obj printer );
	void		wg_setPrinterCursor( wg_obj printer, wg_coordSPX pos );
	wg_coordSPX	wg_printerCursor(wg_obj printer);

	void		wg_printTab( wg_obj printer );
	void		wg_printCRLF( wg_obj printer );
	void		wg_printCRFLWithFontSizes( wg_obj printer, wg_spx maxFontSizeThisLine, wg_spx maxFontSizeNextLine );

	void		wg_print( wg_obj printer, const char * pText );
	void		wg_printAligned( wg_obj printer, wg_placement xAlign, const char * pText );
	void		wg_printWrapping( wg_obj printer, const char * pText, wg_spx wrappedLinesIndent );

	wg_coordSPX	wg_printAt( wg_obj printer, wg_coordSPX pos, const char * pText );
	wg_coordSPX	wg_printInBox( wg_obj printer, const wg_rectSPX box, wg_placement alignment, const char * pText );

	wg_spx		wg_printerLineHeight( wg_obj printer );
	wg_sizeSPX	wg_printerTextSize( wg_obj printer, const char * pString );

	wg_sizeSPX	wg_printerWrappingTextSize( wg_obj printer, const char * pText, wg_spx wrappedLinesIndent );


#ifdef __cplusplus
}
#endif

#endif


