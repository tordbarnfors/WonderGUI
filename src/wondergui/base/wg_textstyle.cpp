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

		m_pFont = bp.font;
		m_pLink = bp.link;
		m_blendMode = bp.blendMode;
		m_backBlendMode = bp.backBlendMode;


		// Generate lists of states that affects size, color backColor and decoration.

		State	sizeStates[State::NbStates];
		pts		stateSizes[State::NbStates];

		State	colorStates[State::NbStates];
		HiColor stateColors[State::NbStates];

		State	backColorStates[State::NbStates];
		HiColor stateBackColors[State::NbStates];

		State	decorationStates[State::NbStates];
		TextDecoration stateDecorations[State::NbStates];

		int 	nbSizeStates = 1;
		int		nbColorStates = 1;
		int		nbBackColorStates = 1;
		int		nbDecorationStates = 1;

		sizeStates[0] = State::Default;
		colorStates[0] = State::Default;
		backColorStates[0] = State::Default;
		decorationStates[0] = State::Default;

		stateSizes[0] = bp.size;
		stateColors[0] = bp.color;
		stateBackColors[0] = bp.backColor;
		stateDecorations[0] = bp.decoration;

		for (auto& stateInfo : bp.states)
		{
			int index = stateInfo.state;

			if (stateInfo.data.size >= 0 )
			{
				int index = stateInfo.state == State::Default ? 0 : nbSizeStates++;
				sizeStates[index] = stateInfo.state;
				stateSizes[index] = stateInfo.data.size;
			}

			if(stateInfo.data.color != HiColor::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbColorStates++;
				colorStates[index] = stateInfo.state;
				stateColors[index] = stateInfo.data.color;
			}

			if(stateInfo.data.backColor != HiColor::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbBackColorStates++;
				backColorStates[index] = stateInfo.state;
				stateBackColors[index] = stateInfo.data.backColor;
			}

			if(stateInfo.data.decoration != TextDecoration::Undefined )
			{
				int index = stateInfo.state == State::Default ? 0 : nbDecorationStates++;
				decorationStates[index] = stateInfo.state;
				stateDecorations[index] = stateInfo.data.decoration;
			}
		}

		// Calc size of index table for surface and color, get their index masks & shifts.

		int	sizeIndexEntries;
		int	colorIndexEntries;
		int	backColorIndexEntries;
		int	decorationIndexEntries;


		std::tie(sizeIndexEntries,m_sizeIndexMask,m_sizeIndexShift) = calcStateToIndexParam(nbSizeStates, sizeStates);
		std::tie(colorIndexEntries,m_colorIndexMask,m_colorIndexShift) = calcStateToIndexParam(nbColorStates, colorStates);
		std::tie(backColorIndexEntries,m_backColorIndexMask,m_backColorIndexShift) = calcStateToIndexParam(nbBackColorStates, backColorStates);
		std::tie(decorationIndexEntries,m_decorationIndexMask,m_decorationIndexShift) = calcStateToIndexParam(nbDecorationStates, decorationStates);


		// Calculate memory needed for all state data

		int colorBytes		= sizeof(HiColor) * nbColorStates;
		int backColorBytes	= sizeof(HiColor) * nbBackColorStates;
		int sizeBytes		= sizeof(pts) * nbSizeStates;
		int decorationBytes	= sizeof(TextDecoration) * nbDecorationStates;

		int indexBytes		= sizeIndexEntries+colorIndexEntries+backColorIndexEntries+decorationIndexEntries;

		// Allocate and pupulate memory for state data

		m_pStateData = malloc(sizeBytes + colorBytes + backColorBytes + decorationBytes + indexBytes);

		auto pDest = (uint8_t*) m_pStateData;

		auto pColors = (HiColor*) pDest;
		for( int i = 0 ; i < nbColorStates ; i++ )
			pColors[i] = stateColors[i];

		m_pColors = pColors;
		pDest += colorBytes;

		auto pBackColors = (HiColor*) pDest;
		for( int i = 0 ; i < nbBackColorStates ; i++ )
			pBackColors[i] = stateBackColors[i];

		m_pBackColors = pBackColors;
		pDest += backColorBytes;

		auto pSizes = (pts*) pDest;
		for( int i = 0 ; i < nbSizeStates ; i++ )
			pSizes[i] = stateSizes[i];

		m_pSizes = pSizes;
		pDest += sizeBytes;

		auto pDecorations = (TextDecoration*) pDest;
		for( int i = 0 ; i < nbDecorationStates ; i++ )
			pDecorations[i] = stateDecorations[i];

		m_pDecorations = pDecorations;
		pDest += decorationBytes;


		m_pColorIndexTab = pDest;
		pDest += colorIndexEntries;

		m_pBackColorIndexTab = pDest;
		pDest += backColorIndexEntries;

		m_pSizeIndexTab = pDest;
		pDest += sizeIndexEntries;

		m_pDecorationIndexTab = pDest;

		generateStateToIndexTab(m_pSizeIndexTab, nbSizeStates, sizeStates);
		generateStateToIndexTab(m_pColorIndexTab, nbColorStates, colorStates);
		generateStateToIndexTab(m_pBackColorIndexTab, nbBackColorStates, backColorStates);
		generateStateToIndexTab(m_pDecorationIndexTab, nbDecorationStates, decorationStates);

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}


	//____ Destructor ______________________________________________________________

	TextStyle::~TextStyle()
	{
		TextStyleManager::_releaseHandle(m_handle);

		free(m_pStateData);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextStyle::typeInfo(void) const
	{
		return TYPEINFO;
	}




	//____ exportAttr() ____________________________________________________________

	void TextStyle::exportAttr( State state, TextAttr * pDest, int scale ) const
	{

		pDest->pFont 		= m_pFont;
		pDest->pLink 		= m_pLink;
		pDest->blendMode	= m_blendMode;
		pDest->backBlendMode = m_backBlendMode;

		pDest->size 		= ptsToSpx(_getSize(state), scale);
		pDest->color		= _getColor(state);
		pDest->backColor	= _getBackColor(state);
		pDest->decoration	= _getDecoration(state);


		if (!pDest->pFont)
			pDest->pFont = s_pDefaultStyle->m_pFont;

		if( pDest->size < 0 )
		{
			pts defSize = s_pDefaultStyle->_getSize(state);
			pDest->size = ptsToSpx((defSize >= 0 ? defSize : 12),scale);				// Default to size 12.
		}

		if( pDest->color == HiColor::Undefined )
		{
			const HiColor& defColor = s_pDefaultStyle->_getColor(state);
			pDest->color = defColor == HiColor::Undefined ? HiColor::Black : defColor;
		}

		if( pDest->decoration == TextDecoration::Undefined )
		{
			TextDecoration defDecoration = s_pDefaultStyle->_getDecoration(state);
			pDest->decoration = defDecoration == TextDecoration::Undefined ? TextDecoration::None : defDecoration;
		}

		if (pDest->backColor == HiColor::Undefined)
			pDest->color = s_pDefaultStyle->_getBackColor(state);

		if (pDest->blendMode == BlendMode::Undefined)
			pDest->blendMode = s_pDefaultStyle->m_blendMode == BlendMode::Undefined ? BlendMode::Blend : s_pDefaultStyle->m_blendMode;			// Default to Blend.

		if (pDest->backBlendMode == BlendMode::Undefined)
			pDest->backBlendMode = s_pDefaultStyle->m_backBlendMode;

	}

	//____ addToAttr() _____________________________________________________________

	void TextStyle::addToAttr( State state, TextAttr * pDest, int scale ) const
	{
		if( m_pFont )
			pDest->pFont = m_pFont;
		if( m_pLink )
			pDest->pLink = m_pLink;
		if (m_blendMode != BlendMode::Undefined)
			pDest->blendMode = m_blendMode;
		if (m_backBlendMode != BlendMode::Undefined)
			pDest->backBlendMode = m_backBlendMode;

		pts size = _getSize(state);
		if( size >= 0 )
			pDest->size	= ptsToSpx(size,scale);

		TextDecoration decoration = _getDecoration(state);
		if( decoration != TextDecoration::Undefined )
			pDest->decoration = decoration;

		HiColor color = _getColor(state);
		if( color != HiColor::Undefined )
			pDest->color = color;

		HiColor backColor = _getBackColor(state);
		if (backColor != HiColor::Undefined)
			pDest->backColor = backColor;
	}

    //____ isStateIdentical() ________________________________________________

    bool TextStyle::isStateIdentical( State state1, State state2 ) const
    {
        return ((_getSize(state1) == _getSize(state2)) &&
                (_getColor(state1) == _getColor(state2)) &&
                (_getBackColor(state1) == _getBackColor(state2)) &&
				(_getDecoration(state1) == _getDecoration(state2)));
    }

	//____ isIdentical() _____________________________________________________

	bool TextStyle::isIdentical( TextStyle * pOther )
	{
		if (m_pFont != pOther->m_pFont || m_pLink != pOther->m_pLink ||
			m_blendMode != pOther->m_blendMode || m_backBlendMode != pOther->m_backBlendMode)
			return false;

		for (int i = 0; i < State::NbStates; i++)
		{
			State state((StateEnum)i);

			if ((_getSize(state) != pOther->_getSize(state)) ||
				(_getDecoration(state) != pOther->_getDecoration(state)) ||
				(_getColor(state) != pOther->_getColor(state)) ||
				(_getBackColor(state) != pOther->_getBackColor(state)) )
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

		State i = state;

		if ((_getSize(i) != pOther->_getSize(i)) ||
			(_getDecoration(i) != pOther->_getDecoration(i)) ||
			(_getColor(i) != pOther->_getColor(i)) ||
			(_getBackColor(i) != pOther->_getBackColor(i)) )
			return false;

		return true;
	}
/*
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
*/

} // namespace wg
