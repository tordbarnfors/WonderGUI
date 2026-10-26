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
#include <wg2_radiobutton.h>
#include <wg2_panel.h>
#include <wg2_eventhandler.h>

static const char	c_widgetType[] = {"RadioButton"};



//____ Constructor _________________________________________________________________

WgRadioButton::WgRadioButton()
{
	m_bAllowUnchecking	= false;
}

//____ Destructor _____________________________________________________________

WgRadioButton::~WgRadioButton()
{
}


//____ Type() _________________________________________________________________

const char * WgRadioButton::Type( void ) const
{
	return GetClass();
}

const char * WgRadioButton::GetClass( void )
{
	return c_widgetType;
}


//____ SetState() _____________________________________________________________

bool WgRadioButton::SetState( bool _state )
{
	if( m_bChecked != _state )
	{
		if( _state )
		{
			WgWidgetHolder * pGroup = _findRadioGroup();
			if( pGroup )
				_unselectRecursively( pGroup );

			// Set and queue event

			m_bChecked = true;
			_queueEvent( new WgEvent::RadiobuttonSelect(this) );
		}
		else
		{
			if( !m_bAllowUnchecking )
				return false;

			m_bChecked = false;
			_queueEvent( new WgEvent::RadiobuttonUnselect(this) );
		}

		_queueEvent( new WgEvent::RadiobuttonToggle(this, m_bChecked) );
		_requestRender();
	}
	return true;
}

//____ _findRadioGroup() _______________________________________________________

WgWidgetHolder * WgRadioButton::_findRadioGroup()
{
	WgWidgetHolder * pRadioGroup = 0;
	WgWidgetHolder * pParent = Parent();
	while( pParent )
	{
		pRadioGroup = pParent;

		if( pParent->IsContainer() )
		{
			if( pParent->IsPanel() && pParent->CastToPanel()->IsRadioGroup() )
				break;

			pParent = pParent->CastToContainer()->Parent();
		}
		else break;
	}

	return pRadioGroup;
}

//____ _unselectRecursively() __________________________________________________

void WgRadioButton::_unselectRecursively( WgWidgetHolder * pParent )
{
	WgWidget * pWidget = pParent->FirstWidget();

	while( pWidget )
	{
		if( pWidget->Type() == WgRadioButton::GetClass() )
		{
			WgRadioButton * pRB = (WgRadioButton*) pWidget;
			if( pRB->m_bChecked )
			{
				pRB->m_bChecked = false;

				WgEventHandler * pHandler = _eventHandler();
				if( pHandler )
				{
					pHandler->QueueEvent( new WgEvent::RadiobuttonUnselect(pRB) );
					pHandler->QueueEvent( new WgEvent::RadiobuttonToggle(pRB, false) );
				}
				pRB->_requestRender();
			}
		}
		else if( pWidget->IsContainer() && (!pWidget->IsPanel() || !pWidget->CastToPanel()->IsRadioGroup()) )
			_unselectRecursively( pWidget->CastToContainer() );

		pWidget = pWidget->NextSibling();
	}
}

//____ _onCloneContent() _______________________________________________________

void WgRadioButton::_onCloneContent( const WgWidget * _pOrg )
{
	WgCheckBox::_onCloneContent( _pOrg );

	WgRadioButton * pOrg = (WgRadioButton *) _pOrg;

	m_bAllowUnchecking	= pOrg->m_bAllowUnchecking;
}
