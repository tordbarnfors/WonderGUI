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
#include <cmath>

#include <wg_texttool.h>
#include <wg2_valueformat.h>
#include <wg_charseq.h>
#include <wg_char.h>
#include <wg_string.h>
#include <assert.h>

//____ Constructor ____________________________________________________________

WgValueFormatter::WgValueFormatter()
{
}

WgValueFormatter::WgValueFormatter( const wg::CharSeq& format ) : m_format(format)
{
}

//____ Destructor _____________________________________________________________

WgValueFormatter::~WgValueFormatter()
{
}

//____ SetFormat() ____________________________________________________________

void WgValueFormatter::SetFormat( const wg::CharSeq& format )
{
	m_format.setFormat(format);
}

//____ Prefix() _______________________________________________________________

wg::String WgValueFormatter::Prefix() const
{
	return m_format.prefix;
}

//____ Suffix() _______________________________________________________________

wg::String WgValueFormatter::Suffix() const
{
	return m_format.suffix;
}

//____ Format() _______________________________________________________________

wg::String WgValueFormatter::Format( Sint64 value ) const
{
	wg::CharBuffer	buff;
	buff.setUnusedCapacity( 32,32 );
	_formatValue( &buff, value );
	buff.pushFront( m_format.prefix );
	buff.pushBack( m_format.suffix );

	if( value < 0 )
		buff.pushFront( wg::Char('-') );
	else if( value >= 0 && m_format.bPlus )
		buff.pushFront( wg::Char('+') );

	return wg::String(&buff);
}

//____ FormatNoPreSuffix() ____________________________________________________

wg::String WgValueFormatter::FormatNoPreSuffix( Sint64 value ) const
{
	wg::CharBuffer	buff;
	buff.setUnusedCapacity( 32,32 );
	_formatValue( &buff, value );

	if( value < 0 )
		buff.pushFront( wg::Char('-') );
	else if( value >= 0 && m_format.bPlus )
		buff.pushFront( wg::Char('+') );

	return wg::String(&buff);
}

//____ _formatValue() _________________________________________________________

void WgValueFormatter::_formatValue( wg::CharBuffer * pBuffer, Sint64 value ) const
{
	const WgValueFormat& f = m_format;

	Sint64 absVal = value >= 0 ? value : -value;

	Sint64 intPart = absVal / m_format.scale;
	Sint64 decPart = absVal % m_format.scale;

	// Write period and decimal part

	if( f.decimals || f.bForcePeriod )
	{
		if( 0 == f.noDecimalThreshold || (int)absVal < f.noDecimalThreshold )
		{
			if( f.bForceDecimals || decPart != 0 )
			{
				pBuffer->pushBack( wg::Char(f.period) );

				for( int i = f.decimals; i > 0 ; i-- )
				{
					decPart *= 10;
					pBuffer->pushBack( wg::Char( (Uint16)(decPart/m_format.scale) + 0x30 ));
					decPart = decPart % m_format.scale;
				}
			}
		}
	}

	// Write integer part to temp area (backwards)

	Uint16	temp2[32];

	int			n = 0;

	while( intPart > 0 )
	{
		temp2[n++] = (Uint16) ((intPart % 10) + 0x30);
		intPart /= 10;
	}

	// Copy integer part acknowledge grouping

	if( f.grouping == 0 )
	{
		for( int i = 0 ; i < n ; i++ )
			pBuffer->pushFront( wg::Char(temp2[i]) );
	}
	else
	{
		for( int i = 0 ; i < n ; i++ )
		{
			if( i != 0 && (i % f.grouping) == 0 )
				pBuffer->pushFront( wg::Char(f.separator) );

			pBuffer->pushFront( wg::Char(temp2[i]) );
		}
	}

	// Possibly fill out with zeroes, acknowledge grouping

	if( n < f.integers )
	{
		for( int i = n ; i < f.integers ; i++ )
		{
			if( i != 0 && (i % f.grouping) == 0 )
				pBuffer->pushFront( wg::Char(f.separator) );

			pBuffer->pushFront( wg::Char(0x30) );
		}
	}
}





//____ WgValueFormat() ________________________________________________________

WgValueFormat::WgValueFormat()
{
	integers		= 1;
	decimals		= 0;
	grouping		= 0;
	separator		= 0xA0 /*0xA0=NO_BREAK_SPACE*/;
	period			= 0x2e;
	bPlus			= false;
	bZeroIsNegative = false;
	bForcePeriod	= false;
	bSetTextStyle	= false;
	bForceDecimals	= true;
	noDecimalThreshold	= 0;
	scale			= 1;
}

//____ WgValueFormat() ________________________________________________________

WgValueFormat::WgValueFormat( const wg::CharSeq& format )
{
	setFormat( format );
}

//____ WgValueFormat() ________________________________________________________

