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

#include <wg_c_printer.h>
#include <wg_printer.h>

using namespace wg;

inline Printer* getPtr(wg_obj obj) {
	return static_cast<Printer*>(reinterpret_cast<Object*>(obj));
}
wg_obj wg_createPrinter()
{
  auto pPrinter = Printer::create();
  pPrinter->retain();
  return static_cast<Object*>(pPrinter);
}


void wg_setPrinterGfxDevice( wg_obj printer, wg_obj gfxDevice )
{
	auto pDevice = static_cast<GfxDevice*>(reinterpret_cast<Object*>(gfxDevice));
	getPtr(printer)->setGfxDevice(pDevice);
}

wg_obj wg_printerGfxDevice(wg_obj printer)
{
	return reinterpret_cast<wg_obj>(static_cast<Object*>(getPtr(printer)->gfxDevice().rawPtr()));
}

void wg_setPrinterFont( wg_obj printer, wg_obj font )
{
	auto pFont = static_cast<Font*>(reinterpret_cast<Object*>(font));
	getPtr(printer)->setFont(pFont);
}

wg_obj wg_printerFont(wg_obj printer)
{
	return reinterpret_cast<wg_obj>(static_cast<Object*>(getPtr(printer)->font().rawPtr()));
}


void wg_setPrinterOrigo( wg_obj printer, wg_coordSPX coord )
{
	getPtr(printer)->setCursorOrigo({coord.x, coord.y});
}

wg_coordSPX	wg_printerOrigo(wg_obj printer)
{
	auto coord = getPtr(printer)->cursorOrigo();
	return { coord.x,coord.y };
}


void wg_setPrinterLineWidth( wg_obj printer, wg_spx width )
{
	getPtr(printer)->setLineWidth(width);
}

wg_spx wg_printerLineWidth(wg_obj printer)
{
	return (wg_spx) getPtr(printer)->lineWidth();
}

void wg_setPrinterTabSize( wg_obj printer, int nbWhiteSpace)
{
	getPtr(printer)->setTabSize(nbWhiteSpace);
}

int wg_printerTabSize(wg_obj printer)
{
	return getPtr(printer)->tabSize();
}

void wg_resetPrinterCursor( wg_obj printer )
{
	getPtr(printer)->resetCursor();
}

void wg_setPrinterCursor( wg_obj printer, wg_coordSPX pos )
{
	getPtr(printer)->setCursor({pos.x,pos.y});
}


wg_coordSPX wg_printerCursor(wg_obj printer)
{
	auto coord = getPtr(printer)->cursor();
	return { coord.x,coord.y };
}

void wg_printTab(wg_obj printer)
{
	getPtr(printer)->tab();
}

void wg_printCRLF( wg_obj printer )
{
	getPtr(printer)->crlf();
}

void wg_printCRFLWithFontSizes( wg_obj printer, wg_spx maxFontSizeThisLine, wg_spx maxFontSizeNextLine )
{
	getPtr(printer)->crlf(maxFontSizeThisLine, maxFontSizeNextLine);
}

void wg_print( wg_obj printer, const char * pText )
{
	getPtr(printer)->print(pText);
}

wg_coordSPX wg_printAt( wg_obj printer, wg_coordSPX pos, const char * pText )
{
	auto coord = getPtr(printer)->printAt({pos.x,pos.y},pText);
	return { coord.x,coord.y };
}

void wg_printAligned( wg_obj printer, wg_placement xAlignment, const char * pText )
{
	getPtr(printer)->printAligned( Placement(xAlignment),pText);
}

void wg_printWrapping( wg_obj printer, const char * pText, wg_spx wrappedLinesIndent )
{
	getPtr(printer)->printWrapping( pText, wrappedLinesIndent );
}


wg_coordSPX wg_printInBox( wg_obj printer, const wg_rectSPX box, wg_placement placement, const char * pText )
{
	auto coord = getPtr(printer)->printInBox( {box.x,box.y,box.w,box.h}, Placement(placement),pText);
	return { coord.x,coord.y };
}

wg_spx wg_printerLineHeight( wg_obj printer )
{
	return getPtr(printer)->lineHeight();
}

wg_sizeSPX wg_printerTextSize( wg_obj printer, const char * pText )
{
	auto size = getPtr(printer)->textSize(pText);
	return {size.w,size.h};
}

wg_sizeSPX wg_printerWrappingTextSize( wg_obj printer, const char * pText, wg_spx wrappedLinesIndent )
{
	auto size = getPtr(printer)->wrappingTextSize(pText, wrappedLinesIndent);
	return {size.w,size.h};
}
