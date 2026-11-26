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

#include <wax_remotebitmap.h>
#include <wax_remotebitmapfactory.h>
#include <wax_util.h>



namespace wax
{

	const TypeInfo RemoteBitmapFactory::TYPEINFO = { "RemoteBitmapFactory", &BitmapFactory::TYPEINFO };

	//____ constructor ________________________________________________________

	RemoteBitmapFactory::RemoteBitmapFactory(StreamEncoder * pEncoder) : m_pEncoder(pEncoder)
	{

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RemoteBitmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ maxSize() ______________________________________________________________

	SizeI RemoteBitmapFactory::maxSize() const
	{
		return RemoteBitmap::maxSize();
	}

	//____ createBitmap() ________________________________________________________

	Bitmap_p RemoteBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint)
	{
		auto p = RemoteBitmap::create(m_pEncoder,blueprint);
		_addReference( p );
		return p;
	}

	Bitmap_p RemoteBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, Blob * pBlob, int pitch )
	{
		auto p = RemoteBitmap::create(m_pEncoder, blueprint, pBlob ,pitch );
		_addReference( p );
		return p;
	}

	Bitmap_p RemoteBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
													 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto p = RemoteBitmap::create(m_pEncoder, blueprint, pPixels, format, pitch, pPalette, paletteSize);
		_addReference(p);
		return p;
	}

	Bitmap_p RemoteBitmapFactory::createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
												const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize)
	{
		auto p = RemoteBitmap::create(m_pEncoder, blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize );
		_addReference(p);
		return p;
	}

	//____ setRememberBitmapsCreated() _________________________________________

	void RemoteBitmapFactory::setRememberBitmapsCreated(bool bKeep )
	{
		m_bKeepReferences = bKeep;
		if (!m_bKeepReferences)
			m_bitmapReferences.clear();
	}

	//____ bitmapsCreated() __________________________________________________

	std::vector<Bitmap_p> RemoteBitmapFactory::bitmapsCreated()
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

	//____ recreateRemoteBitmaps() __________________________________________

	void RemoteBitmapFactory::recreateRemoteBitmaps()
	{
		for (auto& wp : m_bitmapReferences)
		{
			if (wp)
				wp->recreateInStream(m_pEncoder);
		}
	}

	//____ _addReference() ____________________________________________________

	void RemoteBitmapFactory::_addReference(RemoteBitmap* pBitmap)
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
