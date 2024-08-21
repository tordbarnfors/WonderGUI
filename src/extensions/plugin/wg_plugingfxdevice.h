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
#ifndef WG_PLUGINGFXDEVICE_DOT_H
#define WG_PLUGINGFXDEVICE_DOT_H
#pragma once

#include <wg_geo.h>

#include <wg_gfxdevice_gen1.h>
#include <wg_pluginsurfacefactory.h>
#include <wg_pluginedgemapfactory.h>

typedef void* wg_obj;

namespace wg
{
	class PluginSurface;
	class PluginGfxDevice;
	typedef	StrongPtr<PluginGfxDevice> PluginGfxDevice_p;
	typedef	WeakPtr<PluginGfxDevice>	PluginGfxDevice_wp;

	class PluginGfxDevice : public GfxDeviceGen1
	{
	public:

		//.____ Creation __________________________________________

		static PluginGfxDevice_p	create( wg_obj hDevice, PluginSurfaceFactory * pSurfaceFactory, PluginEdgemapFactory * pEdgemapFactory );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

        using GfxDevice::canvas;
        const CanvasInfo canvas(CanvasRef ref) const override;

		const TypeInfo&		surfaceType( void ) const override;

		//.____ Misc _______________________________________________________

		SurfaceFactory_p surfaceFactory() override;
        EdgemapFactory_p edgemapFactory() override;


		//.____ State _________________________________________________
        
        bool        setClipList(int nRectangles, const RectSPX * pRectangles) override;
        void        resetClipList() override;
        bool        pushClipList(int nRectangles, const RectSPX* pRectangles) override;
        bool        popClipList() override;

        const RectSPX* clipList() const override;
        int			clipListSize() const override;
        const RectSPX& clipBounds() const override;


        void        setTintColor( HiColor color ) override;
        HiColor		tintColor() const override;

        void        setTintGradient(const RectSPX& rect, const Gradient& gradient) override;
        void        clearTintGradient() override;

        bool        setBlendMode( BlendMode blendMode ) override;
        BlendMode 	blendMode() const override;

        bool        setBlitSource(Surface * pSource) override;
        Surface_p 	blitSource() const override;

        void        setMorphFactor(float factor) override;
        float		morphFactor() const override;

		void		setBlurbrush( Blurbrush * pBrush) override;
		
		void		setFixedBlendColor( HiColor color ) override;
		HiColor		fixedBlendColor() const override;
		
        void        setRenderLayer(int layer) override;
        int			renderLayer() const override;

        
        
		//.____ Rendering ________________________________________________

		bool	beginRender() override;
		bool	endRender() override;

        void    flush() override;
        
        void    endCanvasUpdate() override;

        
        // Draw methods.

        void    fill(HiColor col) override;
        void    fill( const RectSPX& rect, HiColor col ) override;

        void    plotPixels( int nCoords, const CoordSPX * pCoords, const HiColor * pColors) override;

        void    drawLine( CoordSPX begin, CoordSPX end, HiColor color, spx thickness = 64 ) override;
        void    drawLine( CoordSPX begin, Direction dir, int length, HiColor col, spx thickness = 64 ) override;

        // Blit methods

        void    blit(CoordSPX dest) override;
        void    blit(CoordSPX dest, const RectSPX& src) override;

        void    flipBlit(CoordSPX dest, GfxFlip flip ) override;
        void    flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip ) override;

        void    stretchBlit(const RectSPX& dest) override;
        void    stretchBlit(const RectSPX& dest, const RectSPX& src) override;
 
        void    stretchFlipBlit(const RectSPX& dest, GfxFlip flip) override;
        void    stretchFlipBlit(const RectSPX& dest, const RectSPX& src, GfxFlip flip) override;

		void	precisionBlit(const RectSPX& dest, const RectF& srcSPX) override;
		void	transformBlit(const RectSPX& dest, CoordF srcSPX, const float transform[2][2]) override;
        void    rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) override;

        void    tile(const RectSPX& dest, CoordSPX shift = { 0,0 }) override;
        void    flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift = { 0,0 }) override;

        void    scaleTile(const RectSPX& dest, float scale, CoordSPX shift = { 0,0 }) override;
        void    scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift = { 0,0 }) override;

		void	blur(CoordSPX dest) override;
		void	blur(CoordSPX dest, const RectSPX& src) override;
		void	stretchBlur(const RectSPX& dest) override;
		void	stretchBlur(const RectSPX& dest, const RectSPX& src) override;
		void	transformBlur(const RectSPX& dest, CoordF srcSPX, const float transform[2][2]) override;
		void	rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) override;

		
        // Draw segments methods

        void    drawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor frontFill, HiColor backFill) override;
        void    flipDrawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor frontFill, HiColor backFill, GfxFlip flip) override;

        void    drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness = 0, HiColor outlineColor = HiColor::Black) override;

        void    drawPieChart(const RectSPX& canvas, float start, int nSlices, const float * pSliceSizes, const HiColor * pSliceColors, float hubSize = 0.f, HiColor hubColor = HiColor::Transparent, HiColor backColor = HiColor::Transparent, bool bRectangular = false) override;

        void    drawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode = TintMode::Flat ) override;
        void    flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode = TintMode::Flat) override;

        void	drawEdgemap(CoordSPX dest, Edgemap* pEdgemap) override;
        void	flipDrawEdgemap(CoordSPX dest, Edgemap* pEdgemap, GfxFlip flip) override;

        // Special draw/blit methods

        void    blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale) override;

 
	protected:
		PluginGfxDevice( wg_obj object, PluginSurfaceFactory * pSurfaceFactory, PluginEdgemapFactory * pEdgemapFactory );
		~PluginGfxDevice();

        bool    _beginCanvasUpdate( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectI* pUpdateRects, CanvasLayers * pLayers, int startLayer) override;
        
        void    _canvasWasChanged() override;
        void    _renderLayerWasChanged() override;

        void    _transformBlitSimple(const RectI& dest, CoordI src, const int simpleTransform[2][2], OpType type) override;
        void    _transformBlitComplex(const RectI& dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type) override;

		void	_transformDrawSegments(const RectI& dest, int nSegments, const HiColor * pSegmentColors, int nEdges, const int * pEdges, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2]) override;

        wg_obj  m_cDevice;

		PluginSurfaceFactory_p	m_pSurfaceFactory;
        PluginEdgemapFactory_p	m_pEdgemapFactory;

        bool	m_bRendering;
	};
} // namespace wg
#endif //WG_PLUGINGFXDEVICE_DOT_H

