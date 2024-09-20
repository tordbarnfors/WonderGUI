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

#ifndef	WG_SOFTBACKEND_DOT_H
#define WG_SOFTBACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>
#include <wg_softsurface.h>

namespace wg
{


	class SoftBackend;
	typedef	StrongPtr<SoftBackend>	SoftBackend_p;
	typedef	WeakPtr<SoftBackend>	SoftBackend_wp;


	//____ SoftBackend __________________________________________________________

	class SoftBackend : public GfxBackend
	{

	public:

		//.____ Creation __________________________________________

		static SoftBackend_p	create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;
	
		void	beginSession(const SessionInfo* pSession) override;
		void	endSession() override;

		void	setCanvas( Surface * pSurface ) override;
		void	setCanvas( CanvasRef ref ) override;

		void	setObjects(Object** pBeg, Object** pEnd) override;

		void	setCoords(spx * pBeg, spx * pEnd) override;
		void	setColors(HiColor * pBeg, HiColor * pEnd) override;
		void	setTransforms(Transform * pBeg, Transform * pEnd) override;

		void	processCommands( int32_t* pBeg, int32_t * pEnd) override;


		//.____ Misc _________________________________________________________

		bool	defineCanvas(CanvasRef ref, SoftSurface* pSurface);

		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;


		int		maxEdges() const override;

		const TypeInfo& surfaceType(void) const override;


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

			HiColor*	pTintAxisX;
			HiColor*	pTintAxisY;

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
		
		typedef	void(*PlotListOp_p)(int nCoords, const CoordSPX* pCoords, const HiColor* pColors, uint8_t* pCanvas, int pitchX, int pitchY, const ColTrans& tint);
		typedef	void(*LineOp_p)(uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*ClipLineOp_p)(int clipStart, int clipEnd, uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*FillOp_p)(uint8_t* pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const ColTrans& tint, CoordI patchPos);
		typedef	void(*StraightBlitOp_p)(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const Pitches& pitches, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos, const Transform * pMatrix);
		typedef	void(*TransformBlitOp_p)(const SoftSurface* pSrcSurf, BinalCoord pos, const binalInt matrix[2][2], uint8_t* pDst, int dstPitchX, int dstPitchY, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos);
		typedef void(*SegmentOp_p)(int clipBeg, int clipEnd, uint8_t* pStripStart, int pixelPitch, int nEdges, SegmentEdge* pEdges, const int16_t* pSegmentColors, const HiColor * pSegmentTintmap, int segmentTintmapPitch, const bool* pTransparentSegments, const bool* pOpaqueSegments, const ColTrans& tint);

		static const int16_t 	s_channel_4_1[256];
		static const int16_t	s_channel_4_2[256];
		static const int16_t	s_channel_5[32];
		static const int16_t	s_channel_6[64];
		static const uint8_t	s_fast8_channel_4_1[256];
		static const uint8_t 	s_fast8_channel_4_2[256];
		static const uint8_t	s_fast8_channel_5[32];
		static const uint8_t	s_fast8_channel_6[64];

		static int 				s_mulTab[256];

		static int16_t			s_limit4096Tab[4097 * 3];

		const static int	c_maxSegments = 16;


		//.____ Control ______________________________________________________

		bool	setFillKernel( TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, FillOp_p pKernel);

		bool	setStraightBlitKernel(	PixelFormat sourceFormat, ReadOp readOp, TintMode tintMode,
										BlendMode blendMode, PixelFormat destFormat, StraightBlitOp_p pKernel);

		

		bool	setPlotListKernel(BlendMode blendMode, PixelFormat destFormat, PlotListOp_p pKernel);

		bool	setLineKernel(BlendMode blendMode, PixelFormat destFormat, LineOp_p pKernel);
		bool	setClipLineKernel(BlendMode blendMode, PixelFormat destFormat, ClipLineOp_p pKernel);

		bool	setSegmentStripKernel(SoftBackend::StripSource, BlendMode blendMode, PixelFormat destFormat, SegmentOp_p pKernel);


