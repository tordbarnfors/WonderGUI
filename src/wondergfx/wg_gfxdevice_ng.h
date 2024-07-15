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

#ifndef	WG_GFXDEVICE_NG_DOT_H
#define WG_GFXDEVICE_NG_DOT_H
#pragma once

#include <climits>

#include <wg_pointers.h>
#include <wg_gfxtypes.h>
#include <wg_color.h>
#include <wg_gradient.h>
#include <wg_edgemap.h>


#include <wg_geo.h>
#include <wg_surface.h>
#include <wg_surfacefactory.h>
#include <wg_edgemapfactory.h>
#include <wg_canvaslayers.h>

#include <vector>

namespace wg
{

	class	Surface;
	class	LegacyCTextDisplay;
	class	CaretInstance;
	class 	Pen;

	class GfxDevice;
	typedef	StrongPtr<GfxDevice>	GfxDevice_p;
	typedef	WeakPtr<GfxDevice>	GfxDevice_wp;


	//____ WaveLine ___________________________________________________________

	struct WaveLine
	{
		int		length;
		float	thickness;
		HiColor	color;
		int *	pWave;      // Pixel offset in 24.8 format.
		int		hold;      // Value for extending the line if it is too short (or completely missing).
	};

	//____ CanvasInfo _________________________________________________________

	struct CanvasInfo
	{
		CanvasInfo() {};
		CanvasInfo( CanvasRef _ref, Surface_p _pSurface, SizeI _size, PixelFormat _format, int _scale  ) :
			ref(_ref),
			pSurface(_pSurface),
			size(_size),
			scale(_scale),
			format(_format) {}
		
		CanvasRef	ref = CanvasRef::None;
		Surface_p	pSurface = nullptr;
		SizeSPX		size;
		int			scale = 64;

		PixelFormat	format = PixelFormat::Undefined;
	};


	//____ GfxDevice __________________________________________________________

	class GfxDevice : public Object
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		virtual bool	beginRender();
		virtual bool	endRender();
		virtual bool    isRendering();
		virtual bool	isIdle();
		virtual void	flush();

		inline bool     beginCanvasUpdate(CanvasRef canvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers* pLayers = nullptr, int startLayer = -1);
		inline bool     beginCanvasUpdate(Surface* pCanvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers* pLayers = nullptr, int startLayer = -1);
		virtual void    endCanvasUpdate();

		virtual void	flattenLayers();



	protected:
		GfxDevice();
		virtual ~GfxDevice();

		const static int	c_maxSegments = 16;

		enum class OpType {
			Blit,
			Tile,
			Blur
		};

		enum class Command
		{
			None,
			SetClip,
			SetBlendMode,
			SetMorphFactor,
			SetTintColor,
			SetTintGradient,
			ClearTintGradient,
			SetBlurMatrices,
			SetBlitSource,
			Fill,
			FillSubPixel,				// Includes start/direction lines.
			Plot,
			LineFromTo,
			Blit,
			Blur,
			Segments,
		};

		struct ClipListEntry
		{
			int				nClipRects;
			const RectSPX* pClipRects;
			RectSPX			clipBounds;
		};

		struct RenderState
		{
			HiColor			tintColor;
			Gradient		tintGradient;
			RectSPX			tintGradientRect;
			bool			bTintGradient;
			BlendMode		blendMode;
			float			morphFactor;
			HiColor			fixedBlendColor;
			Surface_p		blitSource;

			spx				blurRadius = 64;

			float			blurMtxR[9];
			float			blurMtxG[9];
			float			blurMtxB[9];
			float			blurMtxA[9];
		};


		struct RenderLayer
		{
			RenderState		currentState;

			std::vector<int>	commandBuffer;
		};


		struct RenderCanvas
		{
			CanvasInfo					info;

			int							activeLayer;
			std::vector<RenderLayer>	layers;
			std::vector<ClipListEntry>	clipListStack;

			std::vector<CoordSPX>		coords;

		};


		bool						m_bRendering = false;
		RenderState					m_renderState;
		RenderLayer*				m_pActiveLayer = nullptr;


		std::vector<RenderCanvas>	m_canvasStack;

		static const int			s_blitFlipTransforms[GfxFlip_size][2][2];
		static const int			s_blitFlipOffsets[GfxFlip_size][2];


} // namespace wg
#endif	// WG_GFXDEVICE_NG_DOT_H

