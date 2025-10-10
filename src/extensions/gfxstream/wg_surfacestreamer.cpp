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

#include <wg_surfacestreamer.h>
#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo SurfaceStreamer::TYPEINFO = { "SurfaceStreamer", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	SurfaceStreamer_p SurfaceStreamer::create(const Blueprint& blueprint)
	{
		if (!blueprint.encoder)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "You must provide a StreamEncoder in the blueprint.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__););
			return nullptr;
		}

		if (!blueprint.surface)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "You must provide a Surface in the blueprint.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__););
			return nullptr;
		}


		return new SurfaceStreamer(blueprint);
	}

	//____ constructor _____________________________________________________________

	SurfaceStreamer::SurfaceStreamer(const Blueprint& blueprint)
	{
		m_pEncoder = blueprint.encoder;
		m_pSurface = blueprint.surface;
		m_canvasRef = blueprint.canvasRef;

		if( blueprint.finalizer)
		 setFinalizer( blueprint.finalizer );

		if (blueprint.streamOnCreate )
			_streamPixels(1, &RectSPX(0,0, SizeSPX(m_pSurface->pixelSize()*64)));
	}

	//____ Destructor _________________________________________________________

	SurfaceStreamer::~SurfaceStreamer()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SurfaceStreamer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _streamPixels() _________________________________________________________

	void SurfaceStreamer::_streamPixels(int nRects, const RectSPX* pRects)
	{
	}



} // namespace wg
