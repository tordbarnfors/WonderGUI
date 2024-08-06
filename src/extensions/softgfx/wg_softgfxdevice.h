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
#ifndef WG_SOFTGFXDEVICE_DOT_H
#define WG_SOFTGFXDEVICE_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_gfxdevice_gen1.h>
#include <wg_softsurface.h>

namespace wg
{


	struct SegmentEdge
	{
		int			begin;				// Pixeloffset, 24.8 format.
		int			end;				// Pixeloffset, 24.8 format. First pixel after edge (segment after the edge has 100% coverage)
		int			coverage;			// 0-65536
		int			coverageInc;		// Increment of coverage for each full pixel we progress
	};


	class SoftGfxDevice;
	typedef	StrongPtr<SoftGfxDevice>	SoftGfxDevice_p;
	typedef	WeakPtr<SoftGfxDevice>	SoftGfxDevice_wp;

	class SoftGfxDevice : public GfxDeviceGen1
	{
		friend class SoftSurface;
		friend class LinearGfxDevice;
	public:

		//.____ Creation __________________________________________

		static SoftGfxDevice_p	create();

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		const TypeInfo&		surfaceType(void) const override;

		//.____ Misc _______________________________________________________

		bool		defineCanvas( CanvasRef ref, SoftSurface * pSurface );

		using GfxDevice::canvas;
		const CanvasInfo canvas(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		//.____ State _________________________________________________

		void		setTintColor(HiColor color) override;

		void		setTintGradient(const RectSPX& rect, const Gradient& gradient) override;
		void		clearTintGradient() override;

		bool		setBlendMode(BlendMode blendMode) override;
		void		setMorphFactor(float factor) override;
		void		setFixedBlendColor( HiColor color ) override;
		void		setBlurMatrices( spx radius, const float red[9], const float green[9], const float blue[9] ) override;

		//.____ Rendering ________________________________________________

		bool	beginRender() override;
		bool	endRender() override;

		//

		using 	GfxDevice::fill;
		void	fill(const RectSPX& rect, HiColor col) override;

		void    plotPixels(int nCoords, const CoordSPX * pCoords, const HiColor * pColors) override;

		void	drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness = 64) override;
		void	drawLine(CoordSPX begin, Direction dir, spx length, HiColor col, spx thickness = 64) override;

		bool	setBlitSource(Surface * pSource) override;

		void	drawEdgemap(CoordSPX dest, Edgemap * pEdgemap ) override;
		void	flipDrawEdgemap(CoordSPX dest, Edgemap * pEdgemap, GfxFlip flip) override;



		//.____ Internal _____________________________________________________

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
			TintMode mode;

			HiColor	flatTintColor;	// Used in TintMode::Color

			// Following used in TintMode GradientX, GradientY and GradientXY.

			RectI	tintRect;

			int32_t	topLeftR;		// Scale: 0 -> (1<<18)
			int32_t	topLeftG;
			int32_t	topLeftB;
			int32_t	topLeftA;

			int32_t	leftIncR;
			int32_t	leftIncG;
			int32_t	leftIncB;
			int32_t	leftIncA;

			int32_t	topRightR;
			int32_t	topRightG;
			int32_t	topRightB;
			int32_t	topRightA;

			int32_t	rightIncR;
			int32_t	rightIncG;
			int32_t	rightIncB;
			int32_t	rightIncA;

			int			morphFactor;	// Scale: 0 -> 4096
			
			HiColor	fixedBlendColor;
			
			int			blurMtxR[9];
			int			blurMtxG[9];
			int			blurMtxB[9];

			CoordSPX	blurOfsSPX[9];
			CoordI		blurOfsPixel[9];			
		};


		struct SegmentGradient
		{
			int			begR;		// Blue value at top of segment column. 0-255 << 16;
			int			begG;
			int			begB;
			int			begA;

			int			incR;		// Increment of begR for each row.
			int			incG;
			int			incB;
			int			incA;
		};