		bool	setTransformBlitKernel(PixelFormat sourceFormat, SampleMethod sampleMethod, ReadOp readOp,
			TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, TransformBlitOp_p pKernel);



	protected:
		SoftBackend();
		virtual ~SoftBackend();

		void _initTables();
		bool _setupDestFormatKernels(PixelFormat format);

		void	_updateBlitFunctions();
		void	_updateTintMode();
		void	_updateBlur(Blurbrush * pBrush);

		int		_scaleLineThickness(float thickness, int slope);




		std::vector<CanvasInfo>	m_definedCanvases;

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;

		Object **			m_pObjectsBeg = nullptr;
		Object **			m_pObjectsEnd = nullptr;

		spx *				m_pCoordsBeg = nullptr;
		spx *				m_pCoordsEnd = nullptr;
		spx *				m_pCoordsPtr = nullptr;

		HiColor*			m_pColorsBeg = nullptr;
		HiColor*			m_pColorsEnd = nullptr;
		HiColor*			m_pColorsPtr = nullptr;

		Transform *			m_pTransformsBeg = nullptr;
		Transform *			m_pTransformsEnd = nullptr;

		// Canvas related

		SoftSurface_p		m_pCanvas;
		PixelBuffer			m_buffer;
		uint8_t*			m_pCanvasPixels;	// Pixels of render layer surface
		int					m_canvasPixelBytes = 0;			// Bytes per pixel of render layer surface
		PixelFormat			m_canvasPixelFormat = PixelFormat::Undefined;
		int					m_canvasPitch = 0;

		// Current state

		BlendMode			m_blendMode = BlendMode::Blend;
//		TintMode			m_tintMode = TintMode::None;

		ColTrans			m_colTrans;

		SoftSurface_p		m_pBlitSource;

		bool				m_bBlitFunctionNeedsUpdate = true;

		// Tintmap buffer

		HiColor *			m_pTintmapXBuffer = nullptr;
		HiColor *			m_pTintmapYBuffer = nullptr;
		int					m_tintmapXBufferSize = 0;
		int					m_tintmapYBufferSize = 0;


//		PixelBuffer		m_canvasPixelBuffer;
//		uint8_t* m_pCanvasPixels;	// Pixels of render layer surface

		static int				s_lineThicknessTable[17];


		static bool				s_bTablesInitialized;




		//

		typedef void(SoftBackend::* StraightBlitProxy_Op)(const RectI& dest, CoordI src, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);
		typedef void(SoftBackend::* TransformBlitProxy_Op)(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);


		void	_onePassStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);
		void	_twoPassStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);

		void	_onePassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);
		void	_twoPassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);

		void	_dummyStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);
		void	_dummyTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);

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
			PlotListOp_p			pPlotListKernels[BlendMode_size];
			SegmentOp_p				pSegmentKernels[3][BlendMode_size];

			StraightBlitOp_p		pStraightBlitFromHiColorKernels[TintMode_size][BlendMode_size];
			StraightBlitOp_p		pStraightBlitFromBGRA8Kernels[TintMode_size][BlendMode_size];

			uint16_t				singlePassBlitKernels[PixelFormat_size];		// PixelFormat of blit source. Offset into m_singlePassBlitKernels +1.
		};

		StraightBlitOp_p	m_pStraightMoveToBGRA8Kernels[PixelFormat_size][ReadOp_size];
		StraightBlitOp_p	m_pStraightMoveToHiColorKernels[PixelFormat_size][ReadOp_size];

		TransformBlitOp_p	m_pTransformMoveToBGRA8Kernels[PixelFormat_size][SampleMethod_size][ReadOp_size];
		TransformBlitOp_p	m_pTransformMoveToHiColorKernels[PixelFormat_size][SampleMethod_size][ReadOp_size];

		DestFormatKernels* m_pKernels[PixelFormat_size];

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



		//



	};

} // namespace wg
#endif	// WG_SOFTBACKEND_DOT_H

