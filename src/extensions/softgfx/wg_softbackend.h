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



	protected:
		SoftBackend();
		virtual ~SoftBackend();

		std::vector<CanvasInfo>	m_definedCanvases;

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;


	};

} // namespace wg
#endif	// WG_SOFTBACKEND_DOT_H

