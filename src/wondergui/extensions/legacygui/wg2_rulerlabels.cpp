
#include <wg2_rulerlabels.h>
#include <wg2_gfxdevice.h>

static const char	c_widgetType[] = {"RulerLabels"};


//____ Constructor ____________________________________________________________

WgRulerLabels::WgRulerLabels()
{
	m_direction = WgDirection::Right;
}

//____ Destructor _____________________________________________________________

WgRulerLabels::~WgRulerLabels()
{
}

//____ Type() _________________________________________________________________

const char * WgRulerLabels::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgRulerLabels::GetClass()
{
	return c_widgetType;
}

//____ AddLabel() ____________________________________________________________

void WgRulerLabels::AddLabel( const wg::CharSeq& text, wg::TextStyle * pStyle, float offset, WgOrigo origo )
{
	Label * pLabel = new Label();
	pLabel->text.setText(text);
	pLabel->text.setStyle(pStyle);
	pLabel->text.setAlignment(origo);
	pLabel->offset = offset;
	pLabel->text.SetAutoEllipsis(false);

	if( m_labels.isEmpty() )
		pLabel->text.setHolder(this);

	m_labels.pushBack(pLabel);
	_requestResize();
	_requestRender();
}

//____ SetDirection() __________________________________________________________

void WgRulerLabels::SetDirection( WgDirection direction )
{
	if( direction == WgDirection::Undefined )
		return;

	m_direction = direction;
	_requestResize();
	_requestRender();
}

//____ PreferredPixelSize() ________________________________________________________________

WgSize WgRulerLabels::PreferredPixelSize() const
{
	//TODO: calculation of length is not good.

	WgSize preferred;

	if( m_direction == WgDirection::Up || m_direction == WgDirection::Down )
	{
		Label * pLabel = m_labels.first();
		while( pLabel )
		{
			int w = pLabel->text.unwrappedWidth();
			if( w > preferred.w )
				preferred.w = w;

			preferred.h += pLabel->text.getLine(0)->lineSpacing;
			pLabel = pLabel->next();
		}
	}
	else
	{
		Label * pLabel = m_labels.first();
		while( pLabel )
		{
			WgSize sz = pLabel->text.unwrappedSize();
			preferred.w += sz.w;

			if( sz.h > preferred.h )
				preferred.h = sz.h;

			pLabel = pLabel->next();
		}
	}

	return preferred;
}


//____ _onRender() _____________________________________________________________________

void WgRulerLabels::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	if( m_direction == WgDirection::Up || m_direction == WgDirection::Down )
	{
		Label * pLabel = m_labels.first();
		while( pLabel )
		{
			int height = pLabel->text.height();
			int ofs = (int) (_canvas.h * pLabel->offset);
			if( m_direction == WgDirection::Up )
				ofs = _canvas.h - ofs;

			switch( pLabel->text.alignment() )
			{
				case WgOrigo::Undefined:
				case WgOrigo::NorthWest:
				case WgOrigo::North:
				case WgOrigo::NorthEast:
					break;
				case WgOrigo::SouthEast:
				case WgOrigo::South:
				case WgOrigo::SouthWest:
					ofs -= height;
					break;
				case WgOrigo::East:
				case WgOrigo::West:
				case WgOrigo::Center:
					ofs -= height/2;
					break;
			}

			WgGfxDevice::PrintText( pDevice, &pLabel->text, WgRect( _canvas.x, _canvas.y + ofs, _canvas.w, height ) );
			pLabel = pLabel->next();
		}
	}
	else
	{
		Label * pLabel = m_labels.first();
		while( pLabel )
		{
			int width = pLabel->text.width();
			int ofs = (int) (_canvas.w * pLabel->offset);
			if( m_direction == WgDirection::Left )
				ofs = _canvas.w - ofs;

			switch( pLabel->text.alignment() )
			{
				case WgOrigo::Undefined:
				case WgOrigo::NorthWest:
				case WgOrigo::SouthWest:
				case WgOrigo::West:
					break;
				case WgOrigo::SouthEast:
				case WgOrigo::NorthEast:
				case WgOrigo::East:
					ofs -= width;
					break;
				case WgOrigo::North:
				case WgOrigo::Center:
				case WgOrigo::South:
					ofs -= width/2;
					break;
			}

			WgGfxDevice::PrintText( pDevice, &pLabel->text, WgRect( _canvas.x + ofs, _canvas.y, width, _canvas.h ) );
			pLabel = pLabel->next();
		}
	}

}

//____ _onCloneContent() _________________________________________________________________

void WgRulerLabels::_onCloneContent( const WgWidget * _pOrg )
{
	const WgRulerLabels * pOrg = static_cast<const WgRulerLabels*>(_pOrg);
}

//____ _onAlphaTest() ____________________________________________________________________

bool WgRulerLabels::_onAlphaTest( const WgCoord& ofs )
{
	return false;
}

//____ _setScale() _____________________________________________________________

void WgRulerLabels::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	Label * pLabel = m_labels.first();
	while( pLabel )
	{
		pLabel->text.SetScale(scale);
		pLabel = pLabel->next();
	}
	_requestResize();
}


//____ _onTextModified() _________________________________________________________________

void WgRulerLabels::_textModified( WgText * pText )
{
	_requestResize();
	_requestRender();
}

