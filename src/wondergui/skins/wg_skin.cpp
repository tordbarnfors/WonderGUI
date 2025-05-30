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
#include <wg_skin.h>
#include <wg_util.h>
#include <wg_base.h>

namespace wg
{

	const TypeInfo Skin::TYPEINFO = { "Skin", &Object::TYPEINFO };

	using namespace Util;

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Skin::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ isOpaque() _________________________________________________________

	bool Skin::isOpaque(State state) const
	{
		return (_coverage({ 0,0,100 * 64,100 * 64 }, 64, state) == RectSPX{0, 0, 100 * 64, 100 * 64});
	}


	//____ _minSize() ______________________________________________________________

	SizeSPX Skin::_minSize(int scale) const
	{
		return SizeSPX(align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_padding,scale)));
	}

	//____ _defaultSize() ______________________________________________________________

	SizeSPX Skin::_defaultSize(int scale) const
	{
		return SizeSPX(align(ptsToSpx(m_spacing,scale)) + align(ptsToSpx(m_padding,scale)));
	}

	//____ _margin() ______________________________________________________________

	BorderSPX Skin::_margin(int scale, State state) const
	{
		return align(ptsToSpx(m_spacing, scale));
	}

	//____ _marginSize() ______________________________________________________________

	SizeSPX Skin::_marginSize(int scale) const
	{
		return SizeSPX(align(ptsToSpx(m_spacing, scale)));
	}

	//____ _padding() ______________________________________________________________

	BorderSPX Skin::_padding(int scale, State state) const
	{
		return align(ptsToSpx(m_padding, scale));
	}

	//____ _paddingSize() ______________________________________________________________

	SizeSPX Skin::_paddingSize(int scale) const
	{
		return SizeSPX(align(ptsToSpx(m_padding, scale)));
	}

	//____ _overflow() ______________________________________________________________

	BorderSPX Skin::_overflow(int scale) const
	{
		return align(ptsToSpx(m_overflow, scale));
	}

	//____ _geoOverflow() ________________________________________________________

	BorderSPX Skin::_geoOverflow(int scale) const
	{
		BorderSPX overflow = align(ptsToSpx(m_overflow, scale)) - align(ptsToSpx(m_spacing, scale));

		if( overflow.top < 0 )
			overflow.top = 0;
		if( overflow.right < 0 )
			overflow.right = 0;
		if( overflow.bottom < 0 )
			overflow.bottom = 0;
		if( overflow.left < 0 )
			overflow.left = 0;
		
		return overflow;
	}

	//____ _contentOfs() ______________________________________________________________

	CoordSPX Skin::_contentOfs(int scale, State state) const
	{
		return align(ptsToSpx(Coord(m_spacing.left, m_spacing.top), scale)) + align(ptsToSpx(Coord(m_padding.left, m_padding.top), scale));
	}

	//____ _sizeForContent() ___________________________________________________

	SizeSPX Skin::_sizeForContent(const SizeSPX& contentSize, int scale) const
	{
		return contentSize + SizeSPX(align(ptsToSpx(m_spacing, scale))) + SizeSPX(align(ptsToSpx(m_padding, scale)));
	}

	//____ _contentRect() ______________________________________________________

	RectSPX Skin::_contentRect(const RectSPX& canvas, int scale, State state) const
	{
		return canvas - align(ptsToSpx(m_spacing, scale)) - align(ptsToSpx(m_padding, scale));
	}

	//____ _contentBorder() ___________________________________________________

	BorderSPX Skin::_contentBorder(int scale, State state) const
	{
		return align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_padding, scale));
	}

	//____ _contentBorderSize() _______________________________________________

	SizeSPX Skin::_contentBorderSize(int scale) const
	{
		return SizeSPX(align(ptsToSpx(m_spacing, scale))) + SizeSPX(align(ptsToSpx(m_padding, scale)));
	}

	//____ _renderBounds() _______________________________________________________

	RectSPX Skin::_renderBounds(const RectSPX& geo, int scale) const
	{
		return geo - align(ptsToSpx(m_spacing, scale)) + align(ptsToSpx(m_overflow, scale));
	}


	//____ _dirtyRect() ________________________________________________________

	RectSPX Skin::_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState, float newValue, float oldValue,
		float newValue2, float oldValue2, int newAnimPos, int oldAnimPos, float* pNewStateFractions,
		float* pOldStateFractions) const
	{
		return RectSPX();
	}

	//____ _animationLength() ____________________________________________

	int Skin::_animationLength(State state) const
	{
		return 0;
	}

	//____ _transitioningStates() __________________________________________________

	Bitmask<uint8_t> Skin::_transitioningStates() const
	{
		return Bitmask<uint8_t>(0);
	}

	//____ _transitionTimes() ____________________________________________

	const int* Skin::_transitionTimes() const
	{
		const static int	transitionTimes[PrimState_Nb] = { 0,0,0,0,0,0,0,0 };

		return transitionTimes;
	}

} // namespace wg
