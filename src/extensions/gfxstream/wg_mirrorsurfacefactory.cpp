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

#include <wg_mirrorsurface.h>
#include <wg_mirrorsurfacefactory.h>
#include <wg_gfxutil.h>



namespace wg
{

	const TypeInfo MirrorSurfaceFactory::TYPEINFO = { "MirrorSurfaceFactory", &SurfaceFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	MirrorSurfaceFactory::MirrorSurfaceFactory(StreamEncoder * pEncoder, SurfaceFactory * pBackingSurfaceFactory ) 
	: m_pEncoder(pEncoder), 
	  m_pBackingSurfaceFactory(pBackingSurfaceFactory)
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MirrorSurfaceFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI MirrorSurfaceFactory::maxSize() const
	{
		return MirrorSurface::maxSize();
	}

	//____ createSurface() ________________________________________________________

	Surface_p MirrorSurfaceFactory::createSurface(const Surface::Blueprint& blueprint)
	{
		auto pOrgSurface = m_pBackingSurfaceFactory->createSurface(blueprint);

		auto p = MirrorSurface::create( WGBP(MirrorSurface, _.encoder = m_pEncoder, _.surface = pOrgSurface ));
		_addReference( p );
		return p;
	}

	Surface_p MirrorSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		auto pOrgSurface = m_pBackingSurfaceFactory->createSurface(blueprint, pBlob, pitch);

		auto p = MirrorSurface::create(WGBP(MirrorSurface, _.encoder = m_pEncoder, _.surface = pOrgSurface, _.streamOnCreate = true));
		_addReference( p );
		return p;
	}

	Surface_p MirrorSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
													 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto pOrgSurface = m_pBackingSurfaceFactory->createSurface(blueprint, pPixels, format, pitch, pPalette, paletteSize);

		auto p = MirrorSurface::create(WGBP(MirrorSurface, _.encoder = m_pEncoder, _.surface = pOrgSurface, _.streamOnCreate = true));
		_addReference(p);
		return p;
	}

	Surface_p MirrorSurfaceFactory::createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto pOrgSurface = m_pBackingSurfaceFactory->createSurface(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);

		auto p = MirrorSurface::create(WGBP(MirrorSurface, _.encoder = m_pEncoder, _.surface = pOrgSurface, _.streamOnCreate = true));
		_addReference(p);
		return p;
	}

	//____ setRememberSurfacesCreated() _________________________________________

	void MirrorSurfaceFactory::setRememberSurfacesCreated(bool bKeep )
	{
		m_bKeepReferences = bKeep;
		if (!m_bKeepReferences)
			m_surfaceReferences.clear();
	}

	//____ surfacesCreated() __________________________________________________

	std::vector<Surface_p> MirrorSurfaceFactory::surfacesCreated()
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

	//____ streamAllAsNew() _____________________________________

	void MirrorSurfaceFactory::streamAllAsNew()
	{
		for (auto& wp : m_surfaceReferences)
		{
			auto pSurface = wp.rawPtr();
			if (pSurface)
			{
				if (pSurface)
					pSurface->streamAsNew(m_pEncoder);
			}
		}
	}


	//____ _addReference() ____________________________________________________

	void MirrorSurfaceFactory::_addReference(MirrorSurface* pSurface)
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
