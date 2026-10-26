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
#ifndef	WAX_REFERENCEBACKEND_DOT_H
#define WAX_REFERENCEBACKEND_DOT_H
#pragma once

#include <wax_gfxbackend.h>
#include <wax_tinttools.h>
#include <wax_softsurface.h>
#include <wax_softedgemap.h>
#include <wax_softwarpmap.h>

#include <vector>

namespace wax
{
	class ReferenceBackend;
	typedef	StrongPtr<ReferenceBackend>	ReferenceBackend_p;
	typedef	WeakPtr<ReferenceBackend>	ReferenceBackend_wp;


	//____ ReferenceBackend _______________________________________________________
	/**
	 * @brief Software backend that renders everything by the book, as the standard for other backends.
	 *
	 * ReferenceBackend is written to be correct and easy to follow, not to be fast. Its output is
	 * what the output of all other backends is measured against, so it doesn't take any shortcuts:
	 *
	 * - Every pixel is calculated on its own, from the definition of the operation.
	 *
	 * - All calculations are done in double precision on linear colors with straight (not
	 *   premultiplied) alpha, where every channel is 0.0 -> 1.0. Colors are converted from
	 *   and to sRGB with the exact sRGB transfer functions, not lookup tables, and are only
	 *   rounded once, when the result is written to the canvas.
	 *
	 * - Coverage of anti-aliased edges is the exact area of the pixel that is covered. A pixel
	 *   partially covered by a shape is drawn as if the canvas had a much higher resolution,
	 *   the shape was drawn there without anti-aliasing and the subpixels were then averaged:
	 *   the area weighted average of the operation's result for the covered part and the
	 *   original pixel for the rest, averaged as premultiplied colors.
	 *
	 * - Blend modes follow the definitions in the guide ("Blending and transparency"). Paint
	 *   modes put paint on the canvas, with BlendMode::Blend being Porter-Duff 'over', also onto
	 *   transparent pixels. Drawing onto a transparent surface and blending that onto a
	 *   background gives the same result as drawing directly onto the background. Effect modes
	 *   change the colors already on the canvas, keep its alpha and use the alpha of the source
	 *   as the strength of the effect.
	 *
	 * - Bilinear sampling interpolates premultiplied colors, so transparent pixels don't bleed
	 *   their color into their neighbours.
	 *
	 * - Tints are evaluated at the center of every pixel, without lookup tables.
	 *
	 * - Shapes (rounded boxes and rings) are made of segments drawn together. A pixel gets the
	 *   average of what each segment gives the part of the pixel it covers, as for edgemaps,
	 *   with coverage worked out from the exact geometry of the shape.
	 *
	 * Geometry follows the GfxBackend contract that all backends implement, which in a few places
	 * is defined in whole pixels (end points of lines and position of edgemaps are rounded to
	 * nearest pixel). Those places are pointed out in the code.
	 *
	 * It uses the same surfaces and edgemaps as SoftBackend (SoftSurface and SoftEdgemap) and
	 * can render to canvases of PixelFormat ARGB_8, XRGB_8, RGB_565, BGR_565 and Alpha_8, in
	 * linear or sRGB color space and either byte order.
	 */

	class ReferenceBackend : public GfxBackend
	{
	public:

		//.____ Creation __________________________________________

		static ReferenceBackend_p	create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		void	beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo = nullptr ) override;
		void	endSession() override;

		void	setCanvas( Surface * pSurface ) override;
		void	setCanvas( CanvasRef ref ) override;

		void	setObjects(Object* const * pBeg, Object* const * pEnd) override;

		void	setRects(const RectSPX * pBeg, const RectSPX * pEnd) override;
		void	setColors(const HiColor * pBeg, const HiColor * pEnd) override;
		void	setTransforms(const Transform * pBeg, const Transform * pEnd) override;

		void	processCommands( const uint16_t* pBeg, const uint16_t * pEnd, int version = 2) override;

		//.____ Misc _________________________________________________________

		bool	defineCanvas(CanvasRef ref, SoftSurface* pSurface);

		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

		int		maxEdges() const override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

	protected:
		ReferenceBackend();
		virtual ~ReferenceBackend();

		// A color in linear color space with straight (not premultiplied) alpha. All channels 0.0 -> 1.0.

		struct Color
		{
			double r = 0, g = 0, b = 0, a = 0;

			Color operator*(const Color& k) const { return { r * k.r, g * k.g, b * k.b, a * k.a }; }
			Color operator*(double f) const { return { r * f, g * f, b * f, a * f }; }
			Color operator+(const Color& k) const { return { r + k.r, g + k.g, b + k.b, a + k.a }; }
		};

		// What a sample outside the source surface reads.

		enum class EdgeMode
		{
			Clamp,			// Nearest pixel on the edge of the surface.
			Clip,			// Transparent black.
			Tile			// The surface is repeated.
		};

		// An edge of an edgemap, through one column of pixels, from its position at the left side
		// of the column (y0) to its position at the right side (y1), in pixels.

		struct EdgeLine
		{
			double	y0, y1;

