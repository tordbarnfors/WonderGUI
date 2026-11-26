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
#ifndef WAX_BITMAPFILEINSPECTOR_DOT_H
#define WAX_BITMAPFILEINSPECTOR_DOT_H
#pragma once


#include <wax_bitmap.h>
#include <wax_bitmapfileheader.h>

namespace wax
{

class BitmapFileInspector;
typedef	StrongPtr<BitmapFileInspector>	BitmapFileInspector_p;
typedef	WeakPtr<BitmapFileInspector>	BitmapFileInspector_wp;

class BitmapFileInspector : public Object
{
public:

	//.____ Creation __________________________________________

	static BitmapFileInspector_p	create( const void * pBitmapFileInMemory );

	//.____ Identification _________________________________________________

	const TypeInfo&		typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	//.____ Content _________________________________________________________________

	Bitmap::Blueprint 	bitmapBlueprint();
	
	int				pixelDataOffset();
	int				pixelDataBytes();

	int				paletteDataOffset();
	int				paletteDataBytes();
	
	int				extraDataOffset();
	int				extraDataBytes();

	bool			isPixelDataCompressed();
	bool			isPaletteDataCompressed();
	bool			isExtraDataCompressed();

protected:
	BitmapFileInspector( const void * pBitmapFileInMemory );

	BitmapFileHeader	m_header;
	const uint8_t *		m_pBitmapFileInMemory;
	
};


//==============================================================================

} // namespace wax
#endif // WAX_BITMAPFILEINSPECTOR_DOT_H



