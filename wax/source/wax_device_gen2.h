/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#ifndef	WAX_DEVICE_GEN2_DOT_H
#define WAX_DEVICE_GEN2_DOT_H
#pragma once

#include <climits>
#include <bitset>

#include <wax_device.h>
#include <wax_backend.h>

#include <wax_pointers.h>
#include <wax_types.h>
#include <wax_color.h>
#include <wax_edgemap.h>
#include <wax_tint.h>


#include <wax_geo.h>
#include <wax_surface.h>
#include <wax_surfacefactory.h>
#include <wax_edgemapfactory.h>
#include <wax_canvaslayers.h>
#include <wax_dynamicbuffer.h>

#include <vector>
#include <deque>

namespace wax
{

	class	Surface;
	class	LegacyCTextDisplay;
	class	CaretInstance;
	class 	Pen;

	class DeviceGen2;
	typedef	StrongPtr<DeviceGen2>	DeviceGen2_p;
	typedef	WeakPtr<DeviceGen2>	DeviceGen2_wp;



	//____ DeviceGen2 __________________________________________________________

	class DeviceGen2 : public Device
	{
	public:

		//.____ Creation __________________________________________

		static DeviceGen2_p	create( Backend * pBackend = nullptr );
		
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _____________________________________________________

		bool				setBackend( Backend * pBackend );
		Backend_p		backend() const { return  m_pBackend; }

		const CanvasInfo&	canvas() const override;
		const CanvasInfo	canvas(CanvasRef ref) const override;

		CanvasLayers_p 		canvasLayers() const override;
		
		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

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
		void		clearTintColor() override;
		bool		hasTintColor() const override;
		HiColor		tintColor() const override;


		void		setTint(const RectSPX& rect, Tint* pTint) override;
		void		clearTint() override;
		bool		hasTint() const override;
		Tint_p		tint() const override;
		RectSPX		tintRect() const override;

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

		//.____ Misc ________________________________________________________________

		void		setSubPixelFill(bool bSubPixelFill);
		bool		subPixelFill() const { return m_bSubPixelFill; }

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
		void			clearLayers();

		// Draw methods.

		void	fill(HiColor color) override;
		void	fill(const RectSPX& rect, HiColor color) override;
		
