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
#ifndef	WG2_ICONHOLDER_DOT_H
#define	WG2_ICONHOLDER_DOT_H

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#ifndef WG2_BLOCKSET_DOT_H
#	include <wg2_blockset.h>
#endif

class WgIconHolder
{
public:
	WgIconHolder();
	virtual ~WgIconHolder() {};

	bool			SetIconScale( float scaleFactor );
	void			SetIconOrigo( WgOrigo origo );
	void			SetIconBorders( WgBorders borders );
	void			SetIconPushingText( bool bPush );

	float			IconScale() const { return m_iconScale; }
	WgOrigo	IconOrigo() const { return m_iconOrigo; }
	WgBorders		IconBorders() const { return m_iconBorders; }
	bool			IsIconPushingText() const { return m_bIconPushText; }

protected:

	virtual void	_iconModified() = 0;

	WgRect			_getIconRect( const WgRect& contentRect, wg::Skin * pSkin, int scale ) const;
	WgRect			_getIconRect( const WgRect& contentRect, const WgSize& iconSize, int scale ) const;
	WgRect			_getTextRect( const WgRect& contentRect, const WgRect& iconRect ) const;
	WgSize			_expandTextRect(WgSize textRectSize, wg::Skin * pSkin, int scale) const;

	wg::SizeI       _skinPrefSize( wg::Skin * pSkin, int scale ) const;


	void			_onCloneContent( const WgIconHolder * _pOrg );

	WgOrigo	m_iconOrigo;
	float			m_iconScale;					// Range: 0.f -> 1.f. 0.f = Fixed size.
	bool			m_bIconPushText;
	WgBorders		m_iconBorders;



};

#endif	// WG2_ICONHOLDER_DOT_H
