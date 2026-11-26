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
#ifndef	WAX_MIRRORBITMAP_DOT_H
#define	WAX_MIRRORBITMAP_DOT_H
#pragma once

#include <wax_bitmap.h>
#include <wax_streamencoder.h>
#include <wax_compression.h>


namespace wax
{

	class MirrorBitmap;
	typedef	StrongPtr<MirrorBitmap>	MirrorBitmap_p;
	typedef	WeakPtr<MirrorBitmap>	MirrorBitmap_wp;

	class MirrorBitmap : public Bitmap
	{
	public:

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Bitmap_p		bitmap;						// Required
			CanvasRef		canvasRef = CanvasRef::None;	// Stream to specified reference, otherwise create bitmap and stream to that.
			Compressor_p	compressor;
			StreamEncoder_p	encoder;						// Required
			Color			fillColor = Color::Undefined;	// What the bitmap was created filled with, for the other end to do the same.
			Finalizer_p		finalizer = nullptr;
			bool			streamOnCreate = false;			// If true, bitmap content will be streamed immediately after creation.
		};

		//.____ Creation __________________________________________

		static MirrorBitmap_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint16_t		inStreamId() const { return m_bitmapId; }
		CanvasRef		inStreamRef() const { return m_canvasRef; }

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
		bool		fill(const RectI& region, Color col) override;

		//.____ Misc __________________________________________________________

		bool		streamAsNew(StreamEncoder* pEncoder);

	protected:

		MirrorBitmap(const Blueprint& blueprint);
		~MirrorBitmap();

		void 	_sendCreateBitmap( Color fillColor = Color::Undefined );
		void	_sendDeleteBitmap();
		void	_sendPixels(int nRects, const RectI* pRects);

		StreamEncoder_p	m_pEncoder;
		Compressor_p	m_pCompressor;

		Bitmap_p		m_pBitmap;							// Bitmap we stream from.
		int				m_observerId;

		CanvasRef		m_canvasRef = CanvasRef::None;		// CanvasRef we stream to, if set.
		uint16_t		m_bitmapId = 0;
	};


}


#endif // WAX_MIRRORBITMAP_DOT_H
