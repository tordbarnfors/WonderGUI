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
#ifndef	WG_SOFTBACKEND_DOT_H
#define WG_SOFTBACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>
#include <wg_tinttools.h>
#include <wg_softtint.h>
#include <wg_softsurface.h>
#include <wg_softformat.h>
#include <wg_softshapes.h>

namespace wg
{
#if defined(__ARM_ARCH) && defined(__APPLE__)
	// Apple Silicon (M1/M2/M3)
	static constexpr size_t kCacheLineSize = 128;
#elif defined(__ARM_ARCH)
	// Cortex-M7 and most other ARM (Cortex-A typically 64)
	static constexpr size_t kCacheLineSize = 32;
#else
	// x86/x64
	static constexpr size_t kCacheLineSize = 64;
#endif

	class SoftBackend;
	class SoftEdgemap;
	class SoftWarpmap;
	typedef	StrongPtr<SoftBackend>	SoftBackend_p;
	typedef	WeakPtr<SoftBackend>	SoftBackend_wp;


	//____ SoftBackend __________________________________________________________

	class SoftBackend : public GfxBackend
	{
		friend class LinearBackend;
	public:

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Finalizer_p	finalizer = nullptr;

			// Buffer for rows of shapes symmetric upside down, kept to draw their lower half
			// mirrored, see SoftShapes::setMirrorBuffer(). Allocated to mirrorBuffer bytes on
			// creation and later as needed up to maxMirrorBuffer. Make them the same to
			// allocate it once.

			int			mirrorBuffer = 0;
			int			maxMirrorBuffer = SoftShapes::c_defaultMaxMirrorBuffer;
		};

		//.____ Creation __________________________________________

		static SoftBackend_p	create();
		static SoftBackend_p	create( const Blueprint& blueprint );

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

		void	processCommands( const uint16_t* pBeg, const uint16_t * pEnd, int version) override;


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

		//.____ Internal _____________________________________________________

		struct Pitches
		{
			int srcX;			// Pitch in bytes for each pixel read.
			int srcY;			// Pitch in bytes from end of line to beginning of next for each line read.
			int dstX;			// Pitch in bytes for each pixel written.
			int dstY;			// Pitch in bytes from end of line to beginning of next for each line written.
		};

		enum ReadOp
		{
			Normal,
			Tile,
			Clip,
			Blur
		};

		const static int             ReadOp_size = 4;

		struct ColTrans
		{
			TintMode	mode;

			HiColor		flatTintColor;	// Used in TintMode::Color

			bool		bTintOpaque;	// Set if the tint is opaque.

			// Following used in TintMode GradientX, GradientY and GradientXY.

			RectI		tintRect;

			const HiColor*	pTintAxisX;
			const HiColor*	pTintAxisY;

			// Other values we also store here

			int			morphFactor;	// Scale: 0 -> 4096

			HiColor		fixedBlendColor;

			int			blurMtxR[9];
			int			blurMtxG[9];
			int			blurMtxB[9];

			CoordSPX	blurOfsSPX[9];
			CoordI		blurOfsPixel[9];
		};

		struct SegmentEdge
		{
			int			begin;				// Pixeloffset, 24.8 format.
			int			end;				// Pixeloffset, 24.8 format. First pixel after edge (segment after the edge has 100% coverage)
			int			coverage;			// 0-65536
			int			coverageInc;		// Increment of coverage for each full pixel we progress
		};
		
		enum class StripSource
		{
			Colors,
			Tintmaps,
			ColorsAndTintmaps,
		};

		// A pixel only partly covered by a shape, for coverage kernels, see SoftShapes::CoveragePixel.
		// Colors are weighted by coverage and alpha, as premultiplied colors:
		//
		//   b,g,r = sum( coverage * alpha * color ), a = sum( coverage * alpha )

		using CoveragePixel = SoftShapes::CoveragePixel;

