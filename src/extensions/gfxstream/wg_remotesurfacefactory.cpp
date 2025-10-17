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

#include <wg_remotesurface.h>
#include <wg_remotesurfacefactory.h>
#include <wg_gfxutil.h>



namespace wg
{

	const TypeInfo RemoteSurfaceFactory::TYPEINFO = { "RemoteSurfaceFactory", &SurfaceFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	RemoteSurfaceFactory::RemoteSurfaceFactory(StreamEncoder * pEncoder) : m_pEncoder(pEncoder)
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RemoteSurfaceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI RemoteSurfaceFactory::maxSize() const
	{
		return RemoteSurface::maxSize();
	}

	//____ createSurface() ________________________________________________________

	Surface_p RemoteSurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		auto p = RemoteSurface::create(m_pEncoder,blueprint);
		_addReference( p );
		return p;
	}

	Surface_p RemoteSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		auto p = RemoteSurface::create(m_pEncoder, blueprint, pBlob ,pitch );
		_addReference( p );
		return p;
	}

	Surface_p RemoteSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
													 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto p = RemoteSurface::create(m_pEncoder, blueprint, pPixels, format, pitch, pPalette, paletteSize);
		_addReference(p);
		return p;
	}

	Surface_p RemoteSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto p = RemoteSurface::create(m_pEncoder, blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize );
		_addReference(p);
		return p;
	}

	//____ setRememberSurfacesCreated() _________________________________________

	void RemoteSurfaceFactory::setRememberSurfacesCreated(bool bKeep )
	{
		m_bKeepReferences = bKeep;
		if (!m_bKeepReferences)
			m_surfaceReferences.clear();
	}

	//____ surfacesCreated() __________________________________________________

	std::vector<Surface_p> RemoteSurfaceFactory::surfacesCreated()
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

	//____ recreateRemoteSurfaces() __________________________________________

	void RemoteSurfaceFactory::recreateRemoteSurfaces()
	{
		for (auto& wp : m_surfaceReferences)
		{
			if (wp)
				wp->recreateInStream(m_pEncoder);
		}
	}

	//____ _addReference() ____________________________________________________

	void RemoteSurfaceFactory::_addReference(RemoteSurface* pSurface)
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
