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

#include <wg_basicnumberlayout.h>
#include <wg_base.h>
#include <wg_util.h>

#include <algorithm>
#include <cmath>

namespace wg
{

	const TypeInfo BasicNumberLayout::TYPEINFO = { "BasicNumberLayout", &NumberLayout::TYPEINFO };

	using namespace Util;

	//____ constructor ________________________________________________________

	BasicNumberLayout::BasicNumberLayout( const Blueprint& blueprint )
	{
		m_blueprint = blueprint;

		if (m_blueprint.prefixNegative.isEmpty())
		{
			m_blueprint.prefixNegative = m_blueprint.prefix;

			Char	minus[2] = { {'-', nullptr}, {0, nullptr} };
			if (!m_blueprint.prefix.isEmpty())
				minus[0].setStyle(m_blueprint.prefix.chars()->stylePtr());
			m_blueprint.prefixNegative.prepend( minus );
		}

		if (m_blueprint.suffixNegative.isEmpty())
			m_blueprint.suffixNegative = m_blueprint.suffix;

		if (m_blueprint.style == nullptr)
		{
			// Should we insert the default style or leave that to realtime?
		}


		if (m_blueprint.styleNegativeDecimals == nullptr)
		{
			if (m_blueprint.styleNegative != nullptr)
				m_blueprint.styleNegativeDecimals = m_blueprint.styleNegative;
			else if( m_blueprint.styleDecimals != nullptr )
				m_blueprint.styleNegativeDecimals = m_blueprint.styleDecimals;
			else
				m_blueprint.styleNegativeDecimals = m_blueprint.style;
		}

		if (m_blueprint.styleNegative == nullptr)
			m_blueprint.styleNegative = m_blueprint.style;

		if (m_blueprint.styleDecimals == nullptr)
			m_blueprint.styleDecimals = m_blueprint.style;

		// Detect identical states

		std::vector<TextStyle*> styles;

		if (blueprint.style)
			styles.push_back( blueprint.style );

		if( blueprint.styleNegative )
			styles.push_back( blueprint.styleNegative );

		if (blueprint.styleDecimals)
			styles.push_back( blueprint.styleDecimals );

		if (blueprint.styleNegativeDecimals)
			styles.push_back( blueprint.styleNegativeDecimals );

		if (!blueprint.prefix.isEmpty())
			_addStylesFromString(styles, blueprint.prefix);

		if (!blueprint.prefixNegative.isEmpty())
			_addStylesFromString(styles, blueprint.prefixNegative);

		if (!blueprint.suffix.isEmpty())
			_addStylesFromString(styles, blueprint.suffix);

		if (!blueprint.suffixNegative.isEmpty())
			_addStylesFromString(styles, blueprint.suffixNegative);

		for (int state1 = 0; state1 < State::NbStates; state1++)
		{
			for (int state2 = 0; state2 < State::NbStates; state2++)
			{
				State one = (StateEnum) state1;
				State two = (StateEnum) state2;

				m_bStatesIdentical[state1][state2] = true;

				for( auto pStyle : styles )
				{
					if (!pStyle->isStateIdentical(one, two))
					{
						m_bStatesIdentical[state1][state2] = false;
						break;
					}
				}
			}
		}

	}

	//____ Destructor _________________________________________________________

	BasicNumberLayout::~BasicNumberLayout()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BasicNumberLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ render() ___________________________________________________________

	void BasicNumberLayout::render(GfxDevice* pDevice, const RectSPX& canvas, double value, int scale, State state)
	{
		// Generate string from value

		auto charBuffer =_valueToStr(value);

		// Place string within canvas.

		CoordSPX ofs = canvas.pos();

		SizeSPX size;
		spx		baseline;
		std::tie(size, baseline) = _strSizeBaseline(charBuffer.chars(), scale, state);
		ofs += Util::placementToRect(m_blueprint.placement, canvas.size(), size).pos();
		ofs.y += baseline;

		// Render characters

		ClipPopData popData;
		if (size.w > canvas.w || size.h > canvas.h)
			popData = limitClipList(pDevice, canvas);

		_renderString(pDevice, charBuffer.chars(), Base::defaultStyle(), ofs, scale, state);

		popClipList(pDevice, popData);
	}

	//____ defaultSize() ____________________________________________________

	SizeSPX BasicNumberLayout::defaultSize(double value, int scale, State state) const
	{
		auto charBuffer = _valueToStr(value);
		SizeSPX size;
		spx		baseline;
		std::tie(size, baseline) = _strSizeBaseline(charBuffer.chars(), scale, state);
		return size;
	}

	//____ stateChangeNeedsRender() ___________________________________________

	bool BasicNumberLayout::stateChangeNeedsRender(State newState, State oldState) const
	{
		//TODO: This might fail if we have a Base::defaultStyle() that isn't static.

		return !m_bStatesIdentical[newState][oldState];
	}

	//____ blueprint() ________________________________________________________

	BasicNumberLayout::Blueprint BasicNumberLayout::blueprint() const
	{
		Blueprint copy = m_blueprint;

		if (!m_bHadNegativePrefix)
			copy.prefixNegative.clear();

		if (copy.suffix == copy.suffixNegative)
			copy.suffixNegative.clear();

		if (copy.styleNegativeDecimals == copy.styleNegative ||
			(copy.styleNegativeDecimals == copy.styleDecimals && copy.styleNegative == copy.style))
			copy.styleNegativeDecimals = nullptr;

		if (copy.styleNegative == copy.style)
			copy.styleNegative = nullptr;

		if (copy.styleDecimals == copy.style)
			copy.styleDecimals = nullptr;

		return copy;
	}

	//____ _valueToStr() ______________________________________________________