		typedef	void(*LineOp_p)(uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*ClipLineOp_p)(int clipStart, int clipEnd, uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*FillOp_p)(uint8_t* pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const ColTrans& tint, CoordI patchPos);
		typedef	void(*StraightBlitOp_p)(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const Pitches& pitches, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos, const Transform * pMatrix);
		typedef	void(*TransformBlitOp_p)(const SoftSurface* pSrcSurf, BinalCoord pos, const binalInt matrix[2][2], uint8_t* pDst, int dstPitchX, int dstPitchY, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos);
		typedef void(*SegmentOp_p)(int clipBeg, int clipEnd, uint8_t* pStripStart, int pixelPitch, int nEdges, SegmentEdge* pEdges, const int16_t* pSegmentColors, const HiColor * pSegmentTintmap, int segmentTintmapPitch, const bool* pTransparentSegments, const bool* pOpaqueSegments, const ColTrans& tint);
		typedef void(*CoverageOp_p)(uint8_t* pDst, int pixelPitch, int length, const CoveragePixel* pPixels, const ColTrans& tint);

		alignas(kCacheLineSize) static const int16_t s_channel_5_linear[32];
		alignas(kCacheLineSize) static const int16_t s_channel_6_linear[64];
		alignas(kCacheLineSize) static const int16_t s_channel_5_sRGB[32];
		alignas(kCacheLineSize) static const int16_t s_channel_6_sRGB[64];
		alignas(kCacheLineSize) static const uint8_t s_fast8_channel_5[32];
		alignas(kCacheLineSize) static const uint8_t s_fast8_channel_6[64];

		// 8-bit channel values rounded to 5 and 6 bits, left in the high bits so that
		// the bits to pack can be shifted and masked out as from the 8-bit value.

		alignas(kCacheLineSize) static uint8_t s_round_channel_5[256];
		alignas(kCacheLineSize) static uint8_t s_round_channel_6[256];

		alignas(kCacheLineSize) static int s_mulTab[256];
		alignas(kCacheLineSize) static int16_t s_limit4096Tab[4097 * 3];
		alignas(kCacheLineSize) static uint32_t s_reciprocalTab[4097];	// (2^32 - 1) / i for 12-bit values, 0 for 0.

		const static int	c_maxSegments = 16;


		//.____ Control ______________________________________________________

		// Kernels are set for SoftFormats, which combine pixel format, color space and byte order.

		bool	setFillKernel( TintMode tintMode, BlendMode blendMode, SoftFormat destFormat, FillOp_p pKernel);

		bool	setStraightBlitKernel(	SoftFormat sourceFormat, ReadOp readOp, TintMode tintMode,
										BlendMode blendMode, SoftFormat destFormat, StraightBlitOp_p pKernel);

		bool	setLineKernel(BlendMode blendMode, SoftFormat destFormat, LineOp_p pKernel);
		bool	setClipLineKernel(BlendMode blendMode, SoftFormat destFormat, ClipLineOp_p pKernel);

		bool	setSegmentStripKernel(SoftBackend::StripSource, BlendMode blendMode, SoftFormat destFormat, SegmentOp_p pKernel);

		bool	setCoverageKernel(BlendMode blendMode, SoftFormat destFormat, CoverageOp_p pKernel);

		bool	setTransformBlitKernel(SoftFormat sourceFormat, SampleMethod sampleMethod, ReadOp readOp,
			TintMode tintMode, BlendMode blendMode, SoftFormat destFormat, TransformBlitOp_p pKernel);



	protected:
		SoftBackend();
		SoftBackend( const Blueprint& blueprint );
		virtual ~SoftBackend();

		void _initTables();
		bool _setupDestFormatKernels(SoftFormat format);

		void	_resetStates();
		void	_updateBlitFunctions();
		void	_selectBlitFunctions(TintMode tintMode);

		// Tint handling, shared with LinearBackend.

