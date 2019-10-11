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

#include <wg_types.h>
#include <wg_gfxdevice.h>

namespace wg
{

	class Surface;
	class Color;

	class NullGfxDevice;
	typedef	StrongPtr<NullGfxDevice>	NullGfxDevice_p;
	typedef	WeakPtr<NullGfxDevice>	NullGfxDevice_wp;

	class NullGfxDevice : public GfxDevice
	{
	public:
		//.____ Creation ________________________________________________

		static NullGfxDevice_p	create( SizeI size );

		//.____ Identification __________________________________________

		bool					isInstanceOf( const char * pClassName ) const override;
		const char *			className( void ) const override;
		static const char		CLASSNAME[];
		static NullGfxDevice_p	cast( Object * pObject );
		const char *			surfaceClassName( void ) const override;

		//.____ Misc _______________________________________________________

		SurfaceFactory_p		surfaceFactory() override;

		//.____ Geometry _________________________________________________

		bool		setCanvas(Surface * pCanvas, bool bResetClipList = true ) override;

		//.____ Rendering ________________________________________________

		void	fill(const RectI& rect, const Color& col) override;
		void	fill(const RectF& rect, const Color& col) override;

		void    plotPixels(int nCoords, const CoordI * pCoords, const Color * pColors) override;

		void	drawLine(CoordI begin, CoordI end, Color color, float thickness) override;


	protected:
		NullGfxDevice( SizeI size );
		~NullGfxDevice();

		void	_transformBlit(const RectI& dest, CoordI src, const int simpleTransform[2][2]) override;
		void	_transformBlit(const RectI& dest, CoordF src, const float complexTransform[2][2]) override;

		void	_transformDrawSegments(const RectI& dest, int nSegments, const Color * pSegmentColors, int nEdges, const int * pEdges, int edgeStripPitch, const int simpleTransform[2][2]) override;

	};


} // namespace wg
#endif //WG_NULLGFXDEVICE_DOT_H

