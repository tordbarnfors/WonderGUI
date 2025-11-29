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
#include <vector>

#ifndef WG2_SIZECAPSULE_DOT_H
#define WG2_SIZECAPSULE_DOT_H

#ifndef WG2_CAPSULE_DOT_H
#	include <wg2_capsule.h>
#endif

class WgSizeCapsule : public WgCapsule
{
public:
	WgSizeCapsule();
	virtual ~WgSizeCapsule();

	virtual const char *Type( void ) const override;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const override { return new WgSizeCapsule(); };

	void	SetSizes( WgSize min, WgSize preferred, WgSize max );   // Measured in points

	void	SetPreferredSize( WgSize size );        // Measured in points
	void	SetMinSize( WgSize size );              // Measured in points
	void	SetMaxSize( WgSize size );              // Measured in points
	void	SetKeepAspectRatio(bool bKeepAspect);
    WgSize  PreferredPointSize() { return m_pointsPreferred; }; // Measured in points

    WgSize	PreferredPixelSize() const override;
	WgSize	MinPixelSize() const override;
	WgSize	MaxPixelSize() const override;

	int		MatchingPixelHeight( int width ) const override;
	int		MatchingPixelWidth( int height ) const override;

private:
	void	_setScale(int scale) override;

	WgSize          m_pixelsMin;              // Measured in pixels
	WgSize          m_pixelsMax;              // Measured in pixels
	WgSize          m_pixelsPreferred = {-1, -1};        // Measured in pixels

	WgSize			m_pointsMin;
	WgSize			m_pointsMax;
	WgSize			m_pointsPreferred = {-1, -1};

	bool			m_bKeepAspect = false;
};

#endif //WG2_SIZECAPSULE_DOT_H
