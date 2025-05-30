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

#include <wg_textstyle.h>
#include <wg_textstylemanager.h>
#include <wg_base.h>

namespace wg
{
	using namespace Util;

	const TypeInfo TextStyle::TYPEINFO = { "TextStyle", &Object::TYPEINFO };

	TextStyle_p	TextStyle::s_pDefaultStyle;

	//____ Blueprint::modifyState() ___________________________________________________

	void TextStyle::Blueprint::modifyState( State _state, const TextStyle::StateData& data )
	{
		for( auto& entry : states )
		{
			if( entry.state == _state )
			{
				if( data.size != -1 )
					entry.data.size = data.size;
				if( data.color != HiColor::Undefined )
					entry.data.color = data.color;
				if( data.backColor != HiColor::Undefined )
					entry.data.backColor = data.backColor;
				if( data.decoration != TextDecoration::Undefined )
					entry.data.decoration = data.decoration;
				
				return;
			}
		}
		
		TextStyle::StateBP sbp;
		sbp.state = _state;
		sbp.data = data;
		states.push_back( sbp );
	}



    //____ create() ________________________________________________________________

    TextStyle_p TextStyle::create( const Blueprint& blueprint)
    {
        return TextStyle_p(new TextStyle(blueprint));
    }


	//____ constructor _____________________________________________________________

	TextStyle::TextStyle( const Blueprint& bp )
	{
		m_handle = TextStyleManager::_reserveHandle(this);

		for (int i = 0; i < State::NbStates; i++)
		{
			m_size[i]			= -1;
			m_color[i]			= HiColor::Black;
			m_backColor[i]		= HiColor::Transparent;
			m_decoration[i]		= TextDecoration::Undefined;
		}

		// Setting everyting for State::Default

		m_pFont = bp.font;
		m_pLink = bp.link;
		m_blendMode = bp.blendMode;
		m_backBlendMode = bp.backBlendMode;

		int idx = State::Default;

		m_size[idx] = bp.size;
		m_color[idx] = bp.color;
		m_backColor[idx] = bp.backColor;
		m_decoration[idx] = bp.decoration;

		m_sizeSetMask.setBit(idx, bp.size >= 0);
		m_colorSetMask.setBit(idx, bp.color != HiColor::Undefined);
		m_backColorSetMask.setBit(idx, bp.backColor != HiColor::Undefined);
		m_decorationSetMask.setBit(idx, bp.decoration != TextDecoration::Undefined);

		// Setting state specific parameters

		for ( auto& entry : bp.states )
		{
			idx = entry.state;

			if( entry.data.size >= 0 )
			{
				m_size[idx] = entry.data.size;
				m_sizeSetMask.setBit(idx, true);
			}

			if( entry.data.color != HiColor::Undefined )
			{
				m_color[idx] = entry.data.color;
				m_colorSetMask.setBit(idx, true );
			}
			
			if( entry.data.backColor != HiColor::Undefined )
			{
				m_backColor[idx] = entry.data.backColor;
				m_backColorSetMask.setBit(idx, true);
			}
			
			if( entry.data.decoration != TextDecoration::Undefined )
			{
				m_decoration[idx] = entry.data.decoration;
				m_decorationSetMask.setBit(idx, true);
			}
		}

		//

		_refreshSize();
		_refreshColor();
		_refreshBgColor();
		_refreshDecoration();

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}


	//____ Destructor ______________________________________________________________

