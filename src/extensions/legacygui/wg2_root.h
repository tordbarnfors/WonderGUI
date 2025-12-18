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
#ifndef WG2_ROOT_DOT_H
#define WG2_ROOT_DOT_H

#ifndef WG2_CONTAINER_DOT_H
#	include <wg2_container.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif


class WgWidget;
class WgEventHandler;

class WgRoot
{
	friend class WgWidget;
	
public:
	virtual ~WgRoot(){}
	
	virtual WgEventHandler *	EventHandler() const = 0;

	virtual int			Scale() const = 0;
	virtual WgWidget *	FindWidget( const WgCoord& ofs, WgSearchMode mode ) = 0;
	virtual bool		SetChild(WgWidget* pWidget) = 0;


protected:
	virtual void        _addPreRenderCall(WgWidget * pWidget) = 0;

};



#endif //WG2_ROOT_DOT_H
