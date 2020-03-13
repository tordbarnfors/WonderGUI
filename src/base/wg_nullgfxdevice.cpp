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

#include <wg_nullgfxdevice.h>

namespace wg
{

	const char NullGfxDevice::CLASSNAME[] = {"NullGfxDevice"};


	//____ create() _______________________________________________________________

	NullGfxDevice_p NullGfxDevice::create( SizeI size )
	{
		return NullGfxDevice_p(new NullGfxDevice(size));
	}

	NullGfxDevice::NullGfxDevice( SizeI size ) : GfxDevice(size)
	{
	}

	NullGfxDevice::~NullGfxDevice()
	{
	}

	//____ isInstanceOf() _________________________________________________________

	bool NullGfxDevice::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;

		return GfxDevice::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * NullGfxDevice::className( void ) const
	{
		return CLASSNAME;
	}

	//____ surfaceClassName() _______________________________________________________

	const char * NullGfxDevice::surfaceClassName( void ) const
	{
		return nullptr;
	}

	//____ surfaceFactory() _______________________________________________________

	SurfaceFactory_p NullGfxDevice::surfaceFactory()
	{
		return SurfaceFactory_p();
	}

	//____ setCanvas() ________________________________________________________

	bool NullGfxDevice::setCanvas(Surface * pCanvas, bool bResetClipList )
	{
		m_pCanvas = pCanvas;
		return true;
	}


	void NullGfxDevice::fill(const RectI& rect, const Color& col)
	{
	}

	void NullGfxDevice::fill(const RectF& rect, const Color& col)
	{
	}

	void NullGfxDevice::plotPixels(int nCoords, const CoordI * pCoords, const Color * pColors)
	{
	}

	void NullGfxDevice::drawLine(CoordI begin, CoordI end, Color color, float thickness)
	{
	}

	void NullGfxDevice::_transformBlit(const RectI& dest, CoordI src, const int simpleTransform[2][2])
	{
	}

	void NullGfxDevice::_transformBlit(const RectI& dest, CoordF src, const float complexTransform[2][2])
	{
	}

	void NullGfxDevice::_transformDrawSegments(const RectI& dest, int nSegments, const Color * pSegmentColors, int nEdges, const int * pEdges, int edgeStripPitch, const int simpleTransform[2][2])
	{
	}


} // namespace wg
