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
#include <memory.h>

#include <wax_mirrorbitmap.h>
#include <wax_mirrorbitmapfactory.h>
#include <wax_util.h>



namespace wax
{

	const TypeInfo MirrorBitmapFactory::TYPEINFO = { "MirrorBitmapFactory", &BitmapFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	MirrorBitmapFactory::MirrorBitmapFactory(StreamEncoder * pEncoder, BitmapFactory * pBackingBitmapFactory ) 
	: m_pEncoder(pEncoder), 
	  m_pBackingBitmapFactory(pBackingBitmapFactory)
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MirrorBitmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI MirrorBitmapFactory::maxSize() const
	{
		return MirrorBitmap::maxSize();
	}

	//____ createBitmap() ________________________________________________________

	Bitmap_p MirrorBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint)
	{
		auto pOrgBitmap = m_pBackingBitmapFactory->createBitmap(blueprint);

		auto p = MirrorBitmap::create( WAXBP(MirrorBitmap, _.encoder = m_pEncoder, _.fillColor = blueprint.fillColor, _.bitmap = pOrgBitmap ));
		_addReference( p );
		return p;
	}

	Bitmap_p MirrorBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		auto pOrgBitmap = m_pBackingBitmapFactory->createBitmap(blueprint, pBlob, pitch);

		auto p = MirrorBitmap::create(WAXBP(MirrorBitmap, _.encoder = m_pEncoder, _.bitmap = pOrgBitmap, _.streamOnCreate = true));
		_addReference( p );
		return p;
	}

	Bitmap_p MirrorBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
													 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto pOrgBitmap = m_pBackingBitmapFactory->createBitmap(blueprint, pPixels, format, pitch, pPalette, paletteSize);

		auto p = MirrorBitmap::create(WAXBP(MirrorBitmap, _.encoder = m_pEncoder, _.bitmap = pOrgBitmap, _.streamOnCreate = true));
		_addReference(p);
		return p;
	}

	Bitmap_p MirrorBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto pOrgBitmap = m_pBackingBitmapFactory->createBitmap(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize);

		auto p = MirrorBitmap::create(WAXBP(MirrorBitmap, _.encoder = m_pEncoder, _.bitmap = pOrgBitmap, _.streamOnCreate = true));
		_addReference(p);
		return p;
	}

	//____ setRememberBitmapsCreated() _________________________________________

	void MirrorBitmapFactory::setRememberBitmapsCreated(bool bKeep )
	{
		m_bKeepReferences = bKeep;
		if (!m_bKeepReferences)
			m_bitmapReferences.clear();
	}

	//____ bitmapsCreated() __________________________________________________

	std::vector<Bitmap_p> MirrorBitmapFactory::bitmapsCreated()
	{
		std::vector<Bitmap_p> vec;
		vec.reserve(m_bitmapReferences.size());

		for (auto& wp : m_bitmapReferences)
		{
			if (wp)
				vec.push_back(wp.rawPtr());
		}
		return vec;
	}

	//____ streamAllAsNew() _____________________________________

	void MirrorBitmapFactory::streamAllAsNew()
	{
		for (auto& wp : m_bitmapReferences)
		{
			auto pBitmap = wp.rawPtr();
			if (pBitmap)
			{
				if (pBitmap)
					pBitmap->streamAsNew(m_pEncoder);
			}
		}
	}


	//____ _addReference() ____________________________________________________

	void MirrorBitmapFactory::_addReference(MirrorBitmap* pBitmap)
	{
		if (m_bKeepReferences)
		{
			for (auto& p : m_bitmapReferences)
			{
				if (!p)
				{
					p = pBitmap;
					return;
				}
			}

			m_bitmapReferences.push_back(pBitmap);
		}
	}

} // namespace wax
