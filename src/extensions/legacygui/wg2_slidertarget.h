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
#ifndef WG2_SLIDERTARGET_DOT_H
#define WG2_SLIDERTARGET_DOT_H

#ifndef WG2_SMARTPTR_DOT_H
#	include <wg2_smartptr.h>
#endif

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif


class WgSliderTarget
{
	friend class WgWidgetSlider;

protected:

	// Calls for Slider

	virtual float	_stepFwd() = 0;
	virtual float	_stepBwd() = 0;
	virtual float	_jumpFwd() = 0;
	virtual float	_jumpBwd() = 0;

	virtual float	_wheelRolled( int distance ) = 0;

	virtual float	_setPosition( float fraction ) = 0;

	virtual WgWidget*_getWidget() = 0;
	virtual float	_getSliderPosition() = 0;
	virtual float	_getSliderSize() = 0;
	
	virtual void	_sliderDragStart() = 0;
	virtual void	_sliderDragEnd() = 0;


	// Calls for Target

	void			_updateSlider( float pos, float size );

	//

	WgWidgetWeakPtr	m_pSlider;
};



#endif

