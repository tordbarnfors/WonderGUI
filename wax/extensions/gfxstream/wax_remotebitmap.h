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
#ifndef	WAX_REMOTEBITMAP_DOT_H
#define	WAX_REMOTEBITMAP_DOT_H
#pragma once


#include <wax_bitmap.h>
#include <wax_streamencoder.h>

#include <vector>

namespace wax
{

	class RemoteBitmap;
	typedef	StrongPtr<RemoteBitmap>	RemoteBitmap_p;
	typedef	WeakPtr<RemoteBitmap>	    RemoteBitmap_wp;

	//____ RemoteBitmap _____________________________________________________________

	class RemoteBitmap : public Bitmap
	{
		friend class RemoteBitmapFactory;
		friend class StreamDevice;

	public:

		//.____ Creation __________________________________________

		static RemoteBitmap_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint);
		static RemoteBitmap_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint, Blob* pBlob, int pitch = 0 );
		static RemoteBitmap_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
										   PixelFormat format = PixelFormat::Undefined, int pitch = 0, const Color8 * pPalette = nullptr, int paletteSize = 0);
		static RemoteBitmap_p	create(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels,
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

		bool		fill(Color col) override;
		bool		fill(const RectI& region, Color col ) override;

		//.____ Misc __________________________________________________________

		bool		recreateInStream(StreamEncoder* pEncoder);

	private:
		RemoteBitmap(StreamEncoder * pEncoder, const Blueprint& blueprint);
		RemoteBitmap(StreamEncoder * pEncoder, const Blueprint& blueprint, Blob* pBlob, int pitch = 0);
		RemoteBitmap(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize);
		RemoteBitmap(StreamEncoder * pEncoder, const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize);

		~RemoteBitmap();

		void		_initPalette(const Blueprint& blueprint);
		void		_sendCreateBitmap(StreamEncoder* pEncoder);
		void		_sendPixels(StreamEncoder* pEncoder, RectI rect, const uint8_t * pSource, int pitch);
		void		_sendDeleteBitmap();

		StreamEncoder_p	m_pEncoder;
		uint16_t		m_inStreamId;		// Id of this bitmap in the stream.
		bool			m_bDynamic;
		Color			m_fillColor = Color::Undefined;	// Set when created without pixels, sent with each CreateBitmap.
		std::vector<Color8>	m_palette;		// Our copy of the palette, which m_pPalette points into.
	};
} // namespace wax
#endif //WAX_REMOTEBITMAP_DOT_H

