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
#include "testwidget.h"
#include <wg_gfxdevice.h>
#include <wg_msgrouter.h>
#include <wg_base.h>
#include <wg_msg.h>

namespace wg
{
	const TypeInfo TestWidget::TYPEINFO = { "TestWidget", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	TestWidget::TestWidget()
	{
		m_bOpaque = false;
		m_bPointsInitialized = false;
		m_bAnimated = false;
	}

	//____ Destructor _____________________________________________________________

	TestWidget::~TestWidget()
	{
		
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TestWidget::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ preferredSize() __________________________________________________________

	Size TestWidget::preferredSize() const
	{
		return Size(200,200);
	}

	//____ start() _________________________________________________________________

	void TestWidget::start()
	{
		_startReceiveUpdates();
	}

	//____ stop() __________________________________________________________________

	void TestWidget::stop()
	{
		_stopReceiveUpdates();
	}

	//____ _cloneContent() _______________________________________________________

	void TestWidget::_cloneContent( const Widget * _pOrg )
	{
	}

	//____ _render() _____________________________________________________________

	void TestWidget::_render( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window )
	{
 //       pDevice->setBlendMode(BlendMode::Invert);
        
        

		if( m_bPointsInitialized )
		{
		
			pDevice->drawLine( _canvas.pos().px(), Coord(_canvas.x + _canvas.w, _canvas.y + _canvas.h ).px(), HiColor(4096,000,000), 10.f );

			for( int i = 0 ; i < 2 ; i+=2 )
			{
				pDevice->drawLine( _canvas.pos() + m_coord[i], _canvas.pos() + m_coord[i+1], HiColor(000,4096,000), 15.5f );
			}
		}


 //       pDevice->setBlendMode(BlendMode::Blend);
        
	}

	//____ _update() ______________________________________________________________

	void TestWidget::_update(int microPassed, int64_t microsecTimestamp)
	{				
		RectI area( 0,0, m_size );			
				
		if( !m_bPointsInitialized )
		{
			for( int i = 0 ; i < 2 ; i+=2 )
			{
				CoordI& c = m_coord[i];
						
				c.x = 0;
				c.y = 0;

				c = m_coord[i+1];

				c.x = 100000;
				c.y = 100000;
						
			}
			m_bPointsInitialized = true;
		}
				
				
		for( int i = 0 ; i < 2 ; i++ )
		{
			CoordI& c = m_coord[i];
					
			if( c.x < area.x )
				c.x = area.x;
						
			if( c.y < area.y )
				c.y = area.y;

			if( c.x > area.x + area.w )
				c.x = area.x + area.w;

			if( c.y > area.y + area.h )
				c.y = area.y + area.h;					

					
					
			if( (c.y == area.y) && (c.x < area.x + area.w) )
				c.x++;
			else if( (c.x == area.x + area.w) && (c.y < area.y + area.h) )
				c.y++;
			else if( (c.y == area.y + area.h) && (c.x > area.x) )
				c.x--;
			else
				c.y--;
		}
				
		_requestRender();
	}

} // namespace wg
