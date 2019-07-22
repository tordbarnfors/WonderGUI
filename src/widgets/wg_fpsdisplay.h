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

#ifndef	WG_FPSDISPLAY_DOT_H
#define WG_FPSDISPLAY_DOT_H
#pragma once

#include <wg_widget.h>
#include	<wg_itextdisplay.h>

namespace wg
{


	class Font;

	class FpsDisplay;
	typedef	StrongPtr<FpsDisplay>		FpsDisplay_p;
	typedef	WeakPtr<FpsDisplay>	FpsDisplay_wp;

	class FpsDisplay : public Widget
	{
	public:

		//.____ Creation __________________________________________

		static FpsDisplay_p	create() { return FpsDisplay_p(new FpsDisplay()); }


		//.____ Interfaces _______________________________________

		ITextDisplay		labels;
		IROTextDisplay		values;


		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static FpsDisplay_p	cast( Object * pObject );


		//.____ Geometry ____________________________________________

		SizeI	preferredSize() const;

	protected:
		FpsDisplay();
		virtual ~FpsDisplay();
		virtual Widget* _newOfMyType() const { return new FpsDisplay(); };

		void		_receive( Msg * pMsg );
		void		_setState( State state );
		void		_render( GfxDevice * pDevice, const RectI& _canvas, const RectI& _window );
		void		_cloneContent( const Widget * _pOrg );

	private:

	const static int c_tickBuffer = 64;


		CTextDisplay	m_labelsText;
		CTextDisplay	m_valuesText;
		int *			m_pTickBuffer;
		int				m_tickBufferOfs;
		RouteId			m_tickRouteId;
	};




} // namespace wg
#endif //FpsDisplay_DOT_H