		enum class TintLayout
		{
			None,			// No tint or flat tint, use m_colTrans.flatTintColor (TintMode None or Flat).
			Vertical,		// Tint only varies vertically. Drawn row by row with TintMode::Flat.
			Horizontal,		// Tint only varies horizontally. One row of tint colors, TintMode::GradientX.
			General			// Tint varies in both directions. Drawn row by row with TintMode::GradientX.
		};

		void	_setTintColor(HiColor color);
		const uint16_t* _setTint(const uint16_t* p, const HiColor*& pColors);
		void	_updateTint();
		inline void	_generateTintRow(int x, int y, int length, HiColor* pOutput) { m_softTint.generate(x, y, 1, 0, length, pOutput); }
		HiColor* _tintRowBuffer(int length);

		// Edgemap tinting, shared with LinearBackend.

		struct EdgemapTinting
		{
			bool		bPerPixel;					// Colors per pixel (StripSource::Tintmaps), otherwise flat colors per segment (StripSource::Colors).
			HiColor*	pColumns;					// Per segment: one column of colors, indexed by edgemap row.
			HiColor*	pGlobal;					// One column of device tint colors.
			int			pitch;						// Colors between segments in pColumns.
			int			bufferBytes;				// Allocated from memStack.
			bool		bColumnInvariant;			// Colors are the same for every column, pColumns only needs to be generated once.
			bool		bColumnsDone;				// pColumns generated for all rows, if bColumnInvariant.
			bool		bColorPerColumn;			// Colors are the same along each column: flat colors (not bPerPixel), updated per column.
			int16_t		colors[c_maxSegments][4];	// Flat colors, BGRA, tint included.
			bool		transparent[c_maxSegments];
			bool		opaque[c_maxSegments];
		};

		void	_beginEdgemapTinting(SoftEdgemap* pEdgemap, int nSegments, EdgemapTinting& tinting, const int simpleTransform[2][2]);
		void	_tintEdgemapColumn(SoftEdgemap* pEdgemap, int nSegments, EdgemapTinting& tinting, int column, int rowBeg, int rowEnd,
								   const int* pEdgeStrips, CoordI canvasStart, const int simpleTransform[2][2]);
		void	_tintEdgemapRows(SoftEdgemap* pEdgemap, int nSegments, EdgemapTinting& tinting, int column, int rowBeg, int rowEnd,
								 const int* pEdgeStrips, CoordI canvasStart, const int simpleTransform[2][2]);
		void	_endEdgemapTinting(EdgemapTinting& tinting);
		void	_flattenEdgemapTinting(SoftEdgemap* pEdgemap, int nSegments, EdgemapTinting& tinting, CoordI canvasCenter);

		void	_drawEdgemapPatch(SoftEdgemap* pEdgemap, bool bTile, const RectI& patch, const RectI& dest, CoordI start,
								  const int simpleTransform[2][2], uint8_t* pBase, CoordI baseCoord, int xPitch, int yPitch,
								  int nSegments, EdgemapTinting& tinting, SegmentOp_p pOp);

		/**
		 * Calls draw(const RectI& subRect) for subrectangles of rect as needed for the current tint,
		 * with m_colTrans prepared for each call. bRuns: use Flat kernels, split into runs of same color
		 * (fallback for kernel sets without GradientX kernels).
		 */
		template<class DrawFunc> void _forTintSpans(const RectI& rect, bool bRuns, DrawFunc&& draw);
		void	_updateBlurRadius(spx radius);

		// Shapes (Command::DrawShape), shared with LinearBackend. _beginShape() decodes a shape
		// and prepares its segments, _drawShape() draws it within one clip rectangle onto
		// pixels where pixel (x,y) is at pOrigin + y * pitch + x * m_canvasPixelBytes.

		struct ShapeSegment
		{
			bool		bActive;				// Segment is drawn.
			bool		bFlat;					// Same color everywhere, including device tint.
			SoftTint	tint;					// Color and tint of segment, without device tint.

