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
#ifndef WG2_SWITCH_DOT_H
#define WG2_SWITCH_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#include <wg2_color.h>

//____ WgSwitch ____________________________________________________________

class WgSwitch : public WgWidget
{
public:
	WgSwitch();
	virtual ~WgSwitch();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgSwitch(); };

	void    SetValue( const float value );
	void    SetColor( WgColor color );
	void    SetOffColor(WgColor color );

	WgSize	PreferredPixelSize() const;
	void    SetPreferredPixelSize(WgSize size);

protected:
	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onEnable();
	void	_onDisable();

private:
	int m_iValue;
	WgColor m_onColor;
	WgColor m_offColor;
	WgSize m_preferredSize;

};


#endif //WG2_SWITCH_DOT_H
