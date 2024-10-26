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

#ifndef WG_NULLGFXDEVICE_DOT_H
#define WG_NULLGFXDEVICE_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_gfxdevice_gen1.h>

namespace wg
{

	class Surface;

	class NullGfxDevice;
	typedef	StrongPtr<NullGfxDevice>	NullGfxDevice_p;
	typedef	WeakPtr<NullGfxDevice>	NullGfxDevice_wp;

	class NullGfxDevice : public GfxDeviceGen1
	{
	public:
		//.____ Creation ________________________________________________

		static NullGfxDevice_p	create();

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		const TypeInfo&			surfaceType( void ) const override;

		//.____ Misc _______________________________________________________

		using GfxDevice::canvas;
		const CanvasInfo canvas(CanvasRef ref) const override;

		SurfaceFactory_p		surfaceFactory() override;
		EdgemapFactory_p		edgemapFactory() override;

		//.____ Rendering ________________________________________________

		void	fill(const RectSPX& rect, HiColor col) override;

		void    plotPixels(int nCoords, const CoordSPX * pCoords, const HiColor * pColors) override;

		void	drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness) override;

		void	flipDrawEdgemap(CoordSPX dest, Edgemap * pEdgemap, GfxFlip flip) override;


	protected:
		NullGfxDevice();
		~NullGfxDevice();

		void	_canvasWasChanged() override;
		void	_renderLayerWasChanged() override;	// Checked for errors before we get here.

		void	_transformBlitSimple(const RectSPX& dest, CoordSPX src, const int simpleTransform[2][2], OpType type) override;
		void	_transformBlitComplex(const RectSPX& dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type) override;

		void	_transformDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdges, const int * pEdges, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2]) override;

		const static TypeInfo	SURFACETYPEINFO;

	};


} // namespace wg
#endif //WG_NULLGFXDEVICE_DOT_H

