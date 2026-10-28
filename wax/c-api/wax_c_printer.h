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
#ifndef WAX_C_PRINTER_DOT_H
#define WAX_C_PRINTER_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

WAX_EXPORT wax_obj		wax_createPrinter();

WAX_EXPORT void			wax_setPrinterDevice( wax_obj printer, wax_obj device );
WAX_EXPORT wax_obj		wax_printerDevice( wax_obj printer );

WAX_EXPORT void			wax_setPrinterFont( wax_obj printer, wax_obj font );
WAX_EXPORT wax_obj		wax_printerFont( wax_obj printer );

WAX_EXPORT void			wax_setPrinterOrigo( wax_obj printer, wax_coordSPX coord );
WAX_EXPORT wax_coordSPX	wax_printerOrigo( wax_obj printer );

WAX_EXPORT void			wax_setPrinterLineWidth( wax_obj printer, wax_spx width );		// Needed for printJustifiedLine()
WAX_EXPORT wax_spx		wax_printerLineWidth( wax_obj printer );

WAX_EXPORT void		    wax_setPrinterTabSize( wax_obj printer, int nbWhiteSpace);
WAX_EXPORT int			wax_printerTabSize(wax_obj printer);

WAX_EXPORT void		    wax_resetPrinterCursor( wax_obj printer );
WAX_EXPORT void		    wax_setPrinterCursor( wax_obj printer, wax_coordSPX pos );
WAX_EXPORT wax_coordSPX	wax_printerCursor(wax_obj printer);

WAX_EXPORT void			wax_printTab( wax_obj printer );
WAX_EXPORT void			wax_printCRLF( wax_obj printer );
WAX_EXPORT void			wax_printCRFLWithFontSizes( wax_obj printer, wax_spx maxFontSizeThisLine, wax_spx maxFontSizeNextLine );

WAX_EXPORT void			wax_print( wax_obj printer, const char * pText );
WAX_EXPORT void			wax_printAligned( wax_obj printer, wax_placement xAlign, const char * pText );
WAX_EXPORT void			wax_printWrapping( wax_obj printer, const char * pText, wax_spx wrappedLinesIndent );

WAX_EXPORT wax_coordSPX	wax_printAt( wax_obj printer, wax_coordSPX pos, const char * pText );
WAX_EXPORT wax_coordSPX	wax_printInBox( wax_obj printer, const wax_rectSPX box, wax_placement alignment, const char * pText );

WAX_EXPORT wax_spx		wax_printerLineHeight( wax_obj printer );
WAX_EXPORT wax_sizeSPX	wax_printerTextSize( wax_obj printer, const char * pString );

WAX_EXPORT wax_sizeSPX	wax_printerWrappingTextSize( wax_obj printer, const char * pText, wax_spx wrappedLinesIndent );


#ifdef __cplusplus
}
#endif

#endif


