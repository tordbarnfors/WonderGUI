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
#include <wax_c_printer.h>
#include <wax_printer.h>

using namespace wax;

inline Printer* getPtr(wax_obj obj) {
	return static_cast<Printer*>(reinterpret_cast<Object*>(obj));
}
wax_obj wax_createPrinter()
{
  auto pPrinter = Printer::create();
  pPrinter->retain();
  return static_cast<Object*>(pPrinter);
}


void wax_setPrinterDevice( wax_obj printer, wax_obj device )
{
	auto pDevice = static_cast<Device*>(reinterpret_cast<Object*>(device));
	getPtr(printer)->setDevice(pDevice);
}

wax_obj wax_printerDevice(wax_obj printer)
{
	return reinterpret_cast<wax_obj>(static_cast<Object*>(getPtr(printer)->device().rawPtr()));
}

void wax_setPrinterFont( wax_obj printer, wax_obj font )
{
	auto pFont = static_cast<Font*>(reinterpret_cast<Object*>(font));
	getPtr(printer)->setFont(pFont);
}

wax_obj wax_printerFont(wax_obj printer)
{
	return reinterpret_cast<wax_obj>(static_cast<Object*>(getPtr(printer)->font().rawPtr()));
}


void wax_setPrinterOrigo( wax_obj printer, wax_coordSPX coord )
{
	getPtr(printer)->setCursorOrigo({coord.x, coord.y});
}

wax_coordSPX	wax_printerOrigo(wax_obj printer)
{
	auto coord = getPtr(printer)->cursorOrigo();
	return { coord.x,coord.y };
}


void wax_setPrinterLineWidth( wax_obj printer, wax_spx width )
{
	getPtr(printer)->setLineWidth(width);
}

wax_spx wax_printerLineWidth(wax_obj printer)
{
	return (wax_spx) getPtr(printer)->lineWidth();
}

void wax_setPrinterTabSize( wax_obj printer, int nbWhiteSpace)
{
	getPtr(printer)->setTabSize(nbWhiteSpace);
}

int wax_printerTabSize(wax_obj printer)
{
	return getPtr(printer)->tabSize();
}

void wax_resetPrinterCursor( wax_obj printer )
{
	getPtr(printer)->resetCursor();
}

void wax_setPrinterCursor( wax_obj printer, wax_coordSPX pos )
{
	getPtr(printer)->setCursor({pos.x,pos.y});
}


wax_coordSPX wax_printerCursor(wax_obj printer)
{
	auto coord = getPtr(printer)->cursor();
	return { coord.x,coord.y };
}

void wax_printTab(wax_obj printer)
{
	getPtr(printer)->tab();
}

void wax_printCRLF( wax_obj printer )
{
	getPtr(printer)->crlf();
}

void wax_printCRFLWithFontSizes( wax_obj printer, wax_spx maxFontSizeThisLine, wax_spx maxFontSizeNextLine )
{
	getPtr(printer)->crlf(maxFontSizeThisLine, maxFontSizeNextLine);
}

void wax_print( wax_obj printer, const char * pText )
{
	getPtr(printer)->print(pText);
}

wax_coordSPX wax_printAt( wax_obj printer, wax_coordSPX pos, const char * pText )
{
	auto coord = getPtr(printer)->printAt({pos.x,pos.y},pText);
	return { coord.x,coord.y };
}

void wax_printAligned( wax_obj printer, wax_placement xAlignment, const char * pText )
{
	getPtr(printer)->printAligned( Placement(xAlignment),pText);
}

void wax_printWrapping( wax_obj printer, const char * pText, wax_spx wrappedLinesIndent )
{
	getPtr(printer)->printWrapping( pText, wrappedLinesIndent );
}


wax_coordSPX wax_printInBox( wax_obj printer, const wax_rectSPX box, wax_placement placement, const char * pText )
{
	auto coord = getPtr(printer)->printInBox( {box.x,box.y,box.w,box.h}, Placement(placement),pText);
	return { coord.x,coord.y };
}

wax_spx wax_printerLineHeight( wax_obj printer )
{
	return getPtr(printer)->lineHeight();
}

wax_sizeSPX wax_printerTextSize( wax_obj printer, const char * pText )
{
	auto size = getPtr(printer)->textSize(pText);
	return {size.w,size.h};
}

wax_sizeSPX wax_printerWrappingTextSize( wax_obj printer, const char * pText, wax_spx wrappedLinesIndent )
{
	auto size = getPtr(printer)->wrappingTextSize(pText, wrappedLinesIndent);
	return {size.w,size.h};
}
