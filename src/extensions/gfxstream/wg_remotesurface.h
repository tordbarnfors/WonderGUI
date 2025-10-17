/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free Ioware; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Ioware Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_REMOTESURFACE_DOT_H
#define	WG_REMOTESURFACE_DOT_H
#pragma once


#include <wg_surface.h>
#include <wg_streamencoder.h>

namespace wg
{

	class RemoteSurface;
	typedef	StrongPtr<RemoteSurface>	RemoteSurface_p;
	typedef	WeakPtr<RemoteSurface>	    RemoteSurface_wp;

	//____ RemoteSurface _____________________________________________________________

	class RemoteSurface : public Surface
	{
		friend class RemoteSurfaceFactory;
		friend class StreamDevice;

	public:

		//.____ Creation __________________________________________

		static RemoteSurface_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint);
		static RemoteSurface_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint, Blob* pBlob, int pitch = 0 );
		static RemoteSurface_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
										   PixelFormat format = PixelFormat::Undefined, int pitch = 0, const Color8 * pPalette = nullptr, int paletteSize = 0);
		static RemoteSurface_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
										   const PixelDescription& pixelDescription, int pitch = 0, const Color8 * pPalette = nullptr, int paletteSize = 0);

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint16_t		inStreamId() const { return m_inStreamId; }

		//.____ Geometry _________________________________________________

		static SizeI	maxSize();

        //.____ Content _______________________________________________________

		int			alpha(CoordSPX coord) override;

		//.____ Control _______________________________________________________

		const PixelBuffer	allocPixelBuffer(const RectI& rect) override;
		bool				pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) override;
		void				pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify = true) override;
		void				freePixelBuffer(const PixelBuffer& buffer) override;

		//.____  Rendering ____________________________________________________

		bool		fill(HiColor col) override;
		bool		fill(const RectI& region, HiColor col ) override;

		//.____ Misc __________________________________________________________

		bool		recreateInStream(StreamEncoder* pEncoder);

	private:
		RemoteSurface(StreamEncoder * pEncoder, const Blueprint& blueprint);
		RemoteSurface(StreamEncoder * pEncoder, const Blueprint& blueprint, Blob* pBlob, int pitch = 0);
		RemoteSurface(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize);
		RemoteSurface(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize);

		~RemoteSurface();

		void		_sendCreateSurface(StreamEncoder* pEncoder);
		void		_sendPixels(StreamEncoder* pEncoder, RectI rect, const uint8_t * pSource, int pitch);
		void		_sendDeleteSurface();

		StreamEncoder_p	m_pEncoder;
		uint16_t		m_inStreamId;		// Id of this surface in the stream.
		bool			m_bDynamic;
	};
} // namespace wg
#endif //WG_REMOTESURFACE_DOT_H

