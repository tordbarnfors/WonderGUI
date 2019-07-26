
#include <wg_rulerlabels.h>
#include <wg_gfxdevice.h>

namespace wg
{

	const char RulerLabels::CLASSNAME[] = {"RulerLabels"};


	//____ Constructor ____________________________________________________________

	RulerLabels::RulerLabels()
	{
		m_direction = Direction::Right;
	}

	//____ Destructor _____________________________________________________________

	RulerLabels::~RulerLabels()
	{
	}

	//____ isInstanceOf() _________________________________________________________

	bool RulerLabels::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;

		return Widget::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * RulerLabels::className( void ) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	RulerLabels_p RulerLabels::cast( Object * pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return RulerLabels_p( static_cast<RulerLabels*>(pObject) );

		return 0;
	}


	//____ addLabel() ____________________________________________________________

	void RulerLabels::addLabel( const CharSeq& text, TextStyle * pStyle, float offset )
	{
		Label * pLabel = new Label(this);
		pLabel->text.set(text);
		pLabel->text.setStyle(pStyle);
		pLabel->offset = offset;

		m_labels.pushBack(pLabel);
		_requestResize();
		_requestRender();
	}

	//____ setDirection() __________________________________________________________

	void RulerLabels::setDirection( Direction direction )
	{
		m_direction = direction;
		_requestResize();
		_requestRender();
	}

	//____ getLabel() ________________________________________________________________

	ITextDisplay_p	RulerLabels::getLabel(int index)
	{
		if( index >= m_labels.size() )
			return ITextDisplay_p();

		return ITextDisplay_p(&m_labels.get(index)->interface);
	}



	//____ _preferredSize() ________________________________________________________________

	SizeI RulerLabels::_preferredSize() const
	{
		//TODO: calculation of length is not good.

		SizeI preferred;

		if( m_direction == Direction::Up || m_direction == Direction::Down )
		{
			Label * pLabel = m_labels.first();
			while( pLabel )
			{
				SizeI sz = pLabel->text.preferredSize();
				if( sz.w > preferred.w )
					preferred.w = sz.w;

				preferred.h += sz.h;
				pLabel = pLabel->next();
			}
		}
		else
		{
			Label * pLabel = m_labels.first();
			while( pLabel )
			{
				SizeI sz = pLabel->text.preferredSize();
				preferred.w += sz.w;

				if( sz.h > preferred.h )
					preferred.h = sz.h;

				pLabel = pLabel->next();
			}
		}

		if( m_pSkin )
			return m_pSkin->_sizeForContent(preferred);
		else
			return preferred;
	}


	//____ _render() _____________________________________________________________________

	void RulerLabels::_render( GfxDevice * pDevice, const RectI& _canvas, const RectI& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		RectI canvas;
		if( m_pSkin )
			canvas = m_pSkin->_contentRect(_canvas,m_state);
		else
			canvas = _canvas;

		if( m_direction == Direction::Up || m_direction == Direction::Down )
		{
			Label * pLabel = m_labels.first();
			while( pLabel )
			{
				int height = pLabel->text.size().h;
				int ofs = (int) (canvas.h * pLabel->offset);
				if( m_direction == Direction::Up )
					ofs = canvas.h - ofs;
	/*
	TODO: Reinstate!!!

				switch( pLabel->text.alignment() )
				{
					case Origo::NorthWest:
					case Origo::North:
					case Origo::NorthEast:
						break;
					case Origo::SouthEast:
					case Origo::South:
					case Origo::SouthWest:
						ofs -= height;
						break;
					case Origo::East:
					case Origo::West:
					case Origo::Center:
						ofs -= height/2;
						break;
				}
	*/
				pLabel->text.render(pDevice, RectI( canvas.x, canvas.y + ofs, canvas.w, height ) );
				pLabel = pLabel->next();
			}
		}
		else
		{
			Label * pLabel = m_labels.first();
			while( pLabel )
			{
				int width = pLabel->text.size().w;
				int ofs = (int) (canvas.w * pLabel->offset);
				if( m_direction == Direction::Left )
					ofs = canvas.w - ofs;
	/*
	TODO: Reinstate!

				switch( pLabel->text.alignment() )
				{
					case Origo::NorthWest:
					case Origo::SouthWest:
					case Origo::West:
						break;
					case Origo::SouthEast:
					case Origo::NorthEast:
					case Origo::East:
						ofs -= width;
						break;
					case Origo::North:
					case Origo::Center:
					case Origo::South:
						ofs -= width/2;
						break;
				}
	*/
				pLabel->text.render( pDevice, RectI( canvas.x + ofs, canvas.y, width, canvas.h ) );
				pLabel = pLabel->next();
			}
		}

	}

	//____ _cloneContent() _________________________________________________________________

	void RulerLabels::_cloneContent( const Widget * _pOrg )
	{
		Widget::_cloneContent( _pOrg );
	}

	//____ _alphaTest() ____________________________________________________________________

	bool RulerLabels::_alphaTest( const CoordI& ofs )
	{
		return Widget::_alphaTest(ofs);
	}


	//____ _setState() ______________________________________________________

	void RulerLabels::_setState( State state )
	{
		Widget::_setState(state);

		Label * p = m_labels.first();
		while( p )
		{
			p->text.setState(state);
			p = p->next();
		}
	}


	//____ _componentPos() __________________________________________________________

	CoordI RulerLabels::_componentPos( const Component * pComponent ) const
	{
		//TODO: Implement!!!
		return CoordI();
	}

	//____ _componentSize() _________________________________________________________

	SizeI RulerLabels::_componentSize( const Component * pComponent ) const
	{
		//TODO: Implement!!!
		return SizeI();
	}

	//____ _componentGeo() __________________________________________________________

	RectI RulerLabels::_componentGeo( const Component * pComponent ) const
	{
		//TODO: Implement!!!
		return RectI();
	}

	//____ _componentRequestResize() __________________________________________________

	void RulerLabels::_componentRequestResize( const Component * pComponent )
	{
		//TODO: Implement!!!
		_requestResize();
	}

} // namespace wg
