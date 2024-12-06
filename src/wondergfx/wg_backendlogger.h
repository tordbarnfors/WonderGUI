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

#ifndef	WG_BACKENDLOGGER_DOT_H
#define WG_BACKENDLOGGER_DOT_H
#pragma once

#include <wg_gfxbackend.h>
#include <iostream>


namespace wg
{

	class BackendLogger;
	typedef	StrongPtr<BackendLogger>	BackendLogger_p;
	typedef	WeakPtr<BackendLogger>		BackendLogger_wp;


	//____ BackendLogger __________________________________________________________

	class BackendLogger : public GfxBackend
	{
	public:
		
		//.____ Creation __________________________________________

		static BackendLogger_p		create(std::ostream& stream, GfxBackend * pBackend) { return BackendLogger_p(new BackendLogger(stream, pBackend)); }


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

		//.____ Misc _________________________________________________________

		const CanvasInfo *	canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		int		maxEdges() const override;

		const TypeInfo& surfaceType(void) const override;


	protected:
		BackendLogger(std::ostream& stream, GfxBackend* pBackend) : m_pBackend(pBackend), m_pStream(&stream) {}

		virtual ~BackendLogger() {};

		void	_printRects(std::ostream& stream, int nRects, const RectSPX* pRects);

		HiColor*		m_pColorsBeg = nullptr;
		HiColor*		m_pColorsEnd = nullptr;
		HiColor*		m_pColorsPtr = nullptr;

		RectSPX*		m_pRectsBeg = nullptr;
		RectSPX*		m_pRectsEnd = nullptr;
		RectSPX*		m_pRectsPtr = nullptr;



		GfxBackend_p	m_pBackend;

		std::ostream*	m_pStream = nullptr;

	};

} // namespace wg
#endif	// WG_BACKENDLOGGER_DOT_H

