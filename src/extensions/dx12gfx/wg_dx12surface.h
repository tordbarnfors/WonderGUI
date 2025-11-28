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
#ifndef WG_DX12SURFACE_DOT_H
#define WG_DX12SURFACE_DOT_H
#pragma once

#include <wg_surface.h>

#include <vector>

namespace wg
{

	class DX12Surface;
	typedef	StrongPtr<DX12Surface>	DX12Surface_p;
	typedef	WeakPtr<DX12Surface>	DX12Surface_wp;

	//____ Class DX12Surface _____________________________________________________________________

	class DX12Surface : public Surface
	{

	public:

		//.____ Creation __________________________________________

		static DX12Surface_p	create( const Blueprint& blueprint);
		static DX12Surface_p	create( const Blueprint& blueprint, Blob* pBlob, int pitch = 0);

		static DX12Surface_p	create( const Blueprint& blueprint, const uint8_t* pPixels,
										PixelFormat format = PixelFormat::Undefined, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		static DX12Surface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
										const PixelDescription& pixelDescription, int pitch = 0,
										const Color8* pPalette = nullptr, int paletteSize = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI			maxSize();

		//.____ Content _______________________________________________________

		int						alpha(CoordSPX coord) override;

		//.____ Control _______________________________________________________

		using Surface::allocPixelBuffer;
		using Surface::pullPixels;
		using Surface::pushPixels;

		const PixelBuffer		allocPixelBuffer(const RectI& rect) override;
		bool					pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void					pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify = true) override;
		void					freePixelBuffer(const PixelBuffer& buffer) override;


	protected:
		DX12Surface(const Blueprint& blueprint);
		DX12Surface(const Blueprint& blueprint, Blob* pBlob, int pitch);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize);
		DX12Surface(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize);

		~DX12Surface();

	};


	//========================================================================================

} // namespace wg
#endif // WG_DX12SURFACE_DOT_H
