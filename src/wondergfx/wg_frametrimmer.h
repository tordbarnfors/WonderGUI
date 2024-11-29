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

#ifndef	WG_FRAMETRIMMER_DOT_H
#define WG_FRAMETRIMMER_DOT_H
#pragma once

#include <wg_gfxbackend.h>


namespace wg
{

	class FrameTrimmer;
	typedef	StrongPtr<FrameTrimmer>	FrameTrimmer_p;
	typedef	WeakPtr<FrameTrimmer>	FrameTrimmer_wp;


	//____ BackendLogger __________________________________________________________

	class FrameTrimmer : public GfxBackend
	{
	public:
		
		//.____ Creation __________________________________________

		static FrameTrimmer_p		create(GfxBackend * pBackend) { return FrameTrimmer_p(new FrameTrimmer(pBackend)); }


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
		void	setRects(RectSPX* pBeg, RectSPX* pEnd) override;
		void	setColors(HiColor* pBeg, HiColor* pEnd) override;
		void	setTransforms(Transform * pBeg, Transform * pEnd) override;
		void	processCommands(int32_t* pBeg, int32_t* pEnd) override;

		void	flush();

		//.____ Misc _________________________________________________________

		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		int		maxEdges() const override;

		const TypeInfo& surfaceType(void) const override;

		void	setTrimLevel( int level );

	protected:
		FrameTrimmer(GfxBackend* pBackend) : m_pBackend(pBackend) {}

		virtual ~FrameTrimmer();

		void		_trimFrames();
		void		_renderFrames();

		void		_trim( RectSPX * pToTrim, RectSPX * pMaskBeg, RectSPX * pMaskEnd, RectSPX * pDrawBeg, RectSPX * pDrawEnd );


		GfxBackend_p	m_pBackend;

		struct CommandSet
		{
			Surface * pCanvas;
			CanvasRef canvasRef;

			size_t 		commandsBeg;
			size_t		commandsEnd;

			size_t		objectsBeg;
			size_t		objectsEnd;

			size_t		rectsBeg;
			size_t		rectsEnd;

			size_t		colorsBeg;
			size_t		colorsEnd;

			size_t		transformsBeg;
			size_t		transformsEnd;
		};

		struct SessionData
		{
			SessionInfo		info;

			std::vector<CommandSet>	commandSets;

			size_t			updateRectsBeg;
			size_t			updateRectsEnd;

			size_t			commandRectsBeg;
			size_t			commandRectsEnd;
		};

		std::vector<SessionData>	m_sessions;

		int							m_trimLevel = 2;

		// Local copies of all our buffers

		std::vector<RectSPX>		m_rects;
		std::vector<HiColor>		m_colors;
		std::vector<Object*>		m_objects;
		std::vector<Transform>		m_transforms;
		std::vector<int32_t>		m_commands;

		// Current state

		Surface *		m_pCanvas = nullptr;
		CanvasRef		m_canvasRef = CanvasRef::None;

		size_t			m_objectsBeg;
		size_t			m_objectsEnd;

		size_t			m_rectsBeg;
		size_t			m_rectsEnd;

		size_t			m_colorsBeg;
		size_t			m_colorsEnd;

		size_t			m_transformsBeg;
		size_t			m_transformsEnd;
	};

} // namespace wg
#endif	// WG_FRAMETRIMMER_DOT_H