WgValueFormat::WgValueFormat( const WgValueFormat& in )
{
	integers	= in.integers;
	decimals	= in.decimals;
	grouping	= in.grouping;
	separator	= in.separator;
	period		= in.period;

	prefix		= in.prefix;
	suffix		= in.suffix;

	bPlus			= in.bPlus;
	bZeroIsNegative = in.bZeroIsNegative;
	bForcePeriod	= in.bForcePeriod;
	pTextStyle		= in.pTextStyle;
	bSetTextStyle	= in.bSetTextStyle;
	bForceDecimals	= in.bForceDecimals;
	noDecimalThreshold = in.noDecimalThreshold;
	scale			= in.scale;
}

//____ WgValueFormat() ________________________________________________________

WgValueFormat::WgValueFormat( int nInt, int nDec, int grouping, bool bPlus,
								Uint16 _separator, Uint16 period, bool bForcePeriod,
								const char * pPrefix, const char * pSuffix )
{
	separator		= _separator /*0xA0=NO_BREAK_SPACE*/;
	period			= 0x2e;

	bSetTextStyle	= false;
	bForceDecimals	= true;
	bZeroIsNegative = false;
	noDecimalThreshold	= 0;

	setFormat( nInt, nDec, grouping, bPlus, separator, period, bForcePeriod );
	setPrefix( pPrefix );
	setSuffix( pSuffix );

}


//____ setFormat() ____________________________________________________________

void WgValueFormat::setFormat( const wg::CharSeq& format )
{
	integers		= 1;
	decimals		= 0;
	grouping		= 0;
	separator		= 0xA0 /*0xA0=NO_BREAK_SPACE*/;
	period			= 0x2e;
	bPlus			= false;
	bZeroIsNegative = false;
	bForcePeriod	= false;
	bSetTextStyle	= false;
	bForceDecimals	= true;
	noDecimalThreshold	= 0;
	scale			= 1;

	wg::CharSeq::CharBasket basket = format.getChars();

	const wg::Char * pBeg = basket.ptr;
	const wg::Char * pEnd = basket.ptr + basket.length;

	// Copy prefix

	const wg::Char * p = pBeg;
	while(  p < pEnd && p->code() != '1' )
	{
		assert( p->code() < '0' || p->code() > '9' );		// No numerics allowed in prefix.
		p++;
	}

	prefix = wg::CharSeq( pBeg, int(p - pBeg) );

	pBeg = p;

	//

	if( pBeg == pEnd )
		return;

	// Save textprops of the '1' character.

	pTextStyle = pBeg->stylePtr();
	pBeg++;

	//

	if( pBeg == pEnd )
		return;

	// Check for separator character

	bool	bHasSeparator = false;

	if( pEnd - pBeg >= 2 )
	{
		Uint16 sep = pBeg->code();
		Uint16 after = pBeg[1].code();

		if( (sep < '0' || sep > '9') && after == '0' )
		{
			separator = sep;
			pBeg++;
			bHasSeparator = true;
		}
	}

	//

	if( pBeg == pEnd )
		return;

	// Count zeroes for grouping

	int nZeroes = 0;

	while( pBeg != pEnd && pBeg->code() == '0' )
	{
		pBeg++;
		nZeroes++;
	}

	if( bHasSeparator && nZeroes > 0 )
		grouping = nZeroes;

	//

	if( pBeg == pEnd )
		return;

	// Check for optional decimal delimeter with 1-6 following zeroes or x:es

	if( pEnd - pBeg >= 2 )
	{
		Uint16 sep = pBeg->code();
		Uint16 after = pBeg[1].code();

		if( after == '0' || after == 'x' || after == 'X' )
		{
			period = sep;
			pBeg+=1;
			nZeroes = 0;
			while( pBeg != pEnd && pBeg->code() == '0' )
			{
				pBeg++;
				nZeroes++;
			}

			int nXes = 0;
			while( pBeg != pEnd && (pBeg->code() == 'x' || pBeg->code() == 'X') )
			{
				pBeg++;
				nXes++;
			}

			decimals = nZeroes;
			scale = (int) pow(10.0,nZeroes+nXes);
		}
	}

	//

	if( pBeg == pEnd )
		return;

	// Copy suffix

	suffix = wg::CharSeq( pBeg, int(pEnd - pBeg) );
}

//____ setFormat() ____________________________________________________________

void WgValueFormat::setFormat( int _nInt, int _nDec, int _grouping, bool _bPlus,
															Uint16 _separator, Uint16 _period, bool _bForcePeriod )
{
	integers		= _nInt;
	decimals		= _nDec;
	grouping		= _grouping;
	bPlus			= _bPlus;
	bForcePeriod	= _bForcePeriod;
	if( _separator )
		separator = _separator;
	if( _period )
		period		= _period;

	scale			= 1;
}

//____ setPrefix() ___________________________________________________________

void WgValueFormat::setPrefix( const wg::String& str )
{
	prefix = str;
}

//____ setSuffix() ___________________________________________________________

void WgValueFormat::setSuffix( const wg::String& str )
{
	suffix = str;
}