		void	drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness = 64, LineCaps caps = LineCaps()) override;
		void	drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness = 64, LineCaps caps = LineCaps()) override;

		// Shapes

		void	drawRoundedBox(const RoundedBoxSpec& spec) override;
		void	drawRing(const RingSpec& spec) override;

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


		// Draw edgemap methods

		void	drawEdgemap(CoordSPX dest, Edgemap* pEdgemap) override;
		void	flipDrawEdgemap(CoordSPX dest, Edgemap* pEdgemap, GfxFlip flip) override;

		void	tileEdgemap(const RectSPX& dest, Edgemap* pEdgemap, CoordSPX shift = { 0,0 }) override;
		void	flipTileEdgemap(const RectSPX& dest, Edgemap* pEdgemap, GfxFlip flip, CoordSPX shift = { 0,0 }) override;

		// Warp methods

		void	warpBlit(CoordSPX dest, Warpmap* pWarpmap, CoordSPX sourceOffset = { 0,0 }) override;
		void	warpBlit(CoordSPX dest, Warpmap* pWarpmap, const RectSPX& source, CoordSPX sourceOffset = { 0,0 }) override;
		void	warpDrawEdgemap(CoordSPX dest, Warpmap* pWarpmap, Edgemap* pEdgemap, CoordSPX sourceOffset = { 0,0 }) override;


		// Special draw/blit methods

		void	blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale) override;

		//.____ Deprecated _______________________________________________________________


		void		drawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor fill) override;
		void		flipDrawWave(const RectSPX& dest, const WaveLine* pTopBorder, const WaveLine* pBottomBorder, HiColor fill, GfxFlip flip) override;

		void		drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness = 0, HiColor outlineColor = HiColor::Black) override;

		void		drawPieChart(const RectSPX& canvas, float start, int nSlices, const float* pSliceSizes, const HiColor* pSliceColors, float hubSize = 0.f, HiColor hubColor = HiColor::Transparent, HiColor backColor = HiColor::Transparent, bool bRectangular = false) override;

		void		drawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, TintMode tintMode = TintMode::Flat) override;
		void		flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode = TintMode::Flat) override;



	protected:

		typedef Backend::Command Command;
		typedef Backend::StateChange StateChange;

		struct ClipList
		{
			int				nRects;
			const RectSPX*	pRects;
			RectSPX			bounds;
		};


		struct RenderState
		{
			Surface_p		blitSource;
			Tint_p			pTint;
			Blurbrush_p		pBlurbrush;

			HiColor			tintColor = HiColor::Undefined;
			HiColor			fixedBlendColor = HiColor::White;

			RectSPX			tintRect;
			BlendMode		blendMode = BlendMode::Blend;
			float			morphFactor = 0.5f;
		};


		// Remembers where in the object table of a layer an object was put, so that it is
		// added only once, see _objectIndex(). Text alone switches blit source between a
		// few glyph cache surfaces for almost every glyph.
		//
		// The cache is direct mapped and an entry is simply replaced when another object
		// lands on it. That can only make an object appear twice in the table, which costs
		// a little but is never wrong. Entries are valid when their generation matches the
		// layer's, which is stepped instead of clearing the cache with every session.

		struct ObjectCacheEntry
		{
			Object*		pObject = nullptr;
			uint32_t	index = 0;
			uint32_t	generation = 0;
		};

		static const int		c_objectCacheBits = 6;		// 64 entries.

		struct RenderLayer
		{
			RenderState				encodedState;				// State encoded in commands

			DynamicBuffer			commands;

			std::vector<RectSPX>	rects;
			std::vector<HiColor>	colors;
			std::vector<Object*>	objects;					// Object table, retained, referred to by index from commands.

			ObjectCacheEntry		objectCache[1 << c_objectCacheBits];
			uint32_t				objectCacheGeneration = 1;

			int						finalCommandsOfs = -1;
			Surface_p				pLayerCanvas;				// Null for layer 0 or any layer rendering directly to layer 0.

			Command					latestCommand = Command::None;
			int						latestCommandOfs = -1;
		};


		struct RenderCanvas
		{
			CanvasInfo					info;
			ClipList					updateRects;

			int							activeLayer;
			std::vector<RenderLayer>	layers;
			std::vector<ClipList>		clipListStack;

			std::vector<Transform>		transforms;			// Excluding the first 15 standardard ones for flip and flood.

			CanvasLayers_p				pLayerInfo;

			RenderState					savedState;			// Saved when we begin a new canvas so we can return 
															// to our right state when that ends and we pop back.
			uint8_t						savedStateChanges;
			
			Backend::SessionInfo		sessionInfo;
		};

		//


		DeviceGen2( Backend * pBackend );
		virtual ~DeviceGen2();

		bool _beginCanvasUpdate(CanvasRef ref, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer);

		void _transformBlitSimple(const RectSPX& _dest, CoordSPX src, int transformOfs, Command cmd);
		void _transformBlitComplex(const RectSPX& _dest, CoordI _src, const Transform& matrix, Command cmd);

		void _encodeEdgemapDraw(const RectSPX& area, CoordSPX origo, Edgemap* pEdgemap, GfxFlip flip, Command cmd);
		void _encodeWarp(CoordSPX dest, Warpmap* pWarpmap, Edgemap* pEdgemap, const RectSPX* pSource, CoordSPX sourceOffset);

		void _stretchBlitWithRigidPartX(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst);
		void _stretchBlitWithRigidPartY(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst);

		bool _isShapeColorDrawn(HiColor color) const;
		int _addShapeClipRects(const RectSPX& area);

		//

		void _resetState(RenderState& state);
		void _encodeStateChanges();
		uint32_t _objectIndex(Object* pObject);		// Index in the active layer's object table, adds the object if needed.
		void _doFlattenLayers();
		void _resetCanvas();

		//

		uint8_t						m_stateChanges = 0;

		bool						m_bRendering = false;
		bool						m_bSubPixelFill = true;

		RenderState					m_renderState;
		ClipList *					m_pActiveClipList = nullptr;
		RenderLayer*				m_pActiveLayer = nullptr;
		RenderCanvas*				m_pActiveCanvas = nullptr;


		std::deque<RenderCanvas>	m_canvasStack;		// We use deque since we have pointers directly into content.

		static const int			s_blitFlipOffsets[GfxFlip_size][2];
		static const int			s_blitFlipTransforms[GfxFlip_size][2][2];

		CanvasInfo					m_dummyCanvasInfo;
		RectSPX						m_dummyRect;
		
		Backend_p				m_pBackend;
	};

	bool DeviceGen2::beginCanvasUpdate(CanvasRef ref, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer)
	{
		return _beginCanvasUpdate(ref, nullptr, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}

	bool DeviceGen2::beginCanvasUpdate(Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers* pLayers, int startLayer)
	{
		return _beginCanvasUpdate(CanvasRef::None, pCanvas, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}


} // namespace wax
#endif	// WAX_DEVICE_GEN2_DOT_H

