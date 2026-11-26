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
#ifndef	WAX_MIRRORBITMAPFACTORY_DOT_H
#define	WAX_MIRRORBITMAPFACTORY_DOT_H
#pragma once

#include <wax_bitmapfactory.h>
#include <wax_streamencoder.h>

#include <vector>

namespace wax
{

	class MirrorBitmapFactory;
	typedef	StrongPtr<MirrorBitmapFactory>		MirrorBitmapFactory_p;
	typedef	WeakPtr<MirrorBitmapFactory>	MirrorBitmapFactory_wp;

	//____ MirrorBitmapFactory _____________________________________________________

	class MirrorBitmapFactory : public BitmapFactory
	{
	public:
		//.____ Creation __________________________________________

		static MirrorBitmapFactory_p	create( StreamEncoder * pEncoder, BitmapFactory * pBackingBitmapFactory ) { return MirrorBitmapFactory_p(new MirrorBitmapFactory(pEncoder,pBackingBitmapFactory)); }

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

		void		streamAllAsNew();



	protected:
		MirrorBitmapFactory( StreamEncoder * pEncoder, BitmapFactory * pBackingBitmapFactory );
		~MirrorBitmapFactory(){}

		void	_addReference(MirrorBitmap* pBitmap);

		bool							m_bKeepReferences = false;
		std::vector<MirrorBitmap_wp>	m_bitmapReferences;

		StreamEncoder_p			m_pEncoder;
		BitmapFactory_p		m_pBackingBitmapFactory;
	};
}


#endif //WAX_MIRRORBITMAPFACTORY_DOT_H