			// Prepared for drawing.

			FillOp_p	pFillOp;				// Fully covered runs, if tint is flat. Device tint applied by kernel.
			bool		bFillRuns;				// pFillOp draws runs of device tint colors.
			HiColor		fillColor;				// Color for pFillOp.
			int16_t		mix[4];					// Color with tints as b, g, r, a for coverage pixels, colors weighted by alpha. If bFlat.
		};

		class ShapeSink : public SoftShapes::RowSink
		{
		public:
			const int16_t*	segmentColors(int segment, int y, int begin, int end, bool& bFlat) override;
			void	row(int y, int nRuns, const SoftShapes::Run* pRuns, const CoveragePixel* pPixels) override;
			void	rows(int yBeg, int yEnd, int nRuns, const SoftShapes::Run* pRuns, const CoveragePixel* pPixels) override;

			SoftBackend*	pBackend;
			uint8_t*		pOrigin;
			int				pitch;
			int				rowEnd;				// Row below the clip rectangle.
		};

		bool	_beginShape(GfxBackend::ShapeKind kind, const uint16_t* pPayload, const HiColor* pColors);
		void	_setupShapeSegment(ShapeSegment& seg, bool bActive, HiColor color, bool bTint, const uint16_t*& pTintWords, const HiColor*& pTintColors);
		void	_drawShape(const RectI& clip, uint8_t* pOrigin, int pitch);
		void	_drawShapeRuns(uint8_t* pOrigin, int pitch, int yBeg, int yEnd, int nRuns, const SoftShapes::Run* pRuns, const CoveragePixel* pPixels, int prefetchRow);
		void	_shapeSegmentColors(int segment, int x, int y, int length, HiColor* pOutput);
		HiColor* _shapeBuffer(int index, int length);

		bool	_fadeByCoverage(BlendMode mode, bool bOpaqueSource) const;
		static CoveragePixel _coveragePixel(const HiColor& color, int coverage);
		void	_drawCoverageEdge(CoverageOp_p pOp, const RectI& rect, int coverage, HiColor color, uint8_t* pOrigin, int pitch);
		void	_drawTintedLine(CoverageOp_p pOp, bool bHorizontal, int majorBeg, int length, int width, int pos, int slope,
								int clipStart, int clipEnd, HiColor color, uint8_t* pOrigin, int pitch);

		int		_scaleLineThickness(float thickness, int slope);

		// Lines with caps (Command::Line), shared with LinearBackend. _beginCappedLine() prepares a
		// line, _drawCappedLine() draws it within one clip rectangle onto pixels where pixel (x,y)
		// is at pOrigin + y * pitch + x * m_canvasPixelBytes. Whole columns between the ends are
		// drawn by the line kernels, the parts around the ends as shapes, see SoftShapes::prepareLine().

		bool	_beginCappedLine(CoordSPX begin, CoordSPX end, spx thickness, LineCaps caps, HiColor color);
		void	_drawCappedLine(const RectI& clip, uint8_t* pOrigin, int pitch);

		// Warps (Command::WarpBlit and WarpDrawEdgemap), shared with LinearBackend. _beginWarp()
		// prepares a command, _drawWarp() draws it within one clip rectangle onto pixels where
		// pixel (x,y) is at pOrigin + y * pitch + x * m_canvasPixelBytes. See wg_softbackend_warp.cpp.

		static const int c_warpOutside = -2;	// WarpCell::kind: Nothing of the source in the cell.
		static const int c_warpMixed = -1;		// WarpCell::kind: Coverage or segment varies over the cell.

		struct WarpCell
		{
			int			x;				// First pixel column of the cell, relative to the Warpmap.
			int			w;				// Width of cell in pixels.
			int			kind;			// c_warpOutside, c_warpMixed or segment (0 for WarpBlit) fully covering the cell.

