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

#ifndef	WG_SURFACESTREAMER_DOT_H
#define	WG_SURFACESTREAMER_DOT_H
#pragma once

#include <wg_surface.h>
#include <wg_streamencoder.h>


namespace wg
{

	class SurfaceStreamer;
	typedef	StrongPtr<SurfaceStreamer>	SurfaceStreamer_p;
	typedef	WeakPtr<SurfaceStreamer>	SurfaceStreamer_wp;

	class SurfaceStreamer : public Object
	{
	public:

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			CanvasRef		canvasRef = CanvasRef::None;	// Stream to specified reference, otherwise create surface and stream to that.
			StreamEncoder_p	encoder;						// Required
			Finalizer_p		finalizer = nullptr;
			Surface_p		surface;						// Required
			bool			streamOnCreate = false;			// If true, surface will be streamed immediately after creation.	
		};

		//.____ Creation __________________________________________

		static SurfaceStreamer_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:

		SurfaceStreamer(const Blueprint& blueprint);
		~SurfaceStreamer();

		void	_streamPixels(int nRects, const RectSPX* pRects);

		StreamEncoder_p	m_pEncoder;

		Surface_p		m_pSurface;

		CanvasRef		m_canvasRef = CanvasRef::None;
		uint16_t		m_surfaceId = 0;
	};


}


#endif WG_SURFACESTREAMER_DOT_H
