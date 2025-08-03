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

#include <wg_lineeditor.h>
#include <wg_key.h>
#include <wg_gfxdevice.h>
#include <wg_msgrouter.h>
#include <wg_base.h>

namespace wg
{
	using namespace Util;

	const TypeInfo LineEditor::TYPEINFO = { "LineEditor", &Widget::TYPEINFO };



	//____ LineEditor() _________________________________________________________________

	LineEditor::LineEditor() : editor(this), m_textScrollOfs(0)
	{
		editor.setMaxLines(1);
	}

	//____ Destructor _____________________________________________________________

	LineEditor::~LineEditor()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LineEditor::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDefaultLengthInChars() _____________________________________________

	void LineEditor::setDefaultLengthInChars( int whitespaceChars )
	{
		if( whitespaceChars != m_defaultLengthInChars )
		{
			m_defaultLengthInChars = whitespaceChars;
			_requestResize();
		}
	}

	//____ setReturnKeyAction() ______________________________________________

	void LineEditor::setReturnKeyAction(KeyAction action)
	{
		m_returnKeyAction = action;
	}

	//____ _defaultSize() _____________________________________________________________

	SizeSPX LineEditor::_defaultSize(int scale) const
	{
		//TODO: Use real text size as default size instead. They should use a SizeCapsule to limit the size if needed.

		SizeSPX	contentSize;

		if( m_defaultLengthInChars < 1 )
		{
			contentSize = editor._defaultSize(scale);
		}
		else
		{
			TextStyle * pStyle = editor._style();

			Font_p pFont = pStyle->font();

			if (pFont)
			{
				pFont->setSize( pStyle->size( State::Default ) * scale );
				contentSize.w = pFont->whitespaceAdvance() * m_defaultLengthInChars;
				contentSize.h = pFont->maxAscend() + pFont->maxDescend();
			}
			else
			{
				contentSize.w = m_defaultLengthInChars * 8 * scale;
				contentSize.h = 16 * scale;
			}

			contentSize = alignUp(contentSize);
		}

		return m_skin.sizeForContent( contentSize, scale );
	}

	//____ _render() ________________________________________________________

	void LineEditor::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		RectSPX contentRect = m_skin.contentRect(_canvas, m_scale, m_state);

		//

		RectSPX textCanvas(contentRect.x - m_textScrollOfs, contentRect.y, _editorWidth(), contentRect.h);

		// We need to clip to canvas since textCanvas can go outside our canvas.

