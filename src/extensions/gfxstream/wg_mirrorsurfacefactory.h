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

#ifndef	WG_MIRRORSURFACEFACTORY_DOT_H
#define	WG_MIRRORSURFACEFACTORY_DOT_H
#pragma once

#include <wg_surfacefactory.h>
#include <wg_streamencoder.h>

#include <vector>

namespace wg
{

	class MirrorSurfaceFactory;
	typedef	StrongPtr<MirrorSurfaceFactory>		MirrorSurfaceFactory_p;
	typedef	WeakPtr<MirrorSurfaceFactory>	MirrorSurfaceFactory_wp;

	//____ MirrorSurfaceFactory _____________________________________________________

	class MirrorSurfaceFactory : public SurfaceFactory
	{
	public:
		//.____ Creation __________________________________________

		static MirrorSurfaceFactory_p	create( StreamEncoder * pEncoder, SurfaceFactory * pBackingSurfaceFactory ) { return MirrorSurfaceFactory_p(new MirrorSurfaceFactory(pEncoder,pBackingSurfaceFactory)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		SizeI		maxSize() const override;

		//.____ Misc _______________________________________________________

		Surface_p	createSurface(const Surface::Blueprint& blueprint) override;
		Surface_p	createSurface(const Surface::Blueprint& blueprint, Blob* pBlob, int pitch) override;
		Surface_p	createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
								  const PixelDescription& pixelDescription, int pitch = 0, const Color8 * pPalette = nullptr, int paletteSize = 0) override;
		Surface_p	createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										  const Color8 * pPalette = nullptr, int paletteSize = 0) override;

		void		setRememberSurfacesCreated(bool bRemember);

		std::vector<Surface_p> surfacesCreated();

		void		streamAllAsNew();



	protected:
		MirrorSurfaceFactory( StreamEncoder * pEncoder, SurfaceFactory * pBackingSurfaceFactory );
		~MirrorSurfaceFactory(){}

		void	_addReference(MirrorSurface* pSurface);

		bool							m_bKeepReferences = false;
		std::vector<MirrorSurface_wp>	m_surfaceReferences;

		StreamEncoder_p			m_pEncoder;
		SurfaceFactory_p		m_pBackingSurfaceFactory;
	};
}


#endif //WG_MIRRORSURFACEFACTORY_DOT_H

