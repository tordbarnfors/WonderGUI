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

#ifndef	WG_STREAMTRIMBACKEND_DOT_H
#define WG_STREAMTRIMBACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>
#include <vector>
#include <deque>

namespace wg
{

	class StreamTrimBackend;
	typedef	StrongPtr<StreamTrimBackend>	StreamTrimBackend_p;
	typedef	WeakPtr<StreamTrimBackend>	StreamTrimBackend_wp;


	//____ StreamTrimBackend __________________________________________________________

	class StreamTrimBackend : public GfxBackend
	{
	public:
		
		//.____ Creation __________________________________________

		static StreamTrimBackend_p		create(GfxBackend * pBackend) { return StreamTrimBackend_p(new StreamTrimBackend(pBackend)); }


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
		void	setObjects(Object** pBeg, Object** pEnd) override;
		void	setRects(RectSPX* pBeg, RectSPX* pEnd) override;
		void	setColors(HiColor* pBeg, HiColor* pEnd) override;
		void	setTransforms(Transform * pBeg, Transform * pEnd) override;
		void	processCommands(int32_t* pBeg, int32_t* pEnd) override;

		//.____ Behavior ______________________________________________________

		void	addNonMaskingSession();
		void	addFullyMaskingSession( CanvasRef canvas, Surface * pCanvas );
		void	addMaskingSession( CanvasRef canvas, Surface * pCanvas, int nMaskingRects, const RectSPX * pMaskingRects );
		void	clearSessionMasks();
		void	setTrimLevel(int level);

		//.____ Misc _________________________________________________________

		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		int		maxEdges() const override;

		const TypeInfo& surfaceType(void) const override;


	protected:
		StreamTrimBackend(GfxBackend* pBackend) : m_pBackend(pBackend) {}

		virtual ~StreamTrimBackend();

		void		_trimFrames();
		void		_renderFrames();

		void		_trim( RectSPX * pToTrim, RectSPX * pMaskBeg, RectSPX * pMaskEnd );


		GfxBackend_p	m_pBackend;

		// Update rects as presented to us before trimming.

		const RectSPX*				m_pUpdateRectsBeg;
		const RectSPX*				m_pUpdateRectsEnd;

		// Update rects trimmed down by the masks, but none removed even if empty.

		std::vector<RectSPX>		m_trimmedUpdateRects;

		// Rendering rects trimmed down by m_trimmedUpdateRects.

		std::vector<RectSPX>		m_renderingRects;

		struct SessionMask
		{
			CanvasRef	canvasRef;
			Surface *	pCanvas;

			std::vector<RectSPX>	rects;
		};

		std::deque <SessionMask>	m_masks;
		int							m_trimLevel = 0;

	};

} // namespace wg
#endif	// WG_STREAMTRIMBACKEND_DOT_H

