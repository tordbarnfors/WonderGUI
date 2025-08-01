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

#include <wg_skindisplay.h>
#include <wg_gfxdevice.h>

namespace wg
{
	using namespace Util;

	const TypeInfo SkinDisplay::TYPEINFO = { "SkinDisplay", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	SkinDisplay::SkinDisplay()
	{
	}

	//____ destructor _____________________________________________________________

	SkinDisplay::~SkinDisplay()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SkinDisplay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDisplaySkin() _______________________________________________________

	void SkinDisplay::setDisplaySkin(Skin_p pSkin)
	{
		if (m_pDisplaySkin != pSkin)
		{
			m_pDisplaySkin = pSkin;
			_requestRender();
		}
	}

	//____ setDisplayState() _______________________________________________________

	void SkinDisplay::setDisplayState(State state)
	{
		if (m_displayState != state)
		{
			m_displayState = state;
			_requestRender();
		}
	}

	//____ setDisplayValue() _______________________________________________________

	void SkinDisplay::setDisplayValue(int value)
	{
		if (m_displayValue != value)
		{
			m_displayValue = value;
			_requestRender();
		}
	}

	//____ setDisplayValue2() ______________________________________________________

	void SkinDisplay::setDisplayValue2(int value2)
	{
		if (m_displayValue2 != value2)
		{
			m_displayValue2 = value2;
			_requestRender();
		}
	}

	//____ setDisplayScale() _______________________________________________________

	void SkinDisplay::setDisplayScale(int scale)
	{
		if (m_displayScale != scale)
		{
			m_displayScale = scale;
			_requestRender();
		}
	}

	//____ _render() ___________________________________________________________

	void SkinDisplay::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Widget::_render(pDevice, _canvas, _window);

		if (m_pDisplaySkin)
		{
			RectSPX content = _contentRect(_canvas);
			m_pDisplaySkin->_render(pDevice, content, m_displayScale, m_displayState, m_displayValue, m_displayValue2);
		}
	}

	//____ _defaultSize() ___________________________________________________________

	SizeSPX SkinDisplay::_defaultSize(int scale) const
	{
		if (m_pDisplaySkin)
			return SizeSPX::max( m_skin.defaultSize(scale), m_skin.contentBorderSize(scale) + m_pDisplaySkin->_defaultSize(scale) );
		else
			return m_skin.defaultSize(scale);
	}


} // namespace wg
