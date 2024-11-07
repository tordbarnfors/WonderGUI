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

#ifndef	WG_LINEARBACKEND_DOT_H
#define WG_LINEARBACKEND_DOT_H
#pragma once

#include <wg_softbackend.h>
#include <wg_softsurface.h>

namespace wg
{
	class LinearBackend;
	typedef	StrongPtr<LinearBackend>	LinearBackend_p;
	typedef	WeakPtr<LinearBackend>		LinearBackend_wp;


	//____ LinearBackend __________________________________________________________

	class LinearBackend : public SoftBackend
	{

	public:

		//.____ Creation __________________________________________

		static LinearBackend_p	create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________
	
		void	beginSession(const SessionInfo* pSession) override;
		void	endSession() override;

		void	processCommands( int32_t* pBeg, int32_t * pEnd) override;


	protected:
		LinearBackend();
		virtual ~LinearBackend();

		void	_updateBlitFunctions();

		void	_onePassStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);
		void	_twoPassStraightBlit(const RectI& dest, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp);

		void	_onePassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);
		void	_twoPassTransformBlit(const RectI& dest, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp);

	};

} // namespace wg
#endif	// WG_LINEARBACKEND_DOT_H

