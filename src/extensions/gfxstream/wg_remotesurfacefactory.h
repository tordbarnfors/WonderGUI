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
#ifndef	WG_REMOTESURFACEFACTORY_DOT_H
#define	WG_REMOTESURFACEFACTORY_DOT_H
#pragma once

#include <wg_surfacefactory.h>
#include <wg_streamencoder.h>

#include <vector>

namespace wg
{

	class RemoteSurfaceFactory;
	typedef	StrongPtr<RemoteSurfaceFactory>		RemoteSurfaceFactory_p;
	typedef	WeakPtr<RemoteSurfaceFactory>	RemoteSurfaceFactory_wp;

	//____ RemoteSurfaceFactory _____________________________________________________

	class RemoteSurfaceFactory : public SurfaceFactory
	{
	public:
		//.____ Creation __________________________________________

		static RemoteSurfaceFactory_p	create( StreamEncoder * pEncoder ) { return RemoteSurfaceFactory_p(new RemoteSurfaceFactory(pEncoder)); }

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

		void		recreateRemoteSurfaces();

	protected:
		RemoteSurfaceFactory( StreamEncoder * pEncoder);
		~RemoteSurfaceFactory(){}

		void	_addReference(RemoteSurface* pSurface);

		bool					m_bKeepReferences = false;
		std::vector<RemoteSurface_wp>	m_surfaceReferences;

		StreamEncoder_p	m_pEncoder;
	};
}


#endif //WG_REMOTESURFACEFACTORY_DOT_H

