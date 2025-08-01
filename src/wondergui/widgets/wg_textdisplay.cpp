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

#include <wg_textdisplay.h>
#include <wg_key.h>
#include <wg_font.h>
#include <wg_gfxdevice.h>
#include <wg_msgrouter.h>

namespace wg
{

	const TypeInfo TextDisplay::TYPEINFO = { "TextDisplay", &Widget::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextDisplay ::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ pointerStyle() ________________________________________

	PointerStyle TextDisplay::pointerStyle() const
	{
		if( display.markedLink() )
			return PointerStyle::Hand;

		return m_pointerStyle;
	}

	//____ tooltip() _____________________________________________________

	String TextDisplay::tooltip() const
	{
		if( !m_tooltip.isEmpty() )
			return m_tooltip;
		else
			return display._tooltip();
	}

	//____ _matchingWidth() _______________________________________________________

	spx TextDisplay::_matchingWidth(spx height, int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale);

		height -= padding.h;
		spx textWidth = display._matchingWidth(height, scale);
		textWidth += padding.w;
		return textWidth;
	}

	//____ _matchingHeight() _______________________________________________________

	spx TextDisplay::_matchingHeight(spx width, int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale);

		width -= padding.w;
		spx textHeight = display._matchingHeight(width, scale);
		textHeight += padding.h;
		return textHeight;
	}

	//____ defaultSize() _____________________________________________________________

	SizeSPX TextDisplay::_defaultSize(int scale) const
	{
		SizeSPX contentSize = display._defaultSize(scale);

		return m_skin.sizeForContent(contentSize, scale);
	}

	//____ _render() ________________________________________________________

	void TextDisplay::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		RectSPX canvas = m_skin.contentRect(_canvas, m_scale, m_state);

		display._render(pDevice, canvas);
	}

	//____ _setState() ______________________________________________________

	void TextDisplay::_setState( State state )
	{
		Widget::_setState(state);
		display._setState(state);
		_requestRender(); //TODO: Only requestRender if text appearance has changed (let display.setState() return if rendering is needed)
	}


	//____ _receive() ______________________________________________________________

	void TextDisplay::_receive( Msg * pMsg )
	{
		Widget::_receive(pMsg);

		RectI canvas = m_skin.contentRect(m_size,m_scale,m_state);

		display._receive( pMsg );

	}


	//____ _resize() ________________________________________________

	void TextDisplay::_resize( const SizeSPX& size, int scale )
	{
		Widget::_resize(size, scale);

		SizeSPX textSize = size - m_skin.contentBorderSize(m_scale);		// Using m_scale here, since that has been fixed.

		display._setSize( textSize, scale );
	}


} // namespace wg
