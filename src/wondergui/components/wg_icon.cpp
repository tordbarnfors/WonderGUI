/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_icon.h>
#include <wg_skin.h>
#include <wg_util.h>

namespace wg
{

	using namespace Util;

	const TypeInfo	Icon::TYPEINFO = { "Icon", &Component::TYPEINFO };

	//____ constructor ____________________________________________________________

	Icon::Icon( Widget * pWidget ) : Component(pWidget), m_skin(this)
	{
		m_placement		= Placement::West;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Icon::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ set() ___________________________________________________________________

	bool Icon::set( Skin * pSkin, Placement placement, pts spacing, bool bOverlap )
	{
		m_skin.set(pSkin);
		m_placement	= placement;
		m_spacing 	= spacing;

		_requestResize();
		return true;
	}

	//____ clear() _________________________________________________________________

	void Icon::clear()
	{
		m_skin.set(nullptr);
		m_placement	= Placement::West;
		m_spacing 	= 0;

		_requestResize();
	}

	//____ _initFromBlueprint() ______________________________________________

	void Icon::_initFromBlueprint(const Blueprint& bp)
	{
		m_skin.set(bp.skin);
		m_placement = bp.placement;
		m_spacing = bp.spacing;
	}

	//____ setPlacement() ___________________________________________________

	void Icon::setPlacement( Placement placement )
	{
		if( placement != m_placement )
		{
			m_placement = placement;
			_requestRender();
		}
	}

	//____ setSpacing() _______________________________________________________

	void Icon::setSpacing( pts spacing )
	{
		if( spacing != m_spacing )
		{
			m_spacing = spacing;
			_requestResize();
		}
	}

	//____ _setSkin() ______________________________________________________________

	void Icon::setSkin( Skin * pSkin )
	{
		if( pSkin != m_skin.get() )
		{
			m_skin.set(pSkin);
			_requestResize();
		}
	}

	//____ _getIconRect() _________________________________________________________

	/*
		Gets an icon-rect for the icon excluding borders, relative to upper left corner of content area.

	*/

	RectSPX Icon::_getIconRect( const RectSPX& contentRect, int scale ) const
	{
		if( m_skin.isEmpty() )
			return RectSPX();
		else
			return _getIconRect(contentRect, m_skin.defaultSize(scale), scale);
	}

	RectSPX Icon::_getIconRect( const RectSPX& contentRect, const SizeSPX& iconSize, int scale ) const
	{
		if (iconSize.w <= 0 && iconSize.h <= 0)
			return RectSPX();
		else
			return placementToRect( m_placement, contentRect, iconSize );
	}


	//____ _getTextRect() _____________________________________________________

	RectSPX Icon::_getTextRect( const RectSPX& contentRect, const RectSPX& iconRect, int scale ) const
	{
		RectSPX textRect = contentRect;

		if( iconRect.w > 0 && iconRect.h > 0 )
		{
			spx spacing = align(ptsToSpx(m_spacing, scale));

			switch( m_placement )
			{
				case Placement::NorthWest:
				case Placement::SouthWest:
				case Placement::West:
				{
					spx diff = iconRect.x - contentRect.x + iconRect.w + spacing;
					textRect.x += diff;
					textRect.w -= diff;
					if( textRect.w < 0 )
						textRect.w = 0;
					break;
				}
				case Placement::NorthEast:
				case Placement::East:
				case Placement::SouthEast:
				{
					textRect.w = iconRect.x - contentRect.x - spacing;
					if( textRect.w < 0 )
						textRect.w = 0;
					break;
				}

				case Placement::North:
				{
					int diff = iconRect.y - contentRect.y + iconRect.h + spacing;
					textRect.y += diff;
					textRect.h -= diff;
					if( textRect.h < 0 )
						textRect.h = 0;
					break;
				}
				case Placement::South:
				{
					textRect.h = iconRect.y - contentRect.y - spacing;
					if( textRect.h < 0 )
						textRect.h = 0;
					break;
				}

				default:				// Undefined and center results in icon centered behind text.
					break;
			}
		}

		return align(textRect);
	}

	//____ _defaultSize() ________________________________________________________

	SizeSPX Icon::_defaultSize(int scale) const
	{
		return m_skin.defaultSize(scale);

	}

	SizeSPX Icon::_defaultSize(int scale, SizeSPX& textSize) const
	{
		if (m_skin.isEmpty())
			return SizeSPX();

		SizeSPX defaultSize = m_skin.defaultSize(scale);
		spx		spacing = align(ptsToSpx(m_spacing, scale));

		switch( m_placement)
		{
			case Placement::Undefined:
			case Placement::Center:
			{
				defaultSize = SizeSPX::max(defaultSize, textSize);
				break;
			}

			case Placement::North:
			case Placement::South:
			{
				defaultSize.h += textSize.h + spacing;
				defaultSize.w = std::max(defaultSize.w, textSize.w);
				break;
			}
			default:
			{
				defaultSize.w += textSize.w + spacing;
				defaultSize.h = std::max(defaultSize.h, textSize.h);
				break;
			}

		}

		return defaultSize;
	}

	//____ _textPaddingSize() _________________________________________________

	SizeSPX Icon::_textPaddingSize(int scale) const
	{
		if (m_skin.isEmpty())
			return SizeSPX();

		SizeSPX iconSize = m_skin.defaultSize(scale);
		spx		spacing = align(ptsToSpx(m_spacing, scale));

		switch (m_placement)
		{
			case Placement::Undefined:
			case Placement::Center:
				return BorderSPX();

			case Placement::North:
			case Placement::South:
				return { 0, iconSize.h + spacing };

			default:
				return { iconSize.w + spacing, 0 };
		}
	}

	//____ _skinValue() _______________________________________________________

	float Icon::_skinValue(const SkinSlot* pSlot) const
	{
		return 0.f;		// Not supported
	}

	//____ _skinValue2() _______________________________________________________

	float Icon::_skinValue2(const SkinSlot* pSlot) const
	{
		return 0.f;		// Not supported
	}

	//____ _skinState() _______________________________________________________

	State Icon::_skinState(const SkinSlot* pSlot) const
	{
		return _widget()->state();
	}

	//____ _skinSize() _______________________________________________________

	SizeSPX Icon::_skinSize(const SkinSlot* pSlot) const
	{
		return _size();
	}

	//____ _skinRequestRender() _______________________________________________________

	void Icon::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		_requestRender(rect);
	}



} // namespace wg