		struct Pitches
		{
			int srcX;			// Pitch in bytes for each pixel read.
			int srcY;			// Pitch in bytes from end of line to beginning of next for each line read.
			int dstX;			// Pitch in bytes for each pixel written.
			int dstY;			// Pitch in bytes from end of line to beginning of next for each line written.
		};

		typedef	void(*PlotOp_p)(uint8_t * pDst, HiColor col, const ColTrans& tint);
		typedef	void(*PlotListOp_p)(const RectI& clip, int nCoords, const CoordI * pCoords, const HiColor * pColors, uint8_t * pCanvas, int pitchX, int pitchY, const ColTrans& tint);
		typedef	void(*LineOp_p)(uint8_t * pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*ClipLineOp_p)(int clipStart, int clipEnd, uint8_t * pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*FillOp_p)(uint8_t * pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const ColTrans& tint, CoordI patchPos);
		typedef	void(*StraightBlitOp_p)(const uint8_t * pSrc, uint8_t * pDst, const SoftSurface * pSrcSurf, const Pitches& pitches, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos, const int simpleTransform[2][2]);
		typedef	void(*TransformBlitOp_p)(const SoftSurface * pSrcSurf, BinalCoord pos, const binalInt matrix[2][2], uint8_t * pDst, int dstPitchX, int dstPitchY, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos);
		typedef void(*SegmentOp_p)(int clipBeg, int clipEnd, uint8_t * pStripStart, int pixelPitch, int nEdges, SegmentEdge * pEdges, const int16_t * pSegmentColors, const SegmentGradient * pSegmentGradients, const bool * pTransparentSegments, const bool* pOpaqueSegments, const SoftGfxDevice::ColTrans& tint);


		static int				s_lineThicknessTable[17];

		static int 				s_mulTab[256];

		static int16_t			s_limit4096Tab[4097 * 3];

		static bool				s_bTablesInitialized;
	
		static const int16_t 	s_channel_4_1[256];		
		static const int16_t	s_channel_4_2[256];

		static const int16_t	s_channel_5_linear[32];
		static const int16_t	s_channel_6_linear[64];

		static const int16_t	s_channel_5_sRGB[32];
		static const int16_t	s_channel_6_sRGB[64];

		static const uint8_t	s_fast8_channel_4_1[256];
		static const uint8_t 	s_fast8_channel_4_2[256];
		static const uint8_t	s_fast8_channel_5[32];
		static const uint8_t	s_fast8_channel_6[64];
		
		//.____ Control ______________________________________________________

		bool	setPlotKernel(BlendMode blendMode, PixelFormat destFormat, PlotOp_p pKernel);
		bool	setPlotListKernel(BlendMode blendMode, PixelFormat destFormat, PlotListOp_p pKernel);

		bool	setLineKernel(BlendMode blendMode, PixelFormat destFormat, LineOp_p pKernel);
		bool	setClipLineKernel(BlendMode blendMode, PixelFormat destFormat, ClipLineOp_p pKernel);
		bool	setFillKernel(TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, FillOp_p pKernel);

		bool	setSegmentStripKernel(bool bTint, BlendMode blendMode, PixelFormat destFormat, SegmentOp_p pKernel);

		bool	setStraightBlitKernel(PixelFormat sourceFormat, ReadOp readOp, TintMode tintMode,
									  BlendMode blendMode, PixelFormat destFormat, StraightBlitOp_p pKernel);
		
		bool	setTransformBlitKernel(PixelFormat sourceFormat, SampleMethod sampleMethod, ReadOp readOp,
									   TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, TransformBlitOp_p pKernel);

	protected:
		SoftGfxDevice();
		~SoftGfxDevice();
		
		void	_canvasWasChanged() override;
		void	_renderLayerWasChanged() override;


		void	_transformBlitSimple(const RectSPX& dest, CoordSPX src, const int simpleTransform[2][2], OpType type) override;
		void	_transformBlitComplex(const RectSPX& dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type) override;

