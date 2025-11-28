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

#include <wg_dx12surface.h>

namespace wg
{

	using namespace std;

	const TypeInfo DX12Surface::TYPEINFO = { "DX12Surface", &Surface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI DX12Surface::maxSize()
	{
		return SizeI(1048576, 1048576);
	}

	//____ Create ______________________________________________________________

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint)
	{
		return DX12Surface_p(new DX12Surface(blueprint));
	}

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return DX12Surface_p(new DX12Surface(blueprint, pBlob, pitch));
	}

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize)
	{
		return DX12Surface_p(new DX12Surface(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	}

	DX12Surface_p DX12Surface::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8* pPalette, int paletteSize)
	{
		return DX12Surface_p(new DX12Surface(blueprint, pPixels, format, pitch, pPalette, paletteSize));
	}

	//____ constructor ________________________________________________________________

	DX12Surface::DX12Surface(const Blueprint& bp) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
	}

	DX12Surface::DX12Surface(const Blueprint& bp, Blob* pBlob, int pitch) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
	}

	DX12Surface::DX12Surface(const Blueprint& bp, const uint8_t* pPixels,
		PixelFormat format, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
	}


	DX12Surface::DX12Surface(const Blueprint& bp, const uint8_t* pPixels,
		const PixelDescription& pixelDescription, int pitch, const Color8* pPalette, int paletteSize) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
	}

	//____ Destructor ______________________________________________________________

	DX12Surface::~DX12Surface()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DX12Surface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ alpha() _______________________________________________________________

	int DX12Surface::alpha(CoordSPX coord)
	{
		return 0;
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer DX12Surface::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer	buf;

		//TODO: Implement!!!

		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool DX12Surface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		return true;
	}

	//____ pullPixels() _______________________________________________________

	void DX12Surface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		//TODO: Implement!!!

		Surface::pullPixels(buffer, bufferRect, bAutoNotify);
	}

	//____ freePixelBuffer() __________________________________________________

	void DX12Surface::freePixelBuffer(const PixelBuffer& buffer)
	{
		// Nothing to do here.
	}

} // namespace wg
