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

#ifndef	WG_GFXBACKEND_DOT_H
#define WG_GFXBACKEND_DOT_H
#pragma once

#include <climits>
#include <bitset>

#include <wg_gfxdevice.h>

#include <wg_pointers.h>
#include <wg_gfxtypes.h>
#include <wg_color.h>
#include <wg_gradient.h>
#include <wg_edgemap.h>
#include <wg_tintmap.h>


#include <wg_geo.h>
#include <wg_surface.h>
#include <wg_surfacefactory.h>
#include <wg_edgemapfactory.h>
#include <wg_canvaslayers.h>

#include <vector>

namespace wg
{


	class GfxBackend;
	typedef	StrongPtr<GfxBackend>	GfxBackend_p;
	typedef	WeakPtr<GfxBackend>		GfxBackend_wp;


	//____ GfxBackend __________________________________________________________

	class GfxBackend : public Object
	{
	public:

		struct SessionInfo
		{
			// Commands 

			int		nStateChanges;		// Number of times state will change through session.
			int		nPlots;				// Number of plot commands.
			int		nLines;				// Number of line commands.
			int		nFill;
			int		nBlit;				// Includes Blit, ClipBlit och Tile.
			int		nBlur;
			int		nEdgemapDraws;

			// Total content of coord buffers

			int		nPoints;			// Total number of points to be plotted
			int		nLineCoords;		// Total number of coordinates for line commands (=nLines*2)
			int		nLineClipRects;		// Total number of clip rectangles for line commands.
			int		nRects;				// Total number of rects for fill, blit, blur and edgemapDraw commands.

			// Total size of other buffers

			int		nColors;			// Total number of colors to be used through all commands.
			int		nTransforms;		// Total number of transforms for session.
			int		nObjects;			// Total number of object references for session.
		};

		enum class Command
		{
			None,
			StateChange,
			Fill,
			Plot,
			Line,
			Blit,
			ClipBlit,					// Might be blitting from outside source surface.
			Tile,
			Blur,
			DrawEdgemap
		};

		enum class StateChange : uint8_t
		{
			BlitSource		= 1,
			BlendMode		= 1 << 1,
			TintColor		= 1 << 2,		// In GfxDevice this is used to mark any change in tint
			TintMap			= 1 << 3,		// Not used in GfxDevice.
			MorphFactor		= 1 << 4,
			FixedBlendColor	= 1 << 5,
			Blur			= 1 << 6,

			IncludeFromLayer= 1 << 7		// Used only by GfxDevice, always zero when we get the commands.
		};
		
		
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		virtual void	beginRender() = 0;
		virtual void	endRender() = 0;
		
		virtual void	beginSession( const SessionInfo * pSession ) = 0;
		virtual void	endSession() = 0;

		virtual void	setCanvas( Surface * pSurface ) = 0;
		virtual void	setCanvas( CanvasRef ref ) = 0;

		virtual void	setObjects(Object** pBeg, Object** pEnd) = 0;

		virtual void	setCoords(spx* pBeg, spx* pEnd) = 0;
		virtual void	setColors(HiColor* pBeg, HiColor* pEnd) = 0;
		virtual void	setTransforms(Transform * pBeg, Transform * pEnd) = 0;

		virtual void	processCommands( int32_t* pBeg, int32_t * pEnd) = 0;


		//.____ Misc _________________________________________________________

		virtual const CanvasInfo *	canvasInfo(CanvasRef ref) const = 0;

		virtual SurfaceFactory_p	surfaceFactory() = 0;
		virtual EdgemapFactory_p	edgemapFactory() = 0;


		virtual int		maxEdges() const = 0;

		virtual const TypeInfo& surfaceType(void) const = 0;



	protected:
		GfxBackend() {};
		virtual ~GfxBackend() {};



		static const Transform			s_blitFlipTransforms[GfxFlip_size];
		static const int				s_defaultBlur[9];
		static const spx				s_defaultBlurRadius;

	};

} // namespace wg
#endif	// WG_GFXBACKEND_DOT_H

