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

#ifndef	WG_GFXDEVICE_GEN2_DOT_H
#define WG_GFXDEVICE_GEN2_DOT_H
#pragma once

#include <climits>
#include <bitset>

#include <wg_gfxdevice.h>

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

	class GfxDeviceGen2;
	typedef	StrongPtr<GfxDeviceGen2>	GfxDeviceGen2_p;
	typedef	WeakPtr<GfxDeviceGen2>	GfxDeviceGen2_wp;





	//____ GfxDeviceGen2 __________________________________________________________

	class GfxDeviceGen2 : public GfxDevice
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _____________________________________________________

		const CanvasInfo&	canvas() const override;
		const CanvasInfo	canvas(CanvasRef ref) const override = 0;

		CanvasLayers_p 		canvasLayers() const;

		//.____ Geometry _________________________________________________

		SizeSPX		canvasSize() const override;


		//.____ State _________________________________________________

		bool		setClipList(int nRectangles, const RectSPX* pRectangles) override;
		void		resetClipList() override;
		bool		pushClipList(int nRectangles, const RectSPX* pRectangles) override;
		bool		popClipList() override;

		const RectSPX* clipList() const override;
		int			clipListSize() const override;
		const RectSPX& clipBounds() const override;

		void		setTintColor(HiColor color) override;
		HiColor		tintColor() const override;

		void		setTintGradient(const RectSPX& rect, const Gradient& gradient) override;
		void		clearTintGradient() override;

		bool		setBlendMode(BlendMode blendMode) override;
		BlendMode 	blendMode() const override;

		bool		setBlitSource(Surface* pSource) override;
		Surface_p 	blitSource() const override;

		void		setMorphFactor(float factor) override;
		float		morphFactor() const override;

		void		setBlurMatrices(spx radius, const float red[9], const float green[9], const float blue[9]) override;

		void		setFixedBlendColor(HiColor color) override;
		HiColor		fixedBlendColor() const override;

		void		setRenderLayer(int layer) override;
		int			renderLayer() const override;




		//.____ Rendering ________________________________________________

		virtual bool	beginRender() override;
		virtual bool	endRender() override;
		virtual bool    isRendering() override;
		virtual bool	isIdle() override;
		virtual void	flush() override;

		inline bool     beginCanvasUpdate(CanvasRef canvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers* pLayers = nullptr, int startLayer = -1) override;
		inline bool     beginCanvasUpdate(Surface* pCanvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers* pLayers = nullptr, int startLayer = -1) override;
		virtual void    endCanvasUpdate() override;

		void			flattenLayers();

		// Draw methods.

		void	fill(HiColor color) override;
		void	fill(const RectSPX& rect, HiColor color) override {}

		void    plotPixels(int nCoords, const CoordSPX* pCoords, const HiColor* pColors) override {}

		void	drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness = 64) override {}
		void	drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness = 64) override {}

		// Blit methods

		void	blit(CoordSPX dest) override {}
		void	blit(CoordSPX dest, const RectSPX& src) override {}

		void	flipBlit(CoordSPX dest, GfxFlip flip) override {}
		void	flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip) override {}

		void	stretchBlit(const RectSPX& dest) override {}
		void	stretchBlit(const RectSPX& dest, const RectSPX& src) override {}

		void	stretchFlipBlit(const RectSPX& dest, GfxFlip flip) override {}
		void	stretchFlipBlit(const RectSPX& dest, const RectSPX& src, GfxFlip flip) override {}

		void	precisionBlit(const RectSPX& dest, const RectF& srcSPX) override {}
		void	transformBlit(const RectSPX& dest, CoordF srcSPX, const float transform[2][2]) override {}
		void	rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) override {}

		void	tile(const RectSPX& dest, CoordSPX shift = { 0,0 }) override {}
		void	flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift = { 0,0 }) override {}

		void	scaleTile(const RectSPX& dest, float scale, CoordSPX shift = { 0,0 }) override {}
		void	scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift = { 0,0 }) override {}

		void	blur(CoordSPX dest) override {}
		void	blur(CoordSPX dest, const RectSPX& src) override {}
		void	stretchBlur(const RectSPX& dest) override {}
		void	stretchBlur(const RectSPX& dest, const RectSPX& src) override {}
		void	transformBlur(const RectSPX& dest, CoordF srcSPX, const float transform[2][2]) override {}
		void	rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) override {}


		// Draw segments methods

		void	drawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor frontFill, HiColor backFill) override {}
		void	flipDrawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor frontFill, HiColor backFill, GfxFlip flip) override {}

		void	drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness = 0, HiColor outlineColor = HiColor::Black) override {}

		void	drawPieChart(const RectSPX& canvas, float start, int nSlices, const float* pSliceSizes, const HiColor* pSliceColors, float hubSize = 0.f, HiColor hubColor = HiColor::Transparent, HiColor backColor = HiColor::Transparent, bool bRectangular = false) override {}

		void	drawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, TintMode tintMode = TintMode::Flat) override {}
		void	flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode = TintMode::Flat) override {}

		void	drawEdgemap(CoordSPX dest, Edgemap* pEdgemap) override {}
		void	flipDrawEdgemap(CoordSPX dest, Edgemap* pEdgemap, GfxFlip flip) override {}



		// Special draw/blit methods

		void	blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale) override {}




	protected:
		GfxDeviceGen2();
		virtual ~GfxDeviceGen2();

		virtual bool _beginCanvasUpdate(CanvasRef ref, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer);

		// 

		virtual void	_setBuffers(CoordSPX* pCoordsBeg, CoordSPX* pCoordsEnd, int32_t* pDataBeg, int32_t* pDataEnd) = 0;
		virtual void	_processCommands( int32_t* pBeg, int32_t * pEnd) = 0;
		virtual void	_resetStates(int32_t* pBeg, int32_t* pEnd) = 0;

		void _encodeStateChanges();



		//

		const static int	c_maxSegments = 16;

		enum class Command
		{
			None,
			StateChange,
			Fill,
			FillSubPixel,				// Includes start/direction lines.
			Plot,
			LineFromTo,
			Blit,
			Blur,
			Segments,
		};

		struct ClipList
		{
			int				nRects;
			const RectSPX*	pRects;
			RectSPX			bounds;
		};

		enum class StateChange : uint8_t
		{
			BlitSource		= 1,
			BlendMode		= 1 << 1,
			TintColor		= 1 << 2,
			TintGradient	= 1 << 3,
			MorphFactor		= 1 << 4,
			FixedBlendColor	= 1 << 5,
			Blur			= 1 << 6,

			IncludeFromLayer= 1 << 7
		};


		struct RenderState
		{
			Surface_p		blitSource;

			uint8_t			stateChanges		= 0;

			HiColor			tintColor			= HiColor::Undefined;
			Gradient		tintGradient;
			RectSPX			tintGradientRect;
			bool			bTintGradient		= false;
			BlendMode		blendMode			= BlendMode::Blend;
			float			morphFactor			= 0.5f;
			HiColor			fixedBlendColor		= HiColor::White;

			spx				blurRadius			= 64;
			float			blurMtxR[9]			= { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
			float			blurMtxG[9]			= { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
			float			blurMtxB[9]			= { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
			float			blurMtxA[9]			= { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
		};


		struct RenderLayer
		{
			RenderState		currentState;

			std::vector<int>	commandBuffer;
		};


		struct RenderCanvas
		{
			CanvasInfo					info;
			ClipList					updateRects;

			int							activeLayer;
			std::vector<RenderLayer>	layers;
			std::vector<ClipList>		clipListStack;

			std::vector<CoordSPX>		coords;
			std::vector<int32_t>		data;

			CanvasLayers_p				pLayerInfo;
		};


		bool						m_bRendering = false;
		RenderState					m_renderState;
		ClipList *					m_pActiveClipList = nullptr;
		RenderLayer*				m_pActiveLayer = nullptr;
		RenderCanvas*				m_pActiveCanvas = nullptr;


		std::vector<RenderCanvas>	m_canvasStack;
		std::vector<Object*>		m_objectStack;

		static const int			s_blitFlipTransforms[GfxFlip_size][2][2];
		static const int			s_blitFlipOffsets[GfxFlip_size][2];

		CanvasInfo					m_dummyCanvasInfo;



	};

	bool GfxDeviceGen2::beginCanvasUpdate(CanvasRef ref, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer)
	{
		return _beginCanvasUpdate(ref, nullptr, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}

	bool GfxDeviceGen2::beginCanvasUpdate(Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer)
	{
		return _beginCanvasUpdate(CanvasRef::None, pCanvas, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}


} // namespace wg
#endif	// WG_GFXDEVICE_GEN2_DOT_H

