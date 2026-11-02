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
#include <wg_basicnumberlayout.h>
#include <wg_base.h>
#include <wg_util.h>
#include <wax_utf8.h>

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

		if (m_blueprint.prefixNegative.empty())
			m_blueprint.prefixNegative = "-" + m_blueprint.prefix;
		else
			m_bHadNegativePrefix = true;

		if (m_blueprint.suffixNegative.empty())
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

		if (blueprint.prefixStyle)
			styles.push_back( blueprint.prefixStyle );

		if (blueprint.suffixStyle)
			styles.push_back( blueprint.suffixStyle );

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

		auto segments =_valueToSegments(value);

		// Place string within canvas.

		CoordSPX ofs = canvas.pos();

		SizeSPX size;
		spx		baseline;
		std::tie(size, baseline) = _segmentsSizeBaseline(segments, Base::defaultStyle(), scale, state);
		ofs += Util::placementToRect(m_blueprint.placement, canvas.size(), size).pos();
		ofs.y += baseline;

		// Render characters

		ClipPopData popData;
		if (size.w > canvas.w || size.h > canvas.h)
			popData = limitClipList(pDevice, canvas);

		_renderSegments(pDevice, segments, Base::defaultStyle(), ofs, scale, state);

		popClipList(pDevice, popData);
	}

	//____ defaultSize() ____________________________________________________

	SizeSPX BasicNumberLayout::defaultSize(double value, int scale, State state) const
	{
		auto segments = _valueToSegments(value);
		SizeSPX size;
		spx		baseline;
		std::tie(size, baseline) = _segmentsSizeBaseline(segments, Base::defaultStyle(), scale, state);
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

	//____ _valueToSegments() ______________________________________________________

	std::vector<NumberLayout::Segment> BasicNumberLayout::_valueToSegments(double value) const
	{
		const static char	codeTab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

		bool bNegative = false;
		
		if (value < 0.0)
		{
			value = -value;
			bNegative = true;
		}

		int64_t	intpart = value;
		int64_t fracpart = (value - intpart) * pow(m_blueprint.base, m_blueprint.decimalMax) + 0.5f;

		// Write decimal sign and fractional part

		std::string decimals;

		int nbDecimals = 0;
		int divider = pow(m_blueprint.base, m_blueprint.decimalMax-1);

		while ((fracpart > 0 || nbDecimals < m_blueprint.decimalMin) && nbDecimals < m_blueprint.decimalMax)
		{
			int printval = int(fracpart / divider);
			fracpart -= int64_t(printval) * divider;
			divider /= m_blueprint.base;

			decimals.push_back( codeTab[printval] );
			nbDecimals++;
		}

		if( nbDecimals > 0 )
			decimals.insert( 0, Utf8::fromCodepoint(m_blueprint.decimalSign) );

		// Write integer part backwards, acknowledge grouping

		std::u32string integers;

		int nbIntegers = 0;
		int groupingCounter = 0;

		while (intpart > 0 || nbIntegers < m_blueprint.integerMin)
		{
			if (groupingCounter == m_blueprint.integerGrouping)
			{
				integers.push_back(m_blueprint.integerGroupSeparator);
				groupingCounter = 0;
			}

			int printval = intpart % m_blueprint.base;
			intpart /= m_blueprint.base;

			integers.push_back(codeTab[printval]);
			groupingCounter++;
			nbIntegers++;
		}

		std::reverse( integers.begin(), integers.end() );

		// Put the segments together

		std::vector<Segment> segments;

		const std::string& prefix = bNegative ? m_blueprint.prefixNegative : m_blueprint.prefix;
		const std::string& suffix = bNegative ? m_blueprint.suffixNegative : m_blueprint.suffix;

		if( !prefix.empty() )
			segments.push_back( { prefix, m_blueprint.prefixStyle } );

		segments.push_back( { Utf8::fromUtf32(integers), bNegative ? m_blueprint.styleNegative : m_blueprint.style } );

		if( !decimals.empty() )
			segments.push_back( { decimals, bNegative ? m_blueprint.styleNegativeDecimals : m_blueprint.styleDecimals } );

		if( !suffix.empty() )
			segments.push_back( { suffix, m_blueprint.suffixStyle } );

		return segments;
	}

} // namespace wg