	CharBuffer BasicNumberLayout::_valueToStr(double value) const
	{
		const static uint16_t	codeTab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

		CharBuffer	buffer;
		buffer.setUnusedCapacity(64, 64);

		bool bNegative = false;
		
		if (value < 0.0)
		{
			value = -value;
			bNegative = true;
		}

		int64_t	intpart = value;
		int64_t fracpart = (value - intpart) * pow(m_blueprint.base, m_blueprint.decimalMax) + 0.5f;

		// Write fractional part

		auto pStyleDecimals = bNegative ? m_blueprint.styleNegativeDecimals : m_blueprint.styleDecimals;

		int nbDecimals = 0;
		int divider = pow(m_blueprint.base, m_blueprint.decimalMax-1);

		while ((fracpart > 0 || nbDecimals < m_blueprint.decimalMin) && nbDecimals < m_blueprint.decimalMax)
		{
			int printval = int(fracpart / divider);
			fracpart -= int64_t(printval) * divider;
			divider /= m_blueprint.base;

			buffer.pushBack( Char(codeTab[printval], pStyleDecimals) );
			nbDecimals++;
		}

		// Possibly add a decimal sign

		if( nbDecimals > 0 )
			buffer.pushFront(Char(m_blueprint.decimalSign, pStyleDecimals));

		// Write integer part, acknowledge grouping

		auto pStyleIntegers = bNegative ? m_blueprint.styleNegative : m_blueprint.style;
		int nbIntegers = 0;
		int grouping = m_blueprint.integerGrouping > 0 ? m_blueprint.integerGrouping : -1;
		int groupingCounter = 0;

		while (intpart > 0 || nbIntegers < m_blueprint.integerMin)
		{
			if (groupingCounter == m_blueprint.integerGrouping)
			{
				buffer.pushFront(Char(m_blueprint.integerGroupSeparator, pStyleIntegers));
				groupingCounter = 0;
			}

			int printval = intpart % m_blueprint.base;
			intpart /= m_blueprint.base;

			buffer.pushFront(Char(codeTab[printval],pStyleIntegers));
			groupingCounter++;
			nbIntegers++;
		}

		// Copy prefix and suffix

		const String& prefix = bNegative ? m_blueprint.prefixNegative : m_blueprint.prefix;
		const String& suffix = bNegative ? m_blueprint.suffixNegative : m_blueprint.suffix;

		if( !prefix.isEmpty() )
			buffer.pushFront(prefix);
		if( !suffix.isEmpty() )
			buffer.pushBack(suffix);

		return buffer;
	}

	//____ _strSize() _________________________________________________________

	std::tuple<SizeSPX, spx> BasicNumberLayout::_strSizeBaseline(const Char * pChars, int scale, State state) const
	{
		SizeSPX			size;

		TextAttr		baseAttr;

		Base::defaultStyle()->exportAttr(state, &baseAttr, scale);

		TextAttr		attr;
		Font_p 			pFont;

		TextStyle_h		hCharStyle = 0xFFFF;			// Force change on first character.

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		spx maxAscend = 0;
		spx maxDescend = 0;
		spx maxDescendGap = 0;							// Including the line gap.
		spx spaceAdv = 0;
		spx width = 0;

		while (!pChars->isEndOfLine())
		{

			if (pChars->styleHandle() != hCharStyle)
			{

				spx oldFontSize = attr.size;

				attr = baseAttr;
				if (pChars->styleHandle() != 0)
					pChars->stylePtr()->addToAttr(state, &attr, scale);

				if (pFont != attr.pFont || attr.size != oldFontSize)
				{
					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning against across different fonts or fontsizes.
				}

				spx ascend = pFont->maxAscend();
				if (ascend > maxAscend)
					maxAscend = ascend;

				spx descend = pFont->maxDescend();
				if (descend > maxDescend)
					maxDescend = descend;

				spx descendGap = descend + pFont->lineGap();
				if (descendGap > maxDescendGap)
					maxDescendGap = descendGap;

				spaceAdv = pFont->whitespaceAdvance();

				hCharStyle = pChars->styleHandle();
			}

			uint16_t charCode = pChars->code();
			pFont->getGlyphWithoutBitmap(charCode, *pGlyph);

			if (pGlyph && charCode > 32 )
			{
				width += pFont->kerning(* pPrevGlyph, * pGlyph);

				width += pGlyph->advance;
			}
			else if (pChars->code() == 32)
				width += spaceAdv;

			pPrevGlyph = pGlyph;

			pChars++;
		}

		// Handle end of text

/*
			// Make sure we have space for eol caret

			if (pCaret)
			{
				SizeSPX eolCellSize(pGlyph ? pGlyph->advance() : 0, pFont->maxAscend() + pFont->maxDescend());
				spx w = pCaret->eolWidth(eolCellSize, scale);
				if (w > eolCellSize.w)
					width += w - eolCellSize.w;
			}
*/

		// Finish this line

		size.w = width;
		size.h = maxAscend + maxDescend;
//		pLines->base = maxAscend;
	

		return  std::make_tuple(Util::alignUp(size),maxAscend);
	}

	//____ _addStylesFromString() _____________________________________________

	void BasicNumberLayout::_addStylesFromString(std::vector<TextStyle*>& stylesVector, const String& string) const
	{
		TextStyle_h prevStyle = 0;
		auto pChars = string.chars();
		while (!pChars->isEndOfText())
		{
			TextStyle_h hStyle = pChars->styleHandle();
			if (hStyle != prevStyle && hStyle != 0 )
			{
				TextStyle* pStyle = pChars->stylePtr();
				if (std::find(stylesVector.begin(), stylesVector.end(), pStyle) == stylesVector.end())
					stylesVector.push_back(pStyle);
				prevStyle = hStyle;
			}
			pChars++;
		}
	}

} // namespace wg
