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
#ifndef WG_SOFTSURFACE_DOT_H
#define WG_SOFTSURFACE_DOT_H
#pragma once

#include <wg_surface.h>

#include <vector>

namespace wg
{

	class SoftSurface;
	typedef	StrongPtr<SoftSurface>	SoftSurface_p;
	typedef	WeakPtr<SoftSurface>		SoftSurface_wp;

	//____ Class SoftSurface _____________________________________________________________________

	class SoftSurface : public Surface
	{
		friend class SoftBackend;
		friend class LinearBackend;
		friend class SoftGfxDevice;
		friend class LinearGfxDevice;
		friend class SoftSurfaceFactory;

	 public:

		//.____ Creation __________________________________________

		static SoftSurface_p	create(const Blueprint& blueprint);
		static SoftSurface_p	create(const Blueprint& blueprint, Blob* pBlob, int pitch = 0 );
		
		static SoftSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
									   PixelFormat format = PixelFormat::Undefined, int pitch = 0,
									   const Color8 * pPalette = nullptr, int paletteSize = 0);
		
		static SoftSurface_p	create(const Blueprint& blueprint, const uint8_t* pPixels,
									   const PixelDescription& pixelDescription, int pitch = 0,
									   const Color8 * pPalette = nullptr, int paletteSize = 0);

		static SoftSurface_p	createInPlace(const Blueprint& blueprint, uint8_t* pPixels, int pitch = 0);

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		static SizeI	maxSize();

		//.____ Content _______________________________________________________

		int				alpha( CoordSPX coord ) override;

		//.____ Control _______________________________________________________

		using Surface::allocPixelBuffer;
		using Surface::pullPixels;
		using Surface::pushPixels;

		const PixelBuffer	allocPixelBuffer(const RectI& rect) override;
		bool				pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void				pullPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void				freePixelBuffer(const PixelBuffer& buffer) override;

		//.____ Internal ______________________________________________________
		
		inline int			pitch() const { return m_pitch; }
		inline uint8_t *	pixels() const { return m_pData; }
		inline int			tileMaskX() const { return m_srcPosMaskX; }
		inline int			tileMaskY() const { return m_srcPosMaskY; }
		inline const HiColor * palette4096() const { return m_pPalette4096; }
		
		//.____ Deprecated ____________________________________________________

		void putPixels(const std::vector<int> &x, const std::vector<int> &y, const std::vector<uint32_t> &col, int length, bool replace);


	protected:
		SoftSurface(const Blueprint& blueprint);
		SoftSurface(const Blueprint& blueprint, Blob* pBlob, int pitch);
		SoftSurface(const Blueprint& blueprint, uint8_t* pPixels, int pitch);		// Create in place!
		SoftSurface(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize);
		SoftSurface(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize);

		~SoftSurface();

		void		_makePalette4096();
		void		_initTiling();

		int			m_srcPosMaskX = 0;
		int			m_srcPosMaskY = 0;

		int			m_pitch;

		Blob_p		m_pBlob;
		uint8_t*	m_pData;

		HiColor*	m_pPalette4096 = nullptr;
	};


	//========================================================================================

} // namespace wg
#endif // WG_SOFTSURFACE_DOT_H
