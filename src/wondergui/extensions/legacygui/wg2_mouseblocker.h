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
#ifndef WG2_MOUSEBLOCKER_DOT_H
#define WG2_MOUSEBLOCKER_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_COLORSET_DOT_H
#	include <wg2_colorset.h>
#endif

#include <vector>

//____ WgMouseBlocker ____________________________________________________________

class WgMouseBlocker : public WgWidget
{
public:
	WgMouseBlocker();
	virtual ~WgMouseBlocker();

	virtual const char *Type( void ) const override;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const override { return new WgMouseBlocker(); };

	void	SetPreferredPointSize( const WgSize& size );
	WgSize	PreferredPixelSize() const override;

	void	SetHoles( int nbHoles, const WgRect * pHoles );
	

protected:

	void	_onCloneContent( const WgWidget * _pOrg ) override;
	bool	_onAlphaTest( const WgCoord& ofs ) override;
	void	_setScale(int scale) override;

private:

	std::vector<WgRect> m_holes;

	WgSize			m_defaultSize;

};


#endif //WG2_FILL_DOT_H
