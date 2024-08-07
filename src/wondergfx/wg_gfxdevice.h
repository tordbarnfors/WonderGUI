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

#ifndef	WG_GFXDEVICE_DOT_H
#define WG_GFXDEVICE_DOT_H
#pragma once

#include <climits>

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

	class GfxDevice;
	typedef	StrongPtr<GfxDevice>	GfxDevice_p;
	typedef	WeakPtr<GfxDevice>		GfxDevice_wp;


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

		virtual const TypeInfo&	surfaceType( void ) const = 0;

		//.____ Misc _______________________________________________________

		virtual const CanvasInfo&	canvas() const = 0;
		virtual const CanvasInfo	canvas(CanvasRef ref) const = 0;

		virtual CanvasLayers_p 		canvasLayers() const = 0;

		virtual SurfaceFactory_p	surfaceFactory() = 0;
		virtual EdgemapFactory_p	edgemapFactory() = 0;

		static constexpr int		maxSegments() { return c_maxSegments; }
		
		//.____ Geometry _________________________________________________

		virtual SizeSPX		canvasSize() const = 0;


		//.____ State _________________________________________________

		virtual bool		setClipList(int nRectangles, const RectSPX * pRectangles) = 0;
		virtual void		resetClipList() = 0;
		virtual bool		pushClipList(int nRectangles, const RectSPX* pRectangles) = 0;
		virtual bool		popClipList() = 0;

		virtual const RectSPX* clipList() const = 0;
		virtual int			clipListSize() const = 0;
		virtual const RectSPX& clipBounds() const = 0;

		virtual void		setTintColor( HiColor color ) = 0;
		virtual HiColor		tintColor() const = 0;

		virtual void		setTintGradient(const RectSPX& rect, const Gradient& gradient) = 0;
		virtual void		clearTintGradient() = 0;

		virtual void		setTintmap(const RectSPX& rect, Tintmap* pTintmap) = 0;
		virtual void		clearTintmap() = 0;

		virtual bool		setBlendMode( BlendMode blendMode ) = 0;
		virtual BlendMode 	blendMode() const = 0;

		virtual bool		setBlitSource(Surface * pSource) = 0;
		virtual Surface_p 	blitSource() const = 0;

		virtual void		setMorphFactor(float factor) = 0;
		virtual float		morphFactor() const = 0;

		virtual void		setBlurMatrices( spx radius, const float red[9], const float green[9], const float blue[9] ) = 0;
		
		virtual void		setFixedBlendColor( HiColor color ) = 0;
		virtual HiColor		fixedBlendColor() const = 0;

		virtual void		setRenderLayer(int layer) = 0;
		virtual int			renderLayer() const = 0;

		
		
		
		//.____ Rendering ________________________________________________

		virtual bool	beginRender() = 0;
		virtual bool	endRender() = 0;
		virtual bool    isRendering() = 0;
		virtual bool	isIdle() = 0;
		virtual void	flush() = 0;

        virtual bool     beginCanvasUpdate( CanvasRef canvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers * pLayers = nullptr, int startLayer = -1 ) = 0;
        virtual bool     beginCanvasUpdate( Surface * pCanvas, int nUpdateRects = 0, const RectSPX* pUpdateRects = nullptr, CanvasLayers * pLayers = nullptr, int startLayer = -1 ) = 0;
        virtual void    endCanvasUpdate() = 0;

        
		// Draw methods.

		virtual void	fill(HiColor color) = 0;
		virtual void	fill( const RectSPX& rect, HiColor color ) = 0;

		virtual void    plotPixels( int nCoords, const CoordSPX * pCoords, const HiColor * pColors) = 0;

	 	virtual void	drawLine( CoordSPX begin, CoordSPX end, HiColor color, spx thickness = 64 ) = 0;
		virtual void	drawLine( CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness = 64 ) = 0;

		// Blit methods

		virtual void	blit(CoordSPX dest) = 0;
		virtual void	blit(CoordSPX dest, const RectSPX& src) = 0;

		virtual void	flipBlit(CoordSPX dest, GfxFlip flip ) = 0;
		virtual void	flipBlit(CoordSPX dest, const RectSPX& src, GfxFlip flip ) = 0;

		virtual void	stretchBlit(const RectSPX& dest) = 0;
		virtual void	stretchBlit(const RectSPX& dest, const RectSPX& src) = 0;

		virtual void	stretchFlipBlit(const RectSPX& dest, GfxFlip flip) = 0;
		virtual void	stretchFlipBlit(const RectSPX& dest, const RectSPX& src, GfxFlip flip) = 0;

		virtual void	precisionBlit(const RectSPX& dest, const RectF& srcSPX) = 0;
		virtual void	transformBlit(const RectSPX& dest, CoordF srcSPX, const float transform[2][2]) = 0;
		virtual void	rotScaleBlit(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) = 0;

		virtual void	tile(const RectSPX& dest, CoordSPX shift = { 0,0 }) = 0;
		virtual void	flipTile(const RectSPX& dest, GfxFlip flip, CoordSPX shift = { 0,0 }) = 0;

		virtual void	scaleTile(const RectSPX& dest, float scale, CoordSPX shift = { 0,0 }) = 0;
		virtual void	scaleFlipTile(const RectSPX& dest, float scale, GfxFlip flip, CoordSPX shift = { 0,0 }) = 0;

		virtual void	blur(CoordSPX dest) = 0;
		virtual void	blur(CoordSPX dest, const RectSPX& src) = 0;
		virtual void	stretchBlur(const RectSPX& dest) = 0;
		virtual void	stretchBlur(const RectSPX& dest, const RectSPX& src) = 0;
		virtual void	transformBlur(const RectSPX& dest, CoordF srcSPX, const float transform[2][2]) = 0;
		virtual void	rotScaleBlur(const RectSPX& dest, float rotationDegrees, float scale, CoordF srcCenter = { 0.5f, 0.5f }, CoordF destCenter = { 0.5f,0.5f }) = 0;


		// Draw segments methods

		virtual void	drawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor frontFill, HiColor backFill) = 0;
		virtual void	flipDrawWave(const RectSPX& dest, const WaveLine * pTopBorder, const WaveLine * pBottomBorder, HiColor frontFill, HiColor backFill, GfxFlip flip) = 0;

		virtual void	drawElipse(const RectSPX& canvas, spx thickness, HiColor color, spx outlineThickness = 0, HiColor outlineColor = HiColor::Black) = 0;

		virtual void	drawPieChart(const RectSPX& canvas, float start, int nSlices, const float * pSliceSizes, const HiColor * pSliceColors, float hubSize = 0.f, HiColor hubColor = HiColor::Transparent, HiColor backColor = HiColor::Transparent, bool bRectangular = false) = 0;

		virtual void	drawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode = TintMode::Flat ) = 0;
		virtual void	flipDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, GfxFlip flip, TintMode tintMode = TintMode::Flat) = 0;

		virtual void	drawEdgemap(CoordSPX dest, Edgemap * pEdgemap ) = 0;
		virtual void	flipDrawEdgemap(CoordSPX dest, Edgemap * pEdgemap, GfxFlip flip) = 0;

				

		// Special draw/blit methods

		virtual void	blitNinePatch(const RectSPX& dstRect, const BorderSPX& dstFrame, const NinePatch& patch, int scale) = 0;



	protected:
		GfxDevice();
		virtual ~GfxDevice();

		const static int	c_maxSegments = 16;
	};



} // namespace wg
#endif	// WG_GFXDEVICE_DOT_H