		void	_transformDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2]) override;

		bool	_setupDestFormatKernels(PixelFormat format);

		inline static void _read_pixel_fast8(const uint8_t* pPixel, PixelFormat format, const Color8* pPalette, const HiColor* pPalette4096, int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA);
		inline static void _write_pixel_fast8(uint8_t* pPixel, PixelFormat format, int16_t b, int16_t g, int16_t r, int16_t a);

		inline static void	_blend_pixels_fast8(BlendMode mode, int morphFactor, PixelFormat destFormat,
			int16_t srcB, int16_t srcG, int16_t srcR, int16_t srcA,
			int16_t backB, int16_t backG, int16_t backR, int16_t backA,
			int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
			int16_t& fixedB, int16_t& fixedG, int16_t& fixedR, int16_t& fixedA);


		inline static void _read_pixel(const uint8_t * pPixel, PixelFormat format, const Color8 * pPalette, const HiColor* pPalette4096, int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA);
		inline static void _write_pixel(uint8_t * pPixel, PixelFormat format, int16_t b, int16_t g, int16_t r, int16_t a);

		inline static void	_blend_pixels(	BlendMode mode, int morphFactor, PixelFormat destFormat,
											int16_t srcB, int16_t srcG, int16_t srcR, int16_t srcA,
											int16_t backB, int16_t backG, int16_t backR, int16_t backA,
											int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
											int16_t& fixedB, int16_t& fixedG, int16_t& fixedR, int16_t& fixedA);

		inline static void	_color_tint_init(	TintMode tintMode, const ColTrans& tint, int bits, int16_t inB, int16_t inG, int16_t inR, int16_t inA,
												int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
												uint32_t& leftB, uint32_t& leftG, uint32_t& leftR, uint32_t& leftA,
												uint32_t& rightB, uint32_t& rightG, uint32_t& rightR, uint32_t& rightA, 
												uint32_t& leftIncB, uint32_t& leftIncG, uint32_t& leftIncR, uint32_t& leftIncA,
												uint32_t& rightIncB, uint32_t& rightIncG, uint32_t& rightIncR, uint32_t& rightIncA,
												uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA, CoordI patchOfs);
			
		inline static void _color_tint_line(	TintMode tintMode, const ColTrans& tint, int bits, int16_t inB, int16_t inG, int16_t inR, int16_t inA,
												int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
												uint32_t& leftB, uint32_t& leftG, uint32_t& leftR, uint32_t& leftA,
												uint32_t& rightB, uint32_t& rightG, uint32_t& rightR, uint32_t& rightA,
												uint32_t& leftIncB, uint32_t& leftIncG, uint32_t& leftIncR, uint32_t& leftIncA,
												uint32_t& rightIncB, uint32_t& rightIncG, uint32_t& rightIncR, uint32_t& rightIncA,
												uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA,
												uint32_t& pixelB, uint32_t& pixelG, uint32_t& pixelR, uint32_t& pixelA, CoordI patchOfs);

		inline static void _color_tint_pixel(TintMode tintMode, int bits,
											int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
											uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA,
											uint32_t& pixelB, uint32_t& pixelG, uint32_t& pixelR, uint32_t& pixelA);

		inline static void _texel_tint_init(TintMode tintMode, const ColTrans& tint, int bits,
											uint32_t& leftB, uint32_t& leftG, uint32_t& leftR, uint32_t& leftA,
											uint32_t& rightB, uint32_t& rightG, uint32_t& rightR, uint32_t& rightA,
											uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA,
											uint32_t& tintB, uint32_t& tintG, uint32_t& tintR, uint32_t& tintA, CoordI patchOfs);

		inline static void _texel_tint_line(TintMode tintMode, const ColTrans& tint, int bits,
											uint32_t& leftB, uint32_t& leftG, uint32_t& leftR, uint32_t& leftA,
											uint32_t& rightB, uint32_t& rightG, uint32_t& rightR, uint32_t& rightA,
											uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA,
											uint32_t& tintB, uint32_t& tintG, uint32_t& tintR, uint32_t& tintA, CoordI patchOfs);

		inline static void _texel_tint_pixel(TintMode tintMode, int bits, int16_t& pixelB, int16_t& pixelG, int16_t& pixelR, int16_t& pixelA,
											uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA,
											uint32_t& tintB, uint32_t& tintG, uint32_t& tintR, uint32_t& tintA);

		inline static int _segment_alpha(bool GRADIENT, int offset, const int16_t * pSegmentColor, const SegmentGradient * pSegmentGradient);
		inline static void _add_segment_color(bool GRADIENT, int blendFraction, int offset, const int16_t * pSegmentColor, const SegmentGradient * pSegmentGradient, int& accB, int& accG, int& accR, int& accA);