			// Positions in the source in pixels with 16 binals, source offset applied. Top-left
			// corner is wrapped into the period on periodic axes, the others follow it.

			int64_t		tl[2];			// Top-left corner.
			int64_t		right[2];		// Top-right corner - top-left corner.
			int64_t		down[2];		// Bottom-left corner - top-left corner.
			int64_t		twist[2];		// Bottom-right - bottom-left - top-right + top-left.

			// At the first pixel drawn of the current row of pixels, stepped from row to row so that
			// rows need no divisions, see _beginWarpCellRow(). pos and step have c_warpExtraBits more
			// binals than positions.

			int64_t		pos[2];			// Position at the center of the first pixel.
			int64_t		posStep[2];		// Change of pos from one row to the next.
			int64_t		step[2];		// Change of position from one pixel to the next along the row.
			int64_t		stepStep[2];	// Change of step from one row to the next.
			int64_t		dy[2];			// Change of position from one pixel to the one below, at the first pixel.
			int64_t		dyStep[2];		// Change of dy along the row.
		};

		struct WarpSetup
		{
			SoftWarpmap*	pWarpmap;
			SoftEdgemap*	pEdgemap;			// nullptr for WarpBlit.
			CoordI			dest;				// Top-left pixel of the Warpmap.

			// Per axis (x, y), in pixels of the source with 16 binals.

			int64_t			size[2];			// Size of source.
			int64_t			period[2];			// 0 if not periodic.
			int64_t			inside[2];			// Part of each period inside the source.
			int64_t			colorLow[2];		// Colors are read at positions wrapped into colorLow -> colorLow + period,
												// which puts a center in the gap between copies in the nearest one.
			bool			bAlwaysInside[2];	// Periodic with period <= size, always inside.
			int64_t			offset[2];			// Source offset, wrapped into period.

			// WarpDrawEdgemap

			int				nSegments;
			bool			bTinted[c_maxSegments];		// Colors from segment tint, otherwise mix.
			bool			bSkip[c_maxSegments];		// Transparent segment that can be skipped.
			int16_t			mix[c_maxSegments][4];		// Flat color as b, g, r, a weighted by alpha. Device tint included if flat.
			HiColor			flat[c_maxSegments];		// Flat color of segment, without device tint.

			bool			bOpaquePaint;		// Replace or Morph onto canvas without alpha, see _beginWarp().
			int				opaqueMorph;		// Morph factor for bOpaquePaint, 4096 for Replace.

			// WarpBlit

			int64_t			sourcePos[2];		// Source rectangle position in surface.
			int64_t			readMin[2];			// Range of positions read from a non-tiling surface, which stays within the
			int64_t			readMax[2];			// source rectangle, in kernel positions (relative to pixel centers if bilinear).
			bool			bBilinear;
			bool			bTileRead;			// Surface is tiling, read with ReadOp::Tile.
			TransformBlitOp_p	pReadOp;		// Reads into a HiColor buffer.
		};

		bool	_beginWarp(const WarpParams& params, SoftWarpmap* pWarpmap, SoftEdgemap* pEdgemap);
		void	_drawWarp(const RectI& clip, uint8_t* pOrigin, int pitch);
		void	_setupWarpCells(int row, int firstColumn, int nColumns);
		void	_beginWarpCellRow(int firstRow, int xBeg, int cellRowY, int cellRowH);
		int		_warpSolidSegment(const int64_t lo[2], const int64_t hi[2], const int64_t copy[2]) const;
		void	_drawWarpRow(int y, int xBeg, int xEnd, uint8_t* pOrigin, int pitch, int prefetchRow);
		int		_warpAxisCoverage(int axis, int64_t s, int64_t gx, int64_t gy) const;
		int		_warpEdgemapCoverage(int64_t sx, int64_t sy, int64_t ax, int64_t bx, int64_t ay, int64_t by, int* pCoverage, int& solidSegment) const;
		void	_warpTintSpan(const SoftTint& tint, int64_t x, int64_t y, int64_t dx, int64_t dy, int length, HiColor* pOutput);
		void	_warpReadSpan(int64_t x, int64_t y, int64_t dx, int64_t dy, int length, HiColor* pOutput, const int* pCoverage);
		int		_warpOpaqueAlpha(int a) const;
		HiColor* _warpBuffer(int index, int length);




