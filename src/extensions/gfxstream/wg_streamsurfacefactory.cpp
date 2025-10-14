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

#include <memory.h>

#include <wg_streamsurface.h>
#include <wg_streamsurfacefactory.h>
#include <wg_gfxutil.h>



namespace wg
{

	const TypeInfo StreamSurfaceFactory::TYPEINFO = { "StreamSurfaceFactory", &SurfaceFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	StreamSurfaceFactory::StreamSurfaceFactory(StreamEncoder * pEncoder) : m_pEncoder(pEncoder)
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamSurfaceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI StreamSurfaceFactory::maxSize() const
	{
		return StreamSurface::maxSize();
	}

	//____ createSurface() ________________________________________________________

	Surface_p StreamSurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		auto p = StreamSurface::create(m_pEncoder,blueprint);
		_addReference( p );
		return p;
	}

	Surface_p StreamSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		auto p = StreamSurface::create(m_pEncoder, blueprint, pBlob ,pitch );
		_addReference( p );
		return p;
	}

	Surface_p StreamSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
													 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto p = StreamSurface::create(m_pEncoder, blueprint, pPixels, format, pitch, pPalette, paletteSize);
		_addReference(p);
		return p;
	}

	Surface_p StreamSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto p = StreamSurface::create(m_pEncoder, blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize );
		_addReference(p);
		return p;
	}

	//____ setRememberSurfacesCreated() _________________________________________

	void StreamSurfaceFactory::setRememberSurfacesCreated(bool bKeep )
	{
		m_bKeepReferences = bKeep;
		if (!m_bKeepReferences)
			m_surfaceReferences.clear();
	}

	//____ surfacesCreated() __________________________________________________

	std::vector<Surface_p> StreamSurfaceFactory::surfacesCreated()
	{
		std::vector<Surface_p> vec;
		vec.reserve(m_surfaceReferences.size());

		for (auto& wp : m_surfaceReferences)
		{
			if (wp)
				vec.push_back(wp.rawPtr());
		}
		return vec;
	}

	//____ streamAllSurfacesAsNew() __________________________________________

	void StreamSurfaceFactory::streamAllSurfacesAsNew()
	{
		for (auto& wp : m_surfaceReferences)
		{
			if (wp)
				wp->streamAsNew(m_pEncoder);
		}
	}


	//____ _addReference() ____________________________________________________

	void StreamSurfaceFactory::_addReference(StreamSurface* pSurface)
	{
		if (m_bKeepReferences)
		{
			for (auto& p : m_surfaceReferences)
			{
				if (!p)
				{
					p = pSurface;
					return;
				}
			}

			m_surfaceReferences.push_back(pSurface);
		}
	}


} // namespace wg
