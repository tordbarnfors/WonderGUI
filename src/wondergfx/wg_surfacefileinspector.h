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
#ifndef WG_SURFACEFILEINSPECTOR_DOT_H
#define WG_SURFACEFILEINSPECTOR_DOT_H
#pragma once


#include <wg_surface.h>
#include <wg_surfacefileheader.h>

namespace wg
{

class SurfaceFileInspector;
typedef	StrongPtr<SurfaceFileInspector>	SurfaceFileInspector_p;
typedef	WeakPtr<SurfaceFileInspector>	SurfaceFileInspector_wp;

class SurfaceFileInspector : public Object
{
public:

	//.____ Creation __________________________________________

	static SurfaceFileInspector_p	create( const char * pSurfaceFileInMemory );

	//.____ Identification _________________________________________________

	const TypeInfo&		typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	//.____ Content _________________________________________________________________

	Surface::Blueprint 	surfaceBlueprint();
	
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
	SurfaceFileInspector( const char * pSurfaceFileInMemory );

	SurfaceFileHeader	m_header;
	const uint8_t *		m_pSurfaceFileInMemory;
	
};


//==============================================================================

} // namespace wg
#endif // WG_SURFACEFILEINSPECTOR_DOT_H



