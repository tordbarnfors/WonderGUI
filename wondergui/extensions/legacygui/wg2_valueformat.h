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
#ifndef	WG2_VALUEFORMAT_DOT_H
#define WG2_VALUEFORMAT_DOT_H

#ifndef	WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#include <wg_textstyle.h>

#include <wg_charseq.h>

#include <wg_string.h>


//____ WgValueFormat __________________________________________________________

class WgValueFormat
{
	public:
		WgValueFormat();
		WgValueFormat( const wg::CharSeq& format );
		WgValueFormat( const WgValueFormat& in );
		WgValueFormat(	int nInt, int nDec, int grouping = 0, bool bPlus = false,
						Uint16 _separator = 0xA0 /*0xA0=NO_BREAK_SPACE*/, Uint16 period = 0x2e, bool bForcePeriod = false, const char * pPrefix = 0, const char * pSuffix = 0 );

		void setFormat( const wg::CharSeq& format );

		void setFormat( int nInt, int nDec, int grouping, bool bPlus = false,
						Uint16 _separator = 0, Uint16 period = 0, bool bForcePeriod = false );

		void setPrefix( const wg::String& str );
		inline const wg::String& getPrefix() const { return prefix; }

		void setSuffix( const wg::String& str );
		inline const wg::String& getSuffix() const { return suffix; }

		inline void setTextStyle( wg::TextStyle * _pStyle ) { pTextStyle = _pStyle; bSetTextStyle = true; }
		inline void clearTextStyle() { bSetTextStyle = false; }
		inline wg::TextStyle_p getTextStyle() const { return pTextStyle; }

		inline void setNoDecimalThreshold( int maxValue ) { noDecimalThreshold = maxValue; }
		inline void clearNoDecimalThreshold() { noDecimalThreshold = 0; }
		inline int  getNoDecimalThreshold() const { return noDecimalThreshold; }

		inline void setForceDecimals( bool bForce ) { bForceDecimals = bForce; }
		inline bool	getForceDecimals() const { return bForceDecimals; }

		inline void setZeroIsNegative( bool bNegative ) { bZeroIsNegative = bNegative; }
		inline bool	getZeroIsNegative() const { return bZeroIsNegative; }

		inline void		setPeriod(Uint16 _period) { period = _period; }
		inline Uint16	getPeriod() const { return period; }

		inline void		setForcePeriod(bool _bForcePeriod) { bForcePeriod = _bForcePeriod; }
		inline bool		getForcePeriod() const { return bForcePeriod; }

		inline void		setIntegers(int nInt) { integers = nInt; }
		inline Uint8	getIntegers() const { return integers; }

		inline void		setDecimals(int nDec) { decimals= nDec; }
		inline Uint8	getDecimals() const { return decimals; }

		inline void		setGrouping(int _grouping) { grouping = _grouping; }
		inline Uint8	getGrouping() const { return grouping; }

		inline void		setPlus(bool _bPlus) { bPlus = _bPlus; }
		inline bool		getPlus() const { return bPlus; }

		inline void		setSeparator(Uint16 _separator) { separator = _separator; }
		inline Uint16	getSeparator() const { return separator; }


		Uint8		integers;			/// Lowest number of integers to display, padded with zeroes.
		Uint8		decimals;			/// Number of decimals to display.
		Uint8		grouping;			/// Number of integers to group together. 0 = no grouping
		Uint16		separator;			/// Character to separate integer groups with.
		Uint16		period;				/// Character used for separating integer and decimal part
		wg::String	prefix;				/// Characters preceding the value, like $, £ or similar.
		wg::String	suffix;				/// Characters following the value.
		bool		bPlus;				/// Set if a plus sign should be preceding positive value.
		bool		bZeroIsNegative;	/// Set if zero value should be deemed negative and be preceeded by minus.
		bool		bForcePeriod;		/// Display period even if there are no decimals?
	wg::TextStyle_p pTextStyle;	/// Text properties for generated text.
		bool		bSetTextStyle;		/// Set if properties should be set for text.
		bool		bForceDecimals;		/// Display decimals even if they are 0
		int			noDecimalThreshold;	/// If the value is >= this, no decimals will be displayed.
		int			scale;				/// Value is scaled by this value before being formatted.
};

//____ WgValueFormatter _______________________________________________________

/*
	Ideas:

			* Use textprops on format-string.
						(props for minus determines props for negative values,
						(props for leading '1' determines props for positive values,
						 props for first decimal determines props for decimals.

						 Prop override order for number portion:
							base from '1'.
							override with decimalprops (if decimals).
							override with minusprop (if negative value).

						Prop override order for prefix/subfix:
							base from individual prefix/subfix characters.
							override with minusprop (if negative value).
						 )

			* Position of minus-sign (before or after prefix).
			* Support hex-values (choose upper/lower).
			* Setting for round off or truncate decimals.

			* Skip bForcePeriod, bZeroIsNegative, bSetTextprop and pTextProperties.
			* Replace noDecimalTreshold with "value digits"?
*/


class WgValueFormatter
{
public:
	WgValueFormatter();
	WgValueFormatter( const wg::CharSeq& format );
	~WgValueFormatter();

	void SetFormat( const wg::CharSeq& format );

	wg::String Prefix() const;
	wg::String Suffix() const;
	int		 Decimals() const { return m_format.decimals; }

	wg::String Format( Sint64 value ) const;
	wg::String FormatNoPreSuffix( Sint64 value ) const;

private:

	void	_formatValue( wg::CharBuffer * pBuffer, Sint64 value ) const;

	WgValueFormat	m_format;
};



#endif
