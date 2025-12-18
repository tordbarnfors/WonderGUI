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
#ifndef	WG_MIRRORSURFACE_DOT_H
#define	WG_MIRRORSURFACE_DOT_H
#pragma once

#include <wg_surface.h>
#include <wg_streamencoder.h>


namespace wg
{

	class MirrorSurface;
	typedef	StrongPtr<MirrorSurface>	MirrorSurface_p;
	typedef	WeakPtr<MirrorSurface>	MirrorSurface_wp;

	class MirrorSurface : public Surface
	{
	public:

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			CanvasRef		canvasRef = CanvasRef::None;	// Stream to specified reference, otherwise create surface and stream to that.
			StreamEncoder_p	encoder;						// Required
			Finalizer_p		finalizer = nullptr;
			Surface_p		surface;						// Required
			bool			streamOnCreate = false;			// If true, surface content will be streamed immediately after creation.
		};

		//.____ Creation __________________________________________

		static MirrorSurface_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint16_t		inStreamId() const { return m_surfaceId; }
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

		bool		fill(HiColor col) override;
		bool		fill(const RectI& region, HiColor col) override;

		//.____ Misc __________________________________________________________

		bool		streamAsNew(StreamEncoder* pEncoder);

	protected:

		MirrorSurface(const Blueprint& blueprint);
		~MirrorSurface();

		void 	_sendCreateSurface();
		void	_sendDeleteSurface();
		void	_sendPixels(int nRects, const RectI* pRects);

		StreamEncoder_p	m_pEncoder;

		Surface_p		m_pSurface;							// Surface we stream from.
		int				m_observerId;

		CanvasRef		m_canvasRef = CanvasRef::None;		// CanvasRef we stream to, if set.
		uint16_t		m_surfaceId = 0;
	};


}


#endif // WG_MIRRORSURFACE_DOT_H
