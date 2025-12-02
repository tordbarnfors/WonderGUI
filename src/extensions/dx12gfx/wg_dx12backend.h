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

#ifndef	WG_DX12BACKEND_DOT_H
#define WG_DX12BACKEND_DOT_H
#pragma once

#include <wg_gfxbackend.h>

namespace wg
{


	class DX12Backend;
	typedef	StrongPtr<DX12Backend>	DX12Backend_p;
	typedef	WeakPtr<DX12Backend>	DX12Backend_wp;


	//____ DX12Backend __________________________________________________________

	class DX12Backend : public GfxBackend
	{
	public:

		//.____ Creation __________________________________________

		static DX12Backend_p	create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		void	beginSession(CanvasRef canvasRef, Surface* pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, const SessionInfo* pInfo = nullptr) override;
		void	endSession() override;

		void	setCanvas(Surface* pSurface) override;
		void	setCanvas(CanvasRef ref) override;

		void	setObjects(Object* const* pBeg, Object* const* pEnd) override;

		void	setRects(const RectSPX* pBeg, const RectSPX* pEnd) override;
		void	setColors(const HiColor* pBeg, const HiColor* pEnd) override;
		void	setTransforms(const Transform* pBeg, const Transform* pEnd) override;

		void	processCommands(const uint16_t* pBeg, const uint16_t* pEnd) override;


		//.____ Misc _________________________________________________________

		bool	setDefaultCanvas(SizeSPX size, int scale);

		const CanvasInfo* canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		int		maxEdges() const override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

	protected:
		DX12Backend();
		~DX12Backend();

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;

		CanvasInfo			m_defaultCanvas;
		CanvasInfo			m_dummyCanvas;

	};

} // namespace wg
#endif	// WG_DX12BACKEND_DOT_H

