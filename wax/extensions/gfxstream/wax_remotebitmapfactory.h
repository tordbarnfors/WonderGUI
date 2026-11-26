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
#ifndef	WAX_REMOTEBITMAPFACTORY_DOT_H
#define	WAX_REMOTEBITMAPFACTORY_DOT_H
#pragma once

#include <wax_bitmapfactory.h>
#include <wax_streamencoder.h>

#include <vector>

namespace wax
{

	class RemoteBitmapFactory;
	typedef	StrongPtr<RemoteBitmapFactory>		RemoteBitmapFactory_p;
	typedef	WeakPtr<RemoteBitmapFactory>	RemoteBitmapFactory_wp;

	//____ RemoteBitmapFactory _____________________________________________________

	class RemoteBitmapFactory : public BitmapFactory
	{
	public:
		//.____ Creation __________________________________________

		static RemoteBitmapFactory_p	create( StreamEncoder * pEncoder ) { return RemoteBitmapFactory_p(new RemoteBitmapFactory(pEncoder)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		SizeI		maxSize() const override;

		//.____ Misc _______________________________________________________

		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint) override;
		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, Blob* pBlob, int pitch) override;
		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels,
								  const PixelDescription& pixelDescription, int pitch = 0, const Color8 * pPalette = nullptr, int paletteSize = 0) override;
		Bitmap_p	createBitmap(const Bitmap::Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										  const Color8 * pPalette = nullptr, int paletteSize = 0) override;

		void		setRememberBitmapsCreated(bool bRemember);

		std::vector<Bitmap_p> bitmapsCreated();

		void		recreateRemoteBitmaps();

	protected:
		RemoteBitmapFactory( StreamEncoder * pEncoder);
		~RemoteBitmapFactory(){}

		void	_addReference(RemoteBitmap* pBitmap);

		bool					m_bKeepReferences = false;
		std::vector<RemoteBitmap_wp>	m_bitmapReferences;

		StreamEncoder_p	m_pEncoder;
	};
}


#endif //WAX_REMOTEBITMAPFACTORY_DOT_H