	TextStyle::~TextStyle()
	{
		TextStyleManager::_releaseHandle(m_handle);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextStyle::typeInfo(void) const
	{
		return TYPEINFO;
	}




	//____ exportAttr() ____________________________________________________________

	void TextStyle::exportAttr( State state, TextAttr * pDest, int scale ) const
	{
		int idx = state;

		
		auto pDef = s_pDefaultStyle;

		pDest->pFont 		= m_pFont;
		pDest->pLink 		= m_pLink;
		pDest->blendMode	= m_blendMode;
		pDest->backBlendMode = m_backBlendMode;

		pDest->size 		= ptsToSpx(m_size[idx], scale);
		pDest->color		= m_color[idx];
		pDest->backColor	= m_backColor[idx];
		pDest->decoration	= m_decoration[idx];

		if (!pDest->pFont)
			pDest->pFont = pDef->m_pFont;

		if( pDest->size < 0 )
			pDest->size = ptsToSpx((pDef->m_size[idx] >= 0 ? pDef->m_size[idx] : 12),scale);				// Default to size 12.

		if( pDest->color == HiColor::Undefined )
			pDest->color = pDef->m_color[idx] == HiColor::Undefined ? HiColor::Black : pDef->m_color[idx];
		
		if( pDest->decoration == TextDecoration::Undefined )
			pDest->decoration = pDef->m_decoration[idx] == TextDecoration::Undefined ? TextDecoration::None : pDef->m_decoration[idx];

		if (pDest->blendMode == BlendMode::Undefined)
			pDest->blendMode = pDef->m_blendMode == BlendMode::Undefined ? BlendMode::Blend : pDef->m_blendMode;			// Default to Blend.

		if (pDest->backColor == HiColor::Undefined)
			pDest->color = pDef->m_color[idx];

		if (pDest->backBlendMode == BlendMode::Undefined)
			pDest->backBlendMode = pDef->m_backBlendMode;

	}

	//____ addToAttr() _____________________________________________________________

	void TextStyle::addToAttr( State state, TextAttr * pDest, int scale ) const
	{
		int idx = state;

		if( m_pFont )
			pDest->pFont = m_pFont;
		if( m_pLink )
			pDest->pLink = m_pLink;
		if (m_blendMode != BlendMode::Undefined)
			pDest->blendMode = m_blendMode;
		if (m_backBlendMode != BlendMode::Undefined)
			pDest->backBlendMode = m_backBlendMode;

		if( m_size[idx] >= 0 )
			pDest->size	= ptsToSpx(m_size[idx],scale);
		if( m_decoration[idx] != TextDecoration::Undefined )
			pDest->decoration = m_decoration[idx];

		if( m_color[idx] != HiColor::Undefined )
			pDest->color = m_color[idx];

		if (m_backColor[idx] != HiColor::Undefined)
			pDest->backColor = m_backColor[idx];
	}

    //____ isStateIdentical() ________________________________________________

    bool TextStyle::isStateIdentical( State state1, State state2 ) const
    {
        int idx1 = state1;
        int idx2 = state2;

        return ((m_size[idx1] == m_size[idx2]) &&
                (m_color[idx1] == m_color[idx2]) &&
                (m_backColor[idx1] == m_backColor[idx2]) &&
                (m_decoration[idx1] == m_decoration[idx2]));
    }

	//____ isIdentical() _____________________________________________________

	bool TextStyle::isIdentical( TextStyle * pOther )
	{
		if (m_pFont != pOther->m_pFont || m_pLink != pOther->m_pLink ||
			m_blendMode != pOther->m_blendMode || m_backBlendMode != pOther->m_backBlendMode)
			return false;

		for (int i = 0; i < State::NbStates; i++)
		{
			if (m_size[i] != pOther->m_size[i] ||
				m_decoration[i] != pOther->m_decoration[i] ||
				m_color[i] != pOther->m_color[i] ||
				m_backColor[i] != pOther->m_backColor[i])
				return false;
		}

		return true;
	}

	//____ isIdenticalForState() _____________________________________________

	bool TextStyle::isIdenticalForState( TextStyle * pOther, State state )
	{
		if (m_pFont != pOther->m_pFont || m_pLink != pOther->m_pLink ||
			m_blendMode != pOther->m_blendMode || m_backBlendMode != pOther->m_backBlendMode)
			return false;

		int i = state;

		if (m_size[i] != pOther->m_size[i] ||
			m_decoration[i] != pOther->m_decoration[i] ||
			m_color[i] != pOther->m_color[i] ||
			m_backColor[i] != pOther->m_backColor[i])
			return false;

		return true;
	}

	//____ blueprint() ____________________________________________________________

	TextStyle::Blueprint TextStyle::blueprint() const
	{
		Blueprint bp;

		bp.backBlendMode = m_backBlendMode;
		bp.blendMode = m_blendMode;
		bp.link = m_pLink;
		bp.font = m_pFont;
		
		int idx = State::Default;

		bp.size = m_size[idx];
		bp.backColor = m_backColor[idx];
		bp.color = m_color[idx];
		bp.decoration = m_decoration[idx];

		Bitmask<uint32_t> stateSetMask = m_sizeSetMask | m_colorSetMask | m_backColorSetMask | m_decorationSetMask;

		for (int i = 1; i < State::NbStates; i++)
		{
			if (stateSetMask.bit(i))
			{
				StateBP bps;
				bps.state = State((StateEnum) i);
				if (m_sizeSetMask.bit(i))
					bps.data.size = m_size[i];
				if (m_colorSetMask.bit(i))
					bps.data.color = m_color[i];
				if (m_backColorSetMask.bit(i))
					bps.data.backColor = m_backColor[i];
				if (m_decorationSetMask.bit(i))
					bps.data.decoration = m_decoration[i];

				bp.states.push_back(bps);
			}
		}

		return bp;
	}

	//____ _refreshSize() _____________________________________________________

	void TextStyle::_refreshSize()
	{
		Bitmask<uint32_t> mask = m_sizeSetMask;
		mask.setBit(0);
		for (int i = 1; i < State::NbStates; i++)
		{
			if (!mask.bit(i))
				m_size[i] = m_size[bestStateIndexMatch(i, mask)];
		}

		//

		auto x = m_size[0];
		for (int i = 1; i < State::NbStates; i++)
			if (m_size[i] != x)
			{
				m_bStaticSize = false;
				return;
			}

		m_bStaticSize = true;
		return;
	}

	//____ _refreshColor() _____________________________________________________

	void TextStyle::_refreshColor()
	{
		Bitmask<uint32_t> mask = m_colorSetMask;
		mask.setBit(0);

		for (int i = 1; i < State::NbStates; i++)
		{
			if (!mask.bit(i))
			{
				int idx = bestStateIndexMatch(i, mask);
				m_color[i] = m_color[idx];
			}
		}


		//

		auto x = m_color[0];
		for (int i = 1; i < State::NbStates; i++)
			if (m_color[i] != x)
			{
				m_bStaticColor = false;
				return;
			}

		m_bStaticColor = true;
		return;
	}

	//____ _refreshBgColor() _____________________________________________________

	void TextStyle::_refreshBgColor()
	{
		Bitmask<uint32_t> mask = m_backColorSetMask;
		mask.setBit(0);

		for (int i = 1; i < State::NbStates; i++)
		{
			if (!mask.bit(i))
			{
				int idx = bestStateIndexMatch(i, mask);
				m_backColor[i] = m_backColor[idx];
			}
		}

		//

		auto x = m_backColor[0];
		for (int i = 1; i < State::NbStates; i++)
			if (m_backColor[i] != x)
			{
				m_bStaticBgColor = false;
				return;
			}

		m_bStaticBgColor = true;
		return;

	}

	//____ _refreshDecoration() _____________________________________________________

	void TextStyle::_refreshDecoration()
	{
		Bitmask<uint32_t> mask = m_decorationSetMask;
		mask.setBit(0);
		for (int i = 1; i < State::NbStates; i++)
		{
			if (!mask.bit(i))
				m_decoration[i] = m_decoration[bestStateIndexMatch(i, mask)];
		}

		//

		auto x = m_decoration[0];
		for (int i = 1; i < State::NbStates; i++)
			if (m_decoration[i] != x)
			{
				m_bStaticDecoration = false;
				return;
			}

		m_bStaticDecoration = true;
		return;
	}

} // namespace wg