/*
		inline static void _init_tint_color(TintMode tintMode, const ColTrans& tint, uint8_t inB, uint8_t inG, uint8_t inR, uint8_t inA,
											uint8_t& outB, uint8_t& outG, uint8_t& outR, uint8_t& outA);

		inline static void _init_tint_color_line(int lineNb, TintMode tintMode, const ColTrans& tint,
											uint8_t inB, uint8_t inG, uint8_t inR, uint8_t inA, uint8_t& outB, uint8_t& outG, uint8_t& outR, uint8_t& outA);

		inline static void _tint_color(int columnNb, TintMode tintMode, const ColTrans& tint,
											uint8_t inB, uint8_t inG, uint8_t inR, uint8_t inA,
											uint8_t& outB, uint8_t& outG, uint8_t& outR, uint8_t& outA);

		inline static void _init_tint_texel(TintMode tintMode, const ColTrans& tint, uint8_t& outB, uint8_t& outG, uint8_t& outR, uint8_t& outA);

		inline static void _init_tint_texel_line(int lineNb, TintMode tintMode, const ColTrans& tint, uint8_t inB, uint8_t inG, uint8_t inR, uint8_t inA,
											uint8_t& outB, uint8_t& outG, uint8_t& outR, uint8_t& outA);

		inline static void _tint_texel(int columnNb, TintMode tintMode, const ColTrans& tint, uint8_t texB, uint8_t texG, uint8_t texR, uint8_t texA,
											uint8_t inB, uint8_t inG, uint8_t inR, uint8_t inA,
											uint8_t& outB, uint8_t& outG, uint8_t& outR, uint8_t& outA );
*/

		void	_lineToEdges(const WaveLine * pWave, int offset, int nPoints, SegmentEdge * pDest, int pitch);

		static void	_initTables();
		virtual void	_updateBlitFunctions();
		int 	_scaleLineThickness(float thickness, int slope);

		void	_updateTintSettings();

		void	_colTransRect(int baseB[4], int baseG[4], int baseR[4], int baseA[], const RectI& _dest);

		//

		typedef void(SoftGfxDevice::*StraightBlitProxy_Op)(const RectI& dest, CoordI src, const int simpleTransform[2][2], CoordI patchPos, StraightBlitOp_p pPassOneOp);
		typedef void(SoftGfxDevice::*TransformBlitProxy_Op)(const RectI& dest, BinalCoord pos, const binalInt matrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);


		void	_onePassStraightBlit(const RectI& dest, CoordI pos, const int simpleTransform[2][2], CoordI patchPos, StraightBlitOp_p pPassOneOp);
		void	_twoPassStraightBlit(const RectI& dest, CoordI pos, const int simpleTransform[2][2], CoordI patchPos, StraightBlitOp_p pPassOneOp);

		void	_onePassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt matrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);
		void	_twoPassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt matrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);

		void	_dummyStraightBlit(const RectI& dest, CoordI pos, const int simpleTransform[2][2], CoordI patchPos, StraightBlitOp_p pPassOneOp);
		void	_dummyTransformBlit(const RectI& dest, BinalCoord pos, const binalInt matrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);

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
			
			PlotOp_p 				pPlotKernels[BlendMode_size];
			LineOp_p 				pLineKernels[BlendMode_size];
			ClipLineOp_p			pClipLineKernels[BlendMode_size];
			FillOp_p				pFillKernels[TintMode_size][BlendMode_size];
			PlotListOp_p			pPlotListKernels[BlendMode_size];
			SegmentOp_p				pSegmentKernels[2][BlendMode_size];

			StraightBlitOp_p		pStraightBlitFromHiColorKernels[TintMode_size][BlendMode_size];
			StraightBlitOp_p		pStraightBlitFromBGRA8Kernels[TintMode_size][BlendMode_size];

			uint16_t				singlePassBlitKernels[PixelFormat_size];		// PixelFormat of blit source. Offset into m_singlePassBlitKernels +1.
		};

		StraightBlitOp_p	m_pStraightMoveToBGRA8Kernels[PixelFormat_size][ReadOp_size];
		StraightBlitOp_p	m_pStraightMoveToHiColorKernels[PixelFormat_size][ReadOp_size];

		TransformBlitOp_p	m_pTransformMoveToBGRA8Kernels[PixelFormat_size][SampleMethod_size][ReadOp_size];
		TransformBlitOp_p	m_pTransformMoveToHiColorKernels[PixelFormat_size][SampleMethod_size][ReadOp_size];

		DestFormatKernels *	m_pKernels[PixelFormat_size];
	
		std::vector<SinglePassBlitKernels>			m_singlePassBlitKernels;
		std::vector<SinglePassStraightBlitKernels>	m_singlePassStraightBlitKernels;
		std::vector<SinglePassTransformBlitKernels>	m_singlePassTransformBlitKernels;


		//

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;


		// Members controlling render states

		ColTrans			m_colTrans = { TintMode::None, HiColor(4096,4096,4096,4096), {0,0,0,0} };		// Color transformation data

		bool				m_bTintOpaque = true;						// Set if tint alpha is 255 after combining tintColor and gradient.

		SoftSurface_p		m_pBlitSource				= nullptr;		// Source surface for blits.
		bool				m_bClipSource				= false;		// Set if transform blit might need to do clipping.

		// These are called to perform blit/tile operations

		StraightBlitProxy_Op	m_pStraightBlitOp = nullptr;		// Function called to perform a straight blit.
		StraightBlitProxy_Op	m_pStraightTileOp = nullptr;		// Function called to perform a straight tile.
		StraightBlitProxy_Op	m_pStraightBlurOp = nullptr;		// Function called to perform a straight blur.

		TransformBlitProxy_Op m_pTransformBlitOp = nullptr;		// Function called to perform a transform blit.
		TransformBlitProxy_Op m_pTransformClipBlitOp = nullptr;	// Function called to perform a transform clip blit.
		TransformBlitProxy_Op m_pTransformTileOp = nullptr;		// Function called to perform a transform tile.
		TransformBlitProxy_Op m_pTransformBlurOp = nullptr;		// Function called to perform a transform blur.


		// These need to be provided to calls to StraightBlitProxy_Op and TransformBlitProxy_Op.

		StraightBlitOp_p		m_pStraightBlitFirstPassOp		= nullptr;
		StraightBlitOp_p		m_pStraightTileFirstPassOp		= nullptr;
		StraightBlitOp_p		m_pStraightBlurFirstPassOp		= nullptr;
		TransformBlitOp_p		m_pTransformBlitFirstPassOp		= nullptr;
		TransformBlitOp_p		m_pTransformClipBlitFirstPassOp	= nullptr;
		TransformBlitOp_p		m_pTransformTileFirstPassOp		= nullptr;
		TransformBlitOp_p		m_pTransformBlurFirstPassOp		= nullptr;

		StraightBlitOp_p		m_pBlitSecondPassOp				= nullptr;		// Second pass is same for straight and transform blits and tiles (always a simple blit).

		//
		
		std::vector<CanvasInfo>	m_definedCanvases;

		Surface_p		m_pRenderLayerSurface = nullptr;	// render layer surface
		PixelBuffer		m_canvasPixelBuffer;
		uint8_t *		m_pCanvasPixels;	// Pixels of render layer surface
		int				m_canvasPixelBits;	// PixelBits of render layer surface
		int				m_canvasPitch;
		PixelFormat		m_canvasPixelFormat = PixelFormat::Undefined;
	};


} // namespace wg
#endif //WG_SOFTGFXDEVICE_DOT_H

