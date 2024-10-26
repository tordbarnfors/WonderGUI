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

	const TypeInfo NullGfxDevice::TYPEINFO = { "NullGfxDevice", &GfxDevice::TYPEINFO };
	const TypeInfo NullGfxDevice::SURFACETYPEINFO = { "[no surface exists for NullGfxDevice]", nullptr };


	//____ create() _______________________________________________________________

	NullGfxDevice_p NullGfxDevice::create()
	{
		return NullGfxDevice_p(new NullGfxDevice());
	}

	NullGfxDevice::NullGfxDevice()
	{
	}

	NullGfxDevice::~NullGfxDevice()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& NullGfxDevice::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ surfaceType() ______________________________________________________

	const TypeInfo& NullGfxDevice::surfaceType( void ) const
	{
		return SURFACETYPEINFO;
	}

	//____ canvas() ___________________________________________________________

	const CanvasInfo NullGfxDevice::canvas(CanvasRef ref) const
	{
		return m_dummyCanvas;
	}


	//____ surfaceFactory() ___________________________________________________

	SurfaceFactory_p NullGfxDevice::surfaceFactory()
	{
		return SurfaceFactory_p();
	}

	//____ edgemapFactory() ___________________________________________________

	EdgemapFactory_p NullGfxDevice::edgemapFactory()
	{
		return EdgemapFactory_p();
	}


	void NullGfxDevice::_canvasWasChanged()
	{
	}

	void NullGfxDevice::_renderLayerWasChanged()
	{
	}

	void NullGfxDevice::fill(const RectSPX& rect, HiColor col)
	{
	}

	void NullGfxDevice::plotPixels(int nCoords, const CoordSPX * pCoords, const HiColor * pColors)
	{
	}

	void NullGfxDevice::drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness)
	{
	}

	void NullGfxDevice::flipDrawEdgemap(CoordSPX dest, Edgemap * pEdgemap, GfxFlip flip)
	{
	}

	void NullGfxDevice::_transformBlitSimple(const RectSPX& dest, CoordSPX src, const int simpleTransform[2][2], OpType type)
	{
	}

	void NullGfxDevice::_transformBlitComplex(const RectSPX& dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type)
	{
	}

	void NullGfxDevice::_transformDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdges, const int * pEdges, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2])
	{
	}


} // namespace wg
