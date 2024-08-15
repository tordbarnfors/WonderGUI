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
		
		void	setCanvas( Surface * pSurface ) override;
		void	setCanvas( CanvasRef ref ) override;

		void	setObjects(Object** pBeg, Object** pEnd) override;

		void	setCoords(spx* pBeg, spx* pEnd) override;
		void	setTransforms(Transform * pBeg, Transform * pEnd) override;

		void	processCommands( int32_t* pBeg, int32_t * pEnd) override;



		//.____ Misc _________________________________________________________

		bool	defineCanvas(CanvasRef ref, SoftSurface* pSurface);

		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;


		int		maxEdges() const override;

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

		typedef	void(*PlotOp_p)(uint8_t* pDst, HiColor col, const ColTrans& tint);
		typedef	void(*PlotListOp_p)(const RectI& clip, int nCoords, const CoordI* pCoords, const HiColor* pColors, uint8_t* pCanvas, int pitchX, int pitchY, const ColTrans& tint);
		typedef	void(*LineOp_p)(uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*ClipLineOp_p)(int clipStart, int clipEnd, uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const ColTrans& tint, CoordI patchPos);
		typedef	void(*FillOp_p)(uint8_t* pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const ColTrans& tint, CoordI patchPos);
		typedef	void(*StraightBlitOp_p)(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const Pitches& pitches, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos, const int simpleTransform[2][2]);
		typedef	void(*TransformBlitOp_p)(const SoftSurface* pSrcSurf, BinalCoord pos, const binalInt matrix[2][2], uint8_t* pDst, int dstPitchX, int dstPitchY, int nLines, int lineLength, const ColTrans& tint, CoordI patchPos);
		typedef void(*SegmentOp_p)(int clipBeg, int clipEnd, uint8_t* pStripStart, int pixelPitch, int nEdges, SegmentEdge* pEdges, const int16_t* pSegmentColors, const HiColor ** pSegmentTintmaps, const bool* pTransparentSegments, const bool* pOpaqueSegments, const ColTrans& tint);

		//.____ Control ______________________________________________________

		bool	setFillKernel( TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, FillOp_p pKernel);

		bool	setStraightBlitKernel(	PixelFormat sourceFormat, ReadOp readOp, TintMode tintMode,
										BlendMode blendMode, PixelFormat destFormat, StraightBlitOp_p pKernel);

		
/*
		bool	setPlotKernel(BlendMode blendMode, PixelFormat destFormat, PlotOp_p pKernel);
		bool	setPlotListKernel(BlendMode blendMode, PixelFormat destFormat, PlotListOp_p pKernel);

		bool	setLineKernel(BlendMode blendMode, PixelFormat destFormat, LineOp_p pKernel);
		bool	setClipLineKernel(BlendMode blendMode, PixelFormat destFormat, ClipLineOp_p pKernel);

		bool	setSegmentStripKernel(bool bTint, BlendMode blendMode, PixelFormat destFormat, SegmentOp_p pKernel);


		bool	setTransformBlitKernel(PixelFormat sourceFormat, SampleMethod sampleMethod, ReadOp readOp,
			TintMode tintMode, BlendMode blendMode, PixelFormat destFormat, TransformBlitOp_p pKernel);
*/



	protected:
		SoftBackend();
		virtual ~SoftBackend();

		void _initTables();

		std::vector<CanvasInfo>	m_definedCanvases;

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;


		static int				s_lineThicknessTable[17];

		static int 				s_mulTab[256];

		static int16_t			s_limit4096Tab[4097 * 3];

		static bool				s_bTablesInitialized;

		static const int16_t 	s_channel_4_1[256];
		static const int16_t	s_channel_4_2[256];
		static const int16_t	s_channel_5[32];
		static const int16_t	s_channel_6[64];
		static const uint8_t	s_fast8_channel_4_1[256];
		static const uint8_t 	s_fast8_channel_4_2[256];
		static const uint8_t	s_fast8_channel_5[32];
		static const uint8_t	s_fast8_channel_6[64];





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

		DestFormatKernels* m_pKernels[PixelFormat_size];

		std::vector<SinglePassBlitKernels>			m_singlePassBlitKernels;
		std::vector<SinglePassStraightBlitKernels>	m_singlePassStraightBlitKernels;
		std::vector<SinglePassTransformBlitKernels>	m_singlePassTransformBlitKernels;


		//



	};

} // namespace wg
#endif	// WG_SOFTBACKEND_DOT_H

