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

#ifndef	WG_STREAMSURFACEFACTORY_DOT_H
#define	WG_STREAMSURFACEFACTORY_DOT_H
#pragma once

#include <wg_surfacefactory.h>

namespace wg
{

	class StreamSurfaceFactory;
	typedef	StrongPtr<StreamSurfaceFactory>		StreamSurfaceFactory_p;
	typedef	WeakPtr<StreamSurfaceFactory>	StreamSurfaceFactory_wp;

	//____ StreamSurfaceFactory _____________________________________________________

	class StreamSurfaceFactory : public SurfaceFactory
	{
	public:
		//.____ Creation __________________________________________

		static StreamSurfaceFactory_p	create( GfxOutStream& stream ) { return StreamSurfaceFactory_p(new StreamSurfaceFactory(stream)); }

		//.____ Identification __________________________________________

		bool						isInstanceOf( const char * pClassName ) const override;
		const char *				className( void ) const override;
		static const char			CLASSNAME[];

		//.____ Geometry _________________________________________________

		SizeI		maxSize() const override;

		//.____ Misc _______________________________________________________

		Surface_p	createSurface( SizeI size, PixelFormat format = PixelFormat::BGRA_8, int flags = SurfaceFlag::Static, const Color * pClut = nullptr ) const override;
		Surface_p	createSurface( SizeI size, PixelFormat format, Blob * pBlob, int pitch, int flags = SurfaceFlag::Static, const Color * pClut = nullptr ) const override;
		Surface_p	createSurface( SizeI size, PixelFormat format, uint8_t * pPixels, int pitch, const PixelDescription * pPixelDescription = 0, int flags = SurfaceFlag::Static, const Color * pClut = nullptr ) const override;
		Surface_p	createSurface( Surface * pOther, int flags = SurfaceFlag::Static ) const override;

	protected:
		StreamSurfaceFactory( GfxOutStream& stream);


		GfxOutStream_p	m_pStream;
	};
}


#endif //WG_STREAMSURFACEFACTORY_DOT_H

