/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#ifndef	WG2_FPSDISPLAY_DOT_H
#define WG2_FPSDISPLAY_DOT_H

#ifndef	WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#include	<wg_textstyle.h>


class WgFpsDisplay:public WgWidget
{
	public:
		WgFpsDisplay();
		virtual ~WgFpsDisplay();
		virtual const char * Type() const;
		static const char * GetClass();
		virtual WgWidget * NewOfMyType() const { return new WgFpsDisplay(); };


		//____ Methods __________________________________________

		void	SetTextStyle( wg::TextStyle * pStyle );
		WgSize	PreferredPixelSize() const;

	protected:
		void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
		void	_onCloneContent( const WgWidget * _pOrg );

	private:

		wg::TextStyle *	m_pStyle;


		Uint32 *		m_pTickBuffer;
		Uint32			m_tickBufferOfs;

};



#endif //WgFpsDisplay_DOT_H
