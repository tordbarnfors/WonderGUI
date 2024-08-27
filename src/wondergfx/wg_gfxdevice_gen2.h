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
#include <wg_gfxbackend.h>

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

		//.____ Creation __________________________________________

		static GfxDeviceGen2_p	create( GfxBackend * pBackend = nullptr );
		
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		const TypeInfo& surfaceType(void) const override;

		//.____ Misc _____________________________________________________

		bool				setBackend( GfxBackend * pBackend );
		GfxBackend_p		backend() const { return  m_pBackend; }

		const CanvasInfo&	canvas() const override;
		const CanvasInfo	canvas(CanvasRef ref) const override;

		CanvasLayers_p 		canvasLayers() const override;
		
		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

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

		void		setTint(HiColor color) override;
		void		setTint(const RectSPX& rect, Tintmap* pTintmap) override;
		void		clearTint() override;

		bool		isTinting() const override;
		HiColor		tintColor() const override;
		Tintmap_p	tintmap() const override;
		RectSPX		tintmapRect() const override;

		bool		setBlendMode(BlendMode blendMode) override;
		BlendMode 	blendMode() const override;

		bool		setBlitSource(Surface* pSource) override;
		Surface_p 	blitSource() const override;

		void		setMorphFactor(float factor) override;
		float		morphFactor() const override;

		void		setBlurbrush(Blurbrush* pBrush) override;
		Blurbrush_p	blurbrush() const override;

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
		void	fill(const RectSPX& rect, HiColor color) override;
		
		void    plotPixels(int nCoords, const CoordSPX* pCoords, const HiColor* pColors) override;

		void	drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness = 64) override;
		void	drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness = 64) override;

		// Blit methods

		void	blit(CoordSPX dest) override;
		void	blit(CoordSPX dest, const RectSPX& src) override;

		void	flipBlit(CoordSPX dest, GfxFlip flip) override;
		void	flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip) override;

		void	stretchBlit(const RectSPX& dest) override;
		void	stretchBlit(const RectSPX& dest, const RectSPX& src) override;

		void	stretchFlipBlit(const RectSPX& dest, GfxFlip flip) override;
		void	stretchFlipBlit(const RectSPX& dest, const RectSPX& src, GfxFlip flip) override;

		void	precisionBlit(const RectSPX& dest, const RectF& srcSPX) override;
		void	transformBlit(const RectSPX& dest, CoordF srcSPX, const Transform& transform) override;
		void	rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) override;

		void	tile(const RectSPX& dest, CoordSPX shift = { 0,0 }) override;
		void	flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift = { 0,0 }) override;

		void	scaleTile(const RectSPX& dest, float scale, CoordSPX shift = { 0,0 }) override;
		void	scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift = { 0,0 }) override;

		void	blur(CoordSPX dest) override;
		void	blur(CoordSPX dest, const RectSPX& src) override;
		void	stretchBlur(const RectSPX& dest) override;
		void	stretchBlur(const RectSPX& dest, const RectSPX& src) override;
		void	transformBlur(const RectSPX& dest, CoordF srcSPX, const Transform& transform) override;
		void	rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) override;


		// Draw segments methods

		void	drawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor frontFill, HiColor backFill) override {}
		void	flipDrawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor frontFill, HiColor backFill, GfxFlip flip) override {}

		void	drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness = 0, HiColor outlineColor = HiColor::Black) override {}

		void	drawPieChart(const RectSPX& canvas, float start, int nSlices, const float* pSliceSizes, const HiColor* pSliceColors, float hubSize = 0.f, HiColor hubColor = HiColor::Transparent, HiColor backColor = HiColor::Transparent, bool bRectangular = false) override {}

		void	drawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, TintMode tintMode = TintMode::Flat) override {}
		void	flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode = TintMode::Flat) override {}

		void	drawEdgemap(CoordSPX dest, Edgemap* pEdgemap) override;
		void	flipDrawEdgemap(CoordSPX dest, Edgemap* pEdgemap, GfxFlip flip) override;


		// Special draw/blit methods

		void	blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale) override;

		//.____ Deprecated _______________________________________________________________

		void		setTintColor(HiColor color) override;
		void		setTintGradient(const RectSPX& rect, const Gradient& gradient) override;
		void		clearTintGradient() override;



	protected:

		typedef GfxBackend::Command Command;
		typedef GfxBackend::StateChange StateChange;

		struct ClipList
		{
			int				nRects;
			const RectSPX*	pRects;
			RectSPX			bounds;
		};


		struct RenderState
		{
			Surface_p		blitSource;

			HiColor			tintColor = HiColor::Undefined;

			RectSPX			tintmapRect;
			Tintmap_p		pTintmap;
			BlendMode		blendMode = BlendMode::Blend;
			float			morphFactor = 0.5f;
			HiColor			fixedBlendColor = HiColor::White;

			Blurbrush_p		pBlurbrush;
		};


		struct RenderLayer
		{
			RenderState			encodedState;				// State encoded in commands

			std::vector<int>		commands;

			std::vector<spx>		coords;
			std::vector<HiColor>	colors;
		};


		struct RenderCanvas
		{
			CanvasInfo					info;
			ClipList					updateRects;

			int							activeLayer;
			std::vector<RenderLayer>	layers;
			std::vector<ClipList>		clipListStack;

			std::vector<Transform>		transforms;			// Excluding the first 12 standardard ones for flip.
			std::vector<Object*>		objects;

			CanvasLayers_p				pLayerInfo;

			RenderState					savedState;			// Saved when we begin a new canvas so we can return 
															// to our right state when that ends and we pop back.
		};

		//


		GfxDeviceGen2( GfxBackend * pBackend );
		virtual ~GfxDeviceGen2();

		bool _beginCanvasUpdate(CanvasRef ref, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer);

		void _transformBlitSimple(const RectSPX& _dest, CoordSPX src, int transformOfs, Command cmd);
		void _transformBlitComplex(const RectSPX& _dest, CoordSPX _src, const Transform& matrix, Command cmd);

		void _stretchBlitWithRigidPartX(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst);
		void _stretchBlitWithRigidPartY(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst);


		//

		void _resetState(RenderState& state);
		void _encodeStateChanges();

		//

		uint8_t						m_stateChanges = 0;

		bool						m_bRendering = false;
		RenderState					m_renderState;
		ClipList *					m_pActiveClipList = nullptr;
		RenderLayer*				m_pActiveLayer = nullptr;
		RenderCanvas*				m_pActiveCanvas = nullptr;


		std::vector<RenderCanvas>	m_canvasStack;

		static const int			s_blitFlipOffsets[GfxFlip_size][2];
		static const int			s_blitFlipTransforms[GfxFlip_size][2][2];

		CanvasInfo					m_dummyCanvasInfo;
		RectSPX						m_dummyRect;
		
		GfxBackend_p				m_pBackend;
		
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

