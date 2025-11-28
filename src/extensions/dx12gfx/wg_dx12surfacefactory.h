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
#ifndef WG_DX12SURFACEFACTORY_DOT_H
#define WG_DX12SURFACEFACTORY_DOT_H

#include <wg_dx12surface.h>
#include <wg_surfacefactory.h>

namespace wg
{

	class DX12SurfaceFactory;
	typedef	StrongPtr<DX12SurfaceFactory>	DX12SurfaceFactory_p;
	typedef	WeakPtr<DX12SurfaceFactory>	DX12SurfaceFactory_wp;

	//____ DX12SurfaceFactory _______________________________________________________

	class DX12SurfaceFactory : public SurfaceFactory
	{
	public:
		//.____ Creation __________________________________________

		static DX12SurfaceFactory_p	create() { return DX12SurfaceFactory_p(new DX12SurfaceFactory()); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		SizeI				maxSize() const override;

		//.____ Misc _______________________________________________________

		Surface_p	createSurface(const Surface::Blueprint& blueprint) override;
		Surface_p	createSurface(const Surface::Blueprint& blueprint, Blob* pBlob, int pitch) override;

		Surface_p	createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
			const PixelDescription& pixelDescription, int pitch = 0,
			const Color8* pPalette = nullptr, int paletteSize = 0) override;

		Surface_p	createSurface(const Surface::Blueprint& blueprint, const uint8_t* pPixels,
			PixelFormat format = PixelFormat::Undefined, int pitch = 0,
			const Color8* pPalette = nullptr, int paletteSize = 0) override;


	protected:
		DX12SurfaceFactory() {}
		virtual ~DX12SurfaceFactory() {}

	};

	//========================================================================================

} // namespace wg
#endif // WG_DX12SURFACEFACTORY_DOT_H
