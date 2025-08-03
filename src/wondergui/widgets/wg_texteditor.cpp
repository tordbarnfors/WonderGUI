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

#include <wg_texteditor.h>
#include <wg_key.h>
#include <wg_gfxdevice.h>
#include <wg_msgrouter.h>
#include <wg_base.h>

namespace wg
{

	const TypeInfo TextEditor::TYPEINFO = { "TextEditor", &Widget::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextEditor::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setReturnKeyAction() ______________________________________________

	void TextEditor::setReturnKeyAction(KeyAction action)
	{
		m_returnKeyAction = action;
	}

	//____ setTabKeyAction() ______________________________________________

	void TextEditor::setTabKeyAction(KeyAction action)
	{
		m_tabKeyAction = action;
	}

	//____ _matchingWidth() _______________________________________________________

	spx TextEditor::_matchingWidth(spx height, int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale);

		height -= padding.h;
		spx textWidth = editor._matchingWidth(height, scale);
		textWidth += padding.w;
		return textWidth;
	}

	//____ _matchingHeight() _______________________________________________________

	spx TextEditor::_matchingHeight( spx width, int scale ) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale);

		width -= padding.w;
		spx textHeight = editor._matchingHeight( width, scale );
		textHeight += padding.h;
		return textHeight;
	}

	//____ _defaultSize() _____________________________________________________________

	SizeSPX TextEditor::_defaultSize(int scale) const
	{
		SizeSPX contentSize = editor._defaultSize(scale);

		return m_skin.sizeForContent(contentSize,scale);
	}

	//____ _render() ________________________________________________________

	void TextEditor::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		RectSPX canvas = m_skin.contentRect(_canvas, m_scale, m_state);

		editor._render( pDevice, canvas );
	}

	//____ _setState() ______________________________________________________

	void TextEditor::_setState( State state )
	{
		Widget::_setState(state);

		editor._setState(state);
		_requestRender(); //TODO: Only requestRender if skin or text appearance has changed.
	}


	//____ _receive() ______________________________________________________________

	void TextEditor::_receive( Msg * pMsg )
	{
		if (pMsg->type() == MsgType::KeyPress)
		{
			auto keyCode = static_cast<KeyPressMsg*>(pMsg)->translatedKeyCode();

			if (keyCode == Key::Tab || keyCode == Key::Return)
			{
				auto action = keyCode == Key::Tab ? m_tabKeyAction : m_returnKeyAction;

				switch (action)
				{
					case KeyAction::None:
						return;								// We don't swallow msg here, let parent react as well.
					case KeyAction::Insert:
						break;								// Msg should be swallowed by editor later.
					case KeyAction::ReleaseFocus:
					{
						releaseFocus();
						pMsg->swallow();
						return;
					}
					case KeyAction::CycleFocus:
					{

						//TODO: Implement!!!
						pMsg->swallow();
						return;
					}
				}
			}
		}

		Widget::_receive( pMsg );
		editor._receive( pMsg );
	}


	//____ _resize() ________________________________________________

	void TextEditor::_resize( const SizeSPX& size, int scale )
	{
		Widget::_resize( size, scale );

		editor._setSize(size - m_skin.contentBorderSize(m_scale), m_scale);
	}

	//____ _markTest() ________________________________________________________

	bool TextEditor::_markTest(const CoordSPX& ofs)
	{
		// Always accept marks within text area.
		
		if( _contentRect().contains(ofs) )
			return true;
		
		return Widget::_markTest(ofs);
	}

	//____ _receiveComponentNotif() ______________________________________________

	void TextEditor::_receiveComponentNotif( Component * pComponent, ComponentNotif notification, int value, void * pData )
	{
		if( notification == ComponentNotif::TextModified )
		{
			auto p = static_cast<StaticText::NotifData*>(pData);
			
			Base::msgRouter()->post(TextEditMsg::create(&editor, p->offset, p->deleted, p->inserted));
		}
	}


} // namespace wg