			double	at(double x) const { return y0 + (y1 - y0) * x; }		// x is 0.0 -> 1.0 over the column.
		};

		void	_resetStates();

		// Commands

		const uint16_t*	_changeStates(const uint16_t* p);

		void	_fill(const RectSPX& rect, const Color& color);
		void	_drawLine(CoordSPX begin, CoordSPX end, spx thickness, const Color& color, int nClipRects, const RectSPX* pClipRects);
		void	_drawCappedLine(CoordSPX begin, CoordSPX end, spx thickness, LineCaps caps, const Color& color, int nClipRects, const RectSPX* pClipRects);
		void	_blit(Command cmd, CoordI source, CoordSPX dest, const Transform& transform, bool bCustomTransform, const RectSPX& rect);
		void	_drawEdgemap(SoftEdgemap* pEdgemap, bool bTile, GfxFlip flip, CoordSPX origo, const RectSPX& rect);

		void	_drawEdgemapPixel(SoftEdgemap* pEdgemap, int column, int row, CoordI canvasPixel);
		void	_decodeSegmentTints(SoftEdgemap* pEdgemap);

		void	_warp(const WarpParams& params, SoftWarpmap* pWarpmap, SoftEdgemap* pEdgemap, const RectSPX& rect);
		int		_edgemapSegmentAt(SoftEdgemap* pEdgemap, double x, double y) const;

		// Pixels of the canvas

		bool	_isInsideCanvas(int x, int y) const;
		Color	_canvasPixel(int x, int y) const;
		void	_setCanvasPixel(int x, int y, const Color& color);
		void	_plotPixel(int x, int y, const Color& color, double coverage);

		bool	_isEffectMode() const;
		Color	_effectSource(int nColors, const Color* pColors, const double* pCoverage) const;

		Color	_blend(const Color& src, const Color& dst) const;
		Color	_blendEffect(const Color& src, const Color& dst) const;
		double	_blendAlphaOnly(double src, double dst) const;

		static Color	_over(const Color& src, const Color& dst);
		static Color	_averageColors(int nColors, const Color* pColors, const double* pWeights, bool bPremultiplied);

		// Pixels of the blit source

		Color	_sampleSource(double x, double y, EdgeMode edgeMode) const;
		Color	_sampleBlurred(double x, double y) const;
		Color	_sourcePixel(int x, int y, EdgeMode edgeMode) const;

		// Tints

		Color			_deviceTint(int x, int y) const;
		Color			_segmentColor(SoftEdgemap* pEdgemap, int segment, int column, int row) const;
		static Color	_tintColorAt(const TintTools::DecodedTint& tint, double x, double y);

		// Conversions

		static Color	_toLinear(HiColor color);
		static double	_sRGBToLinear(double value);
		static double	_linearToSRGB(double value);

		static Color	_readPixel(const PixelBuffer& buffer, int x, int y);
		static void		_writePixel(const PixelBuffer& buffer, int x, int y, const Color& color);

		static RectI	_pixelRect(const RectSPX& rect);

		// Edgemap coverage

		static void		_segmentCoverage(const EdgeLine* pEdges, int nEdges, int row, double* pCoverage);
		static double	_areaBelow(const EdgeLine* pEdges, int nEdges, int row);

		// Shapes (Command::DrawShape), see "Shapes" in wax_referencebackend.cpp.

		struct ShapeEdge;
		class Shape;
		class RoundedBoxShape;
		class RingShape;
		class LineShape;

		void			_drawShape(ShapeKind kind, const uint16_t* pPayload, int nRects);
		void			_drawShapePixel(const Shape& shape, int x, int y);
		static void		_shapeCoverage(const Shape& shape, int x, int y, std::vector<double>& coverage);

		//

		std::vector<CanvasInfo>	m_definedCanvases;

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;
		WarpmapFactory_p	m_pWarpmapFactory;

		Object * const *	m_pObjectsBeg = nullptr;		// Object table, commands refer to objects by index.
		const RectSPX *		m_pRectsPtr = nullptr;
		const HiColor *		m_pColorsPtr = nullptr;
		const Transform *	m_pTransformsBeg = nullptr;

		// Canvas

		Surface_p			m_pCanvas;
		PixelBuffer			m_canvasBuffer;
		bool				m_bAlphaOnlyCanvas = false;		// Canvas is Alpha_8, which is blended differently.
		bool				m_bCanvasHasAlpha = false;

		// States

		Surface_p			m_pBlitSource;
		PixelBuffer			m_blitSourceBuffer;

		BlendMode			m_blendMode = BlendMode::Blend;
		Color				m_tintColor;					// Flat tint color, linear.
		TintTools::DecodedTint	m_tint;						// Tint over the canvas, nLayers is 0 if none.
		double				m_morphFactor = 0.5;

		double				m_blurRadius = 0;				// In pixels.
		double				m_blurWeights[3][9];			// Red, green and blue weights for the 3x3 samples.

		std::vector<TintTools::DecodedTint>	m_segmentTints;	// Tints of the segments of the edgemap being drawn.
	};

} // namespace wax
#endif	// WAX_REFERENCEBACKEND_DOT_H