		std::vector<CanvasInfo>	m_definedCanvases;

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;
		WarpmapFactory_p	m_pWarpmapFactory;

		Object * const *	m_pObjectsBeg = nullptr;		// Object table, commands refer to objects by index.
		Object * const *	m_pObjectsEnd = nullptr;

		const RectSPX *		m_pRectsBeg = nullptr;
		const RectSPX *		m_pRectsEnd = nullptr;
		const RectSPX *		m_pRectsPtr = nullptr;

		const HiColor*		m_pColorsBeg = nullptr;
		const HiColor*		m_pColorsEnd = nullptr;
		const HiColor*		m_pColorsPtr = nullptr;

		const Transform *	m_pTransformsBeg = nullptr;
		const Transform *	m_pTransformsEnd = nullptr;

		// Canvas related

		SoftSurface_p		m_pCanvas;
		PixelBuffer			m_buffer;
		uint8_t*			m_pCanvasPixels;	// Pixels of render layer surface
		int					m_canvasPixelBytes = 0;			// Bytes per pixel of render layer surface
		SoftFormat			m_canvasSoftFormat = SoftFormat::Undefined;
		int					m_canvasPitch = 0;

		// Current state

		BlendMode			m_blendMode = BlendMode::Blend;
//		TintMode			m_tintMode = TintMode::None;

		ColTrans			m_colTrans;

		SoftSurface_p		m_pBlitSource;

		bool				m_bBlitFunctionNeedsUpdate = true;

		// Tint

		HiColor				m_tintColor = HiColor::White;		// Flat tint color (StateChange::TintColor).
		TintTools::DecodedTint	m_tint;							// Current Tint (StateChange::Tint), nLayers == 0 if none.
		SoftTint			m_softTint;							// Tint and tint color combined, prepared for rendering.
		TintLayout			m_tintLayout = TintLayout::None;
		std::vector<HiColor>	m_tintRow;
		bool				m_bBlitTintRuns = false;			// Blits need to use runs with Flat kernels, GradientX kernels missing.

		// Shapes

		SoftShapes					m_shapes;
		SoftShapes::RoundedBox		m_roundedBox;
		SoftShapes::Ring			m_ring;
		std::vector<uint32_t>		m_ringSliceSizes;
		GfxBackend::ShapeKind		m_shapeKind;
		std::vector<ShapeSegment>	m_shapeSegments;
		std::vector<HiColor>		m_shapeBuffers[2];				// Colors of a segment (0) and the device tint (1) along a row.
		std::vector<std::vector<int16_t>>	m_shapeMixBuffers;		// Colors of segments that aren't flat, for coverage pixels.
		std::vector<CoveragePixel>	m_coverageColumn;
		std::vector<CoveragePixel>	m_coverageEdge;					// Edge of a fill or span of a tinted line, see _drawCoverageEdge().
		CoverageOp_p				m_pShapeCoverageOp = nullptr;

		// Lines with caps

		SoftShapes::LineParts		m_lineParts;
		ClipLineOp_p				m_pLineOp = nullptr;			// Body of line, unless tinted...
		CoverageOp_p				m_pTintedLineOp = nullptr;		// ...by a Tint.
		HiColor						m_lineColor;					// Linear, with tint color for m_pLineOp.
		HiColor						m_tintedLineColor;				// Linear, for m_pTintedLineOp.

		// Warps