		auto pop = limitClipList(pDevice, contentRect );
		editor._render(pDevice, textCanvas );
		popClipList(pDevice, pop);
	}

	//____ _setState() ______________________________________________________

	void LineEditor::_setState( State state )
	{
		bool bEditorNeedsResize = m_state.isFocused() != state.isFocused();

		Widget::_setState(state);
		editor._setState(state);

		if( !state.isFocused() )
			m_textScrollOfs = 0;

		_requestRender(); //TODO: Only requestRender if skin or text appearance has changed.

		if( bEditorNeedsResize )
		{
			editor._setSize( SizeSPX( _editorWidth(), m_size.h - m_skin.contentBorderSize(m_scale).h ), m_scale );

			if( state.isFocused() )
				editor.caretIntoView();
		}
	}

	//____ _receive() ______________________________________________________________

	void LineEditor::_receive( Msg * pMsg )
	{
		if (pMsg->type() == MsgType::KeyPress)
		{
			auto keyCode = static_cast<KeyPressMsg*>(pMsg)->translatedKeyCode();

			if (keyCode == Key::Escape)
			{
				releaseFocus();
				pMsg->swallow();
				return;						// We don't swallow msg on purpose. Parent might also want to react on escape.
			}

			if (keyCode == Key::Return)
			{
				switch (m_returnKeyAction)
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

	void LineEditor::_resize( const SizeSPX& size, int scale )
	{
		Widget::_resize( size, scale );

		editor._setSize( SizeSPX( _editorWidth(), size.h - m_skin.contentBorderSize(m_scale).h ), scale );
	}

	//____ _componentPos() __________________________________________________________

	CoordSPX LineEditor::_componentPos( const Component * pComponent ) const
	{
		if (pComponent != &editor)
			return CoordSPX();

		CoordSPX c(-m_textScrollOfs, 0);

		return m_skin.contentOfs( m_scale, m_state ) + c;
	}

	//____ _componentSize() _________________________________________________________

	SizeSPX LineEditor::_componentSize( const Component * pComponent ) const
	{
		if (pComponent != &editor)
			return m_size;

		return SizeSPX( _editorWidth(), m_size.h - m_skin.contentBorderSize(m_scale).h );
	}

	//____ _componentGeo() __________________________________________________________

	RectSPX LineEditor::_componentGeo( const Component * pComponent ) const
	{
		if (pComponent != &editor)
			return m_size;

		RectSPX r = m_skin.contentRect( m_size, m_scale, m_state );
		r.x -= m_textScrollOfs;
		r.w = _editorWidth();
		return r;
	}

	//____ _componentRequestRender() ______________________________________________

	void LineEditor::_componentRequestRender(const Component * pComponent)
	{
		if (pComponent != &editor)
			return _requestRender();

		_requestRender(m_skin.contentRect(m_size, m_scale, m_state));
	}

	void LineEditor::_componentRequestRender( const Component * pComponent, const RectSPX& rect )
	{
		if (pComponent != &editor)
			return _requestRender(rect);

		RectSPX dirt = rect;
		dirt.x -= m_textScrollOfs;

		RectSPX visible = m_skin.contentRect( m_size, m_scale, m_state );
		dirt += visible.pos();

		if (dirt.isOverlapping(visible))
			_requestRender(RectSPX::overlap(dirt, visible));
	}

	//____ _componentRequestResize() ______________________________________________

	void LineEditor::_componentRequestResize( const Component * pComponent )
	{
		if (pComponent != &editor)
			return Widget::_componentRequestResize(pComponent);

		SizeSPX defaultSize = editor._defaultSize(m_scale);

		SizeSPX contentSize = m_size - m_skin.contentBorderSize(m_scale);

		if(defaultSize.h != contentSize.h || (m_defaultLengthInChars < 1 && defaultSize.w != contentSize.w) )
			_requestResize();
		else
			editor._setSize( SizeSPX(_editorWidth(), contentSize.h ),m_scale);
	}

	//____ _componentRequestInView() ____________________________________________

	void LineEditor::_componentRequestInView(const Component * pComponent, const RectSPX& mustHave, const RectSPX& niceToHave)
	{
		spx scrollOfs = m_textScrollOfs;
		SizeSPX canvas = _componentSize(pComponent);

		SizeSPX window = m_size - m_skin.contentBorderSize(m_scale);

		if (scrollOfs > 0 && canvas.w - scrollOfs < window.w)
			scrollOfs = canvas.w < window.w ? 0 : canvas.w - window.w;

		if( mustHave.x < scrollOfs )
			scrollOfs = mustHave.x;

		if( mustHave.x + mustHave.w > scrollOfs + window.w )
			scrollOfs = mustHave.x + mustHave.w - window.w;

		if (scrollOfs != m_textScrollOfs)
		{
			m_textScrollOfs = scrollOfs;
			_requestRender();
		}
	}

	//____ _receiveComponentNotif() ______________________________________________

	void LineEditor::_receiveComponentNotif( Component * pComponent, ComponentNotif notification, int value, void * pData )
	{
		if( notification == ComponentNotif::TextModified )
		{
			auto p = static_cast<StaticText::NotifData*>(pData);

			Base::msgRouter()->post(TextEditMsg::create(&editor, p->offset, p->deleted, p->inserted));
		}
	}

	//____ _editorWidth() ________________________________________________________

	spx LineEditor::_editorWidth() const
	{
		if( m_state.isFocused() )
			return editor._defaultSize(m_scale).w;
		else
			return _contentRect().w;
	}


} // namespace wg
