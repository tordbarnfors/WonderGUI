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
#ifndef WG2_FILL_DOT_H
#define WG2_FILL_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_COLORSET_DOT_H
#	include <wg2_colorset.h>
#endif

//____ WgFiller ____________________________________________________________

class WgFiller : public WgWidget
{
public:
	WgFiller();
	virtual ~WgFiller();

	virtual const char *Type( void ) const override;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const override{ return new WgFiller(); };

	void	SetColors( const WgColorsetPtr& pColors );
	const	WgColorsetPtr	GetColors() const		{ return m_pColors; }

	void	SetPreferredPointSize( const WgSize& size );
	WgSize	PreferredPixelSize() const override;

	void	SetSkinValue( float value );
	float	SkinValue() const { return m_skinValue; }

protected:

	bool	_isColorChanged( WgMode newMode ) const;
	void	_onCloneContent( const WgWidget * _pOrg ) override;
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window ) override;
	bool	_onAlphaTest( const WgCoord& ofs ) override;
	void	_onEnable() override;
	void	_onDisable() override;
	void	_setScale(int scale) override;

private:

	WgColorsetPtr	m_pColors;
	WgSize			m_defaultSize;
	WgMode			m_mode;

	float			m_skinValue = 1.f;			// Range 0.f -> 1.f. Used for meter-skins.

};


#endif //WG2_FILL_DOT_H