		WarpSetup					m_warp;
		std::vector<WarpCell>		m_warpCells;
		std::vector<SoftShapes::Run>	m_warpRuns;
		std::vector<CoveragePixel>	m_warpPixels;
		std::vector<int>			m_warpCoverage;
		std::vector<HiColor>		m_warpBuffers[c_maxSegments + 2];	// Colors of segments, then source or device tint.


//		PixelBuffer		m_canvasPixelBuffer;
//		uint8_t* m_pCanvasPixels;	// Pixels of render layer surface

		static int				s_lineThicknessTable[17];


		static bool				s_bTablesInitialized;




		//

		typedef void(SoftBackend::* StraightBlitProxy_Op)(const RectI& dest, CoordI src, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);
		typedef void(SoftBackend::* TransformBlitProxy_Op)(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp, Command cmd);


		void	_onePassStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);
		void	_twoPassStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);

		void	_onePassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp, Command cmd);
		void	_twoPassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp, Command cmd);

		void	_dummyStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);
		void	_dummyTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp, Command cmd);

		//

		struct SinglePassStraightBlitKernels // 80 bytes on 32-bit system
		{
			StraightBlitOp_p	pKernels[ReadOp_size][TintMode_size];
		};

		struct SinglePassTransformBlitKernels // 240 bytes on 32-bit system
		{
			TransformBlitOp_p	pKernels[SampleMethod_size][ReadOp_size][TintMode_size];		// Optimization opportunity: Only 2 sample methods used
		};

		struct SinglePassBlitKernels	// 44 bytes on all systems
		{
			SinglePassBlitKernels();

			uint16_t	straightBlitKernels[BlendMode_size];	// Offset into m_singlePassStraightBlitKernels
			uint16_t	transformBlitKernels[BlendMode_size];	// Offset into m_singlePassTransformBlitKernels
		};

		struct DestFormatKernels	// 752 bytes on 32-bit system
		{
			DestFormatKernels();

			LineOp_p 				pLineKernels[BlendMode_size];
			ClipLineOp_p			pClipLineKernels[BlendMode_size];
			FillOp_p				pFillKernels[TintMode_size][BlendMode_size];
			SegmentOp_p				pSegmentKernels[3][BlendMode_size];
			CoverageOp_p			pCoverageKernels[BlendMode_size];

			StraightBlitOp_p		pStraightBlitFromHiColorKernels[TintMode_size][BlendMode_size];
			StraightBlitOp_p		pStraightBlitFromBGRA8Kernels[TintMode_size][BlendMode_size];

			uint16_t				singlePassBlitKernels[SoftFormat_size];		// SoftFormat of blit source. Offset into m_singlePassBlitKernels +1.
		};

		StraightBlitOp_p	m_pStraightMoveToBGRA8Kernels[SoftFormat_size][ReadOp_size];
		StraightBlitOp_p	m_pStraightMoveToHiColorKernels[SoftFormat_size][ReadOp_size];

		TransformBlitOp_p	m_pTransformMoveToBGRA8Kernels[SoftFormat_size][SampleMethod_size][ReadOp_size];
		TransformBlitOp_p	m_pTransformMoveToHiColorKernels[SoftFormat_size][SampleMethod_size][ReadOp_size];

		DestFormatKernels* m_pKernels[SoftFormat_size];

		std::vector<SinglePassBlitKernels>			m_singlePassBlitKernels;
		std::vector<SinglePassStraightBlitKernels>	m_singlePassStraightBlitKernels;
		std::vector<SinglePassTransformBlitKernels>	m_singlePassTransformBlitKernels;

		// These are called to perform blit/tile operations

		StraightBlitProxy_Op	m_pStraightBlitOp = nullptr;		// Function called to perform a straight blit.
		StraightBlitProxy_Op	m_pStraightTileOp = nullptr;		// Function called to perform a straight tile.
		StraightBlitProxy_Op	m_pStraightBlurOp = nullptr;		// Function called to perform a straight blur.

		TransformBlitProxy_Op m_pTransformBlitOp = nullptr;		// Function called to perform a transform blit.
		TransformBlitProxy_Op m_pTransformClipBlitOp = nullptr;	// Function called to perform a transform clip blit.
		TransformBlitProxy_Op m_pTransformTileOp = nullptr;		// Function called to perform a transform tile.
		TransformBlitProxy_Op m_pTransformBlurOp = nullptr;		// Function called to perform a transform blur.


		// These need to be provided to calls to StraightBlitProxy_Op and TransformBlitProxy_Op.

		StraightBlitOp_p		m_pStraightBlitFirstPassOp = nullptr;
		StraightBlitOp_p		m_pStraightTileFirstPassOp = nullptr;
		StraightBlitOp_p		m_pStraightBlurFirstPassOp = nullptr;
		TransformBlitOp_p		m_pTransformBlitFirstPassOp = nullptr;
		TransformBlitOp_p		m_pTransformClipBlitFirstPassOp = nullptr;
		TransformBlitOp_p		m_pTransformTileFirstPassOp = nullptr;
		TransformBlitOp_p		m_pTransformBlurFirstPassOp = nullptr;

		StraightBlitOp_p		m_pBlitSecondPassOp = nullptr;		// Second pass is same for straight and transform blits and tiles (always a simple blit).
		StraightBlitOp_p		m_pClipBlitSecondPassOp = nullptr;	// Same for transform clip blits, but without BlendMode optimized for opaque sources.



		//



	};


	//____ _forTintSpans() ____________________________________________________

	template<class DrawFunc>
	void SoftBackend::_forTintSpans(const RectI& rect, bool bRuns, DrawFunc&& draw)
	{
		if (rect.w <= 0 || rect.h <= 0)
			return;

		// Draws one row of tint colors (already in buffer) as runs of same color with Flat kernels.

		auto drawRuns = [&](const HiColor* pRow, int y, int h)
		{
			int i = 0;
			while (i < rect.w)
			{
				int j = i + 1;
				while (j < rect.w && pRow[j] == pRow[i])
					j++;

				m_colTrans.flatTintColor = pRow[i];
				draw(RectI(rect.x + i, y, j - i, h));
				i = j;
			}
		};

		switch (m_tintLayout)
		{
			case TintLayout::None:
				draw(rect);
				break;

			case TintLayout::Horizontal:
			{
				HiColor* pRow = _tintRowBuffer(rect.w);
				_generateTintRow(rect.x, rect.y, rect.w, pRow);

				if (bRuns)
					drawRuns(pRow, rect.y, rect.h);
				else
				{
					m_colTrans.pTintAxisX = pRow;
					m_colTrans.tintRect.x = rect.x;
					draw(rect);
				}
				break;
			}

			case TintLayout::Vertical:
			{
				// Rows with same color are drawn together.

				HiColor* pCol = _tintRowBuffer(rect.h);
				for (int i = 0; i < rect.h; i++)
					_generateTintRow(rect.x, rect.y + i, 1, pCol + i);

				int i = 0;
				while (i < rect.h)
				{
					int j = i + 1;
					while (j < rect.h && pCol[j] == pCol[i])
						j++;

					m_colTrans.flatTintColor = pCol[i];
					draw(RectI(rect.x, rect.y + i, rect.w, j - i));
					i = j;
				}
				break;
			}

			case TintLayout::General:
			{
				HiColor* pRow = _tintRowBuffer(rect.w);

				for (int y = rect.y; y < rect.y + rect.h; y++)
				{
					_generateTintRow(rect.x, y, rect.w, pRow);

					if (bRuns)
						drawRuns(pRow, y, 1);
					else
					{
						m_colTrans.pTintAxisX = pRow;
						m_colTrans.tintRect.x = rect.x;
						draw(RectI(rect.x, y, rect.w, 1));
					}
				}
				break;
			}
		}
	}

} // namespace wg
#endif	// WG_SOFTBACKEND_DOT_H

