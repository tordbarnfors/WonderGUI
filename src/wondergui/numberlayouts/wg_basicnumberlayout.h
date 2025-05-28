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
#ifndef	WG_BASICNUMBERLAYOUT_DOT_H
#define WG_BASICNUMBERLAYOUT_DOT_H
#pragma once

#include <wg_numberlayout.h>
#include <wg_textstyle.h>

#include <vector>

namespace wg
{


	class BasicNumberLayout;
	typedef	StrongPtr<BasicNumberLayout>	BasicNumberLayout_p;
	typedef	WeakPtr<BasicNumberLayout>		BasicNumberLayout_wp;

	//____ BasicNumberLayout __________________________________________________________

	class BasicNumberLayout : public NumberLayout
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			int			base = 10;						///< Numeric base used. Any value between 2 and 16 is accepted. Default is 10 (normal decimal values).

			int			decimalMax = 6;					///< Maximum number of decimals to display. Last digit is rounded. Default is 6.
			int			decimalMin = 0;					///< Minimum number of decimals to display. Default is 0.
			uint16_t	decimalSign = '.';				///< Character to use for decimal sign. Default is a dot/period character.

			int			integerGrouping = 3;			///< Number of integers displayed before an integerGroupSeparator is inserted. Default is 3.
			uint16_t	integerGroupSeparator = ' ';	///< Character used to separate groups of integers. Default is whitespace.
			int			integerMin = 1;					///< Minimum number of integers displayed. Default is 1. Valid range is 0-32.

			Placement	placement = Placement::East;	///< Placement of displayed value. Default is East, i.e. right aligned and vertically centered.

			String		prefix;							///< A prefix to be displayed left of the value e.g. '$' or '£'. Negative values display the minus sign before this prefix unless prefixNegative is set. 
			String		prefixNegative;					///< A separate prefix to display for negative values only, should include the minus sign.

			TextStyle_p	style;							///< TextStyle for the number, including decimal sign, group separator and (if prefixNegative not set) the minus sign.
			TextStyle_p	styleDecimals;					///< A separate TextStyle, overriding .style for the decimal sign and decimals of the value. 
			TextStyle_p	styleNegative;					///< A separate TextStyle to use for negative numbers. Overrides .style and .styleDecimals.
			TextStyle_p	styleNegativeDecimals;			///< A separate TextStyle to use for decimal sign and decimals of negative values, overrides .style and .styleNegative.

			String		suffix;							///< A suffix to be displayed right of the value e.g. " cm" or " km". 
			String		suffixNegative;					///< A separate suffix to be displayed for negative values only.
		};

		//.____ Creation __________________________________________

		static BasicNumberLayout_p	create( const Blueprint& blueprint ) { return BasicNumberLayout_p(new BasicNumberLayout( blueprint )); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc __________________________________________________

		void 	render(GfxDevice* pDevice, const RectSPX& canvas, double value, int scale, State state) override;
		SizeSPX	defaultSize(double value, int scale, State state) const override;
		bool	stateChangeNeedsRender(State newState, State oldState) const override;

		Blueprint blueprint() const;

	protected:
		BasicNumberLayout( const Blueprint& descriptor );
		~BasicNumberLayout();

		CharBuffer	_valueToStr(double value ) const;
		std::tuple<SizeSPX, spx> _strSizeBaseline(const Char * pChars, int scale, State state) const;
		void		_addStylesFromString(std::vector<TextStyle*>& stylesVector, const String& string) const;


		Blueprint	m_blueprint;

		bool		m_bStatesIdentical[State::NbStates][State::NbStates];
		bool		m_bHadNegativePrefix = false;
	};


} // namespace wg
#endif //WG_BASICNUMBERLAYOUT_DOT_H
