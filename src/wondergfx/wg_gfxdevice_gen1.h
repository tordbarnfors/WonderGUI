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

#ifndef	WG_GFXDEVICE_GEN1_DOT_H
#define WG_GFXDEVICE_GEN1_DOT_H
#pragma once

#include <climits>

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

	class GfxDeviceGen1;
	typedef	StrongPtr<GfxDeviceGen1>	GfxDeviceGen1_p;
	typedef	WeakPtr<GfxDeviceGen1>	GfxDeviceGen1_wp;


	//____ GfxDeviceGen1 __________________________________________________________

	class GfxDeviceGen1 : public GfxDevice
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		const TypeInfo&	surfaceType( void ) const override = 0;

		//.____ Misc _______________________________________________________

		inline const CanvasInfo&	canvas() const override { return m_canvas; }
		const CanvasInfo			canvas(CanvasRef ref) const override = 0;

		inline CanvasLayers_p 		canvasLayers() const override { return m_pCanvasLayers; }

		static constexpr int		maxSegments() { return c_maxSegments; }
		
		//.____ Geometry _________________________________________________

		inline 	SizeSPX		canvasSize() const override { return m_canvas.size; }


		//.____ State _________________________________________________

		bool		setClipList(int nRectangles, const RectSPX * pRectangles) override;
		void		resetClipList() override;
		bool		pushClipList(int nRectangles, const RectSPX* pRectangles) override;
		bool		popClipList() override;

		const RectSPX*	clipList() const override { return m_pClipRects; }
		int			clipListSize() const override { return m_nClipRects; }
		const RectSPX&	clipBounds() const override { return m_clipBounds; }

		void		setTint(HiColor color) override;
		void		setTint(const RectSPX& rect, Tintmap* pTintmap) override;
		void		clearTint() override;

		bool		isTinting() const override;
		Tintmap_p	tintmap() const override;
		RectSPX		tintmapRect() const override;

		void		setTintColor( HiColor color ) override;
		HiColor		tintColor() const override { return m_tintColor; }

		void		setTintGradient(const RectSPX& rect, const Gradient& gradient) override;
		void		clearTintGradient() override;

		bool		setBlendMode( BlendMode blendMode ) override;
		BlendMode 	blendMode() const override { return m_blendMode; }

		bool		setBlitSource(Surface * pSource) override;
		Surface_p 	blitSource() const override { return m_pBlitSource; }

		void		setMorphFactor(float factor) override;
		float		morphFactor() const override { return m_morphFactor; }
		
		void		setBlurbrush(Blurbrush* pBrush) override;
		Blurbrush_p	blurbrush() const override;

		void		setFixedBlendColor( HiColor color ) override;
		HiColor		fixedBlendColor() const  override{ return m_fixedBlendColor; }

		void		setRenderLayer(int layer) override;
		int			renderLayer() const  override { return m_renderLayer; }

		
		
		
		//.____ Rendering ________________________________________________

		bool	beginRender() override;
		bool	endRender() override;
		bool    isRendering() override;
		bool	isIdle() override;
		void	flush() override;

        inline bool     beginCanvasUpdate( CanvasRef canvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers * pLayers = nullptr, int startLayer = -1 ) override;
        inline bool     beginCanvasUpdate( Surface * pCanvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers * pLayers = nullptr, int startLayer = -1 ) override;
        void    		endCanvasUpdate() override;

        
		// Draw methods.

		void	fill(HiColor color) override;
		void	fill( const RectSPX& rect, HiColor color ) override = 0;
		void	drawLine( CoordSPX begin, CoordSPX end, HiColor color, spx thickness = 64 ) override = 0;
		void	drawLine( CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness = 64 ) override;

		// Blit methods

		void	blit(CoordSPX dest) override;
		void	blit(CoordSPX dest, const RectSPX& src) override;

		void	flipBlit(CoordSPX dest, GfxFlip flip ) override;
		void	flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip ) override;

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

		void	drawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill) override;
		void	flipDrawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill, GfxFlip flip) override;

		void	drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness = 0, HiColor outlineColor = HiColor::Black) override;

		void	drawPieChart(const RectSPX& canvas, float start, int nSlices, const float * pSliceSizes, const HiColor * pSliceColors, float hubSize = 0.f, HiColor hubColor = HiColor::Transparent, HiColor backColor = HiColor::Transparent, bool bRectangular = false) override;

		void	drawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode = TintMode::Flat ) override;
		void	flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode = TintMode::Flat) override;

		void	drawEdgemap(CoordSPX dest, Edgemap * pEdgemap ) override;
		void	flipDrawEdgemap(CoordSPX dest, Edgemap * pEdgemap, GfxFlip flip)  override = 0;

				

		// Special draw/blit methods

		void	blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale) override;



	protected:
		GfxDeviceGen1();
		virtual ~GfxDeviceGen1();

		const static int	c_maxSegments = 16;

		enum class OpType {
			Blit,
			Tile,
			Blur
		};


		//

		virtual void	_canvasWasChanged() = 0;
		virtual void	_renderLayerWasChanged() = 0;	// Checked for errors before we get here.
		virtual void	_clipListWasChanged();			// Called when cliplist has been changed.

		virtual void	_transformBlitSimple(const RectSPX& dest, CoordSPX src, const int simpleTransform[2][2], OpType type ) = 0;
		virtual void	_transformBlitComplex(const RectSPX& dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type ) = 0;

		virtual void	_transformDrawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor fill, const int simpleTransform[2][2]);
		virtual void	_transformDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2]) = 0;



		void	_traceLine(int * pDest, int nPoints, const WaveLine * pWave, int offset);

		virtual bool _beginCanvasUpdate(CanvasRef ref, Surface * pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer);
		void	_clearRenderLayer();						// Initializes and possibly clear render layer. 


		void	_stretchBlitWithRigidPartX(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst);
		void	_stretchBlitWithRigidPartY(const RectSPX& src, const RectSPX& dst, spx rigidPartOfs, spx rigidPartLength, spx rigidPartLengthDst);

		//

		struct StashedClipList
		{
			int				nClipRects;
			const RectSPX * pClipRects;
			RectSPX			clipBounds;
		};

		struct StashedCanvas
		{
			CanvasInfo		canvas;
			CanvasLayers_p	pCanvasLayers;
			StashedClipList	updateRects;
			StashedClipList	clipRects;
			int				renderLayer;
			Bitmask<int>	layersInitialized;
			HiColor			tintColor;
			Gradient		tintGradient;
			RectSPX			tintGradientRect;
			bool			bTintGradient;
			BlendMode		blendMode;
			float			morphFactor;
			HiColor			fixedBlendColor;

			Surface_p		layerSurfaces[CanvasLayers::c_maxLayers];		// Should maybe be a separate stack...
		};

		std::vector<StashedCanvas>		m_canvasStack;			// Offset of layer 0 in m_canvasLayers for each recursion of canvas.
		std::vector<StashedClipList>	m_clipListStack;

		//

		static const int s_blitFlipTransforms[GfxFlip_size][2][2];
		static const int s_blitFlipOffsets[GfxFlip_size][2];
		
		CanvasInfo		m_canvas;
		CanvasInfo		m_dummyCanvas;			// Returned when calling canvas() with an undefined reference.
		CanvasLayers_p	m_pCanvasLayers;

		Surface_p		m_pBlitSource;

		Blurbrush_p		m_pBlurbrush;

		int			m_renderLayer = 0;

		Surface_p	m_layerSurfaces[CanvasLayers::c_maxLayers+1];

		const RectSPX*	m_pCanvasUpdateRects;
		int				m_nCanvasUpdateRects;
		RectI			m_canvasUpdateBounds;

		const RectSPX * m_pClipRects = nullptr;
		int			m_nClipRects = 0;
		RectSPX		m_clipBounds = { 0,0,0,0 };

		HiColor		m_tintColor = HiColor::White;		// Current Tint color.
		BlendMode	m_blendMode = BlendMode::Blend;		// Current BlendMode.
		float		m_morphFactor = 0.5f;				// Factor used for morphing in BlendMode::Morph.

		HiColor		m_fixedBlendColor = HiColor::Undefined;
		
		Gradient	m_tintGradient;
		RectSPX		m_tintGradientRect = { 0,0,0,0 };
		bool		m_bTintGradient = false;

		bool        m_bRendering = false;
		
		bool		m_bIsProxyDevice = false;		// Set by subclasses that just wrap calls and rendering is performed elsewhere.
	};


	bool GfxDeviceGen1::beginCanvasUpdate(CanvasRef ref, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer)
	{
		return _beginCanvasUpdate(ref, nullptr, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}

	bool GfxDeviceGen1::beginCanvasUpdate(Surface * pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer)
	{
		return _beginCanvasUpdate( CanvasRef::None, pCanvas, nUpdateRects, pUpdateRects, pLayers, startLayer);
	}


} // namespace wg
#endif	// WG_GFXDEVICE_DOT_H

