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

#include <wg_textbase.h>

namespace wg
{
	using namespace Util;

	const TypeInfo	TextBase::TYPEINFO = { "TextBase", &Component::TYPEINFO };

	//____ constructor _____________________________________________________________

	TextBase::TextBase(Widget * pWidget ) : Component( pWidget )
	{
		_layout()->addText(this);
	}

	//____ destructor ______________________________________________________________

	TextBase::~TextBase()
	{
		_layout()->removeText(this);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextBase::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setStyle() ______________________________________________________________

	void TextBase::setStyle( TextStyle * pStyle )
	{
		if( pStyle == m_pStyle )
			return;
		
		TextStyle_p pOld = m_pStyle;			// Keep ref count until onStyleChanged has been called.
		m_pStyle = pStyle;
		_layout()->onStyleChanged(this, pStyle, pOld);
	}

	//____ clearStyle() ____________________________________________________________

	void TextBase::clearStyle()
	{
		if( !m_pStyle )
			return;
		
		TextStyle_p pOld = m_pStyle;			// Keep ref count until onStyleChanged has been called.
		m_pStyle = 0;
		_layout()->onStyleChanged(this, nullptr, pOld);
	}

	//____ setLayout() __________________________________________________________

	void TextBase::setLayout( TextLayout * pLayout )
	{
		if( pLayout == m_pLayout )
			return;

		_layout()->removeText(this);
		m_pLayout = pLayout;
		_layout()->addText(this);
	}

	//____ clearLayout() ________________________________________________________

	void TextBase::clearLayout()
	{
		if( !m_pLayout )
			return;

		_layout()->removeText(this);
		m_pLayout = 0;
		_layout()->addText(this);
	}

	//____ _setState() ______________________________________________________________

	void TextBase::_setState( State state )
	{
		if( state == m_state )
			return;

		State old = m_state;
		m_state = state;
		_layout()->onStateChanged( this, state, old );
	}

	//____ _defaultSize() _________________________________________________________

	SizeSPX TextBase::_defaultSize(int scale) const
	{
		return _layout()->defaultSize(this, scale);
	}

	//____ _matchingWidth() _________________________________________________________

	spx TextBase::_matchingWidth( spx height, int scale ) const
	{
		return _layout()->matchingWidth(this, height, scale);
	}

	//____ _matchingHeight() ________________________________________________________

	spx TextBase::_matchingHeight( spx width, int scale ) const
	{
		return _layout()->matchingHeight(this, width, scale);
	}

	//____ _charAtPos() ___________________________________________________________

	int TextBase::_charAtPos( CoordSPX pos ) const
	{
		return _layout()->charAtPos(this,pos);
	}

	//____ _charRect() ____________________________________________________________

	RectSPX TextBase::_charRect( int charOfs ) const
	{
		return _layout()->charRect(this, charOfs);
	}

	//____ _charLine() ____________________________________________________________

	int TextBase::_charLine( int charOfs ) const
	{
		return _layout()->charLine(this, charOfs);
	}

	//____ _setSize() _____________________________________________________________

	void TextBase::_setSize( SizeSPX size, int scale )
	{
		if( size == m_size && scale && m_scale )
			return;

		SizeSPX oldSize = m_size;
		int		oldScale = m_scale;
		m_size = size;
		m_scale = scale;
		_layout()->onResized(this,size, oldSize, scale, oldScale);
	}

	//_____ _render() _____________________________________________________________

	void  TextBase::_render( GfxDevice * pDevice, const RectSPX& _canvas )
	{
		_layout()->render(this, pDevice, _canvas );
	}

	//____ _rectForRange() __________________________________________________________

	RectSPX  TextBase::_rectForRange( int ofs, int length ) const
	{
		return _layout()->rectForRange(this, ofs, length);
	}

	//____ _tooltip() _______________________________________________________________

	String TextBase::_tooltip() const
	{
		return _layout()->tooltip(this);
	}

	//____ _getString() ___________________________________________________________________

	String TextBase::_getString() const
	{
		return String(&m_charBuffer);
	}

	//____ _clear() _________________________________________________________________

	void TextBase::_clear()
	{
		int removed = m_charBuffer.length();
		m_charBuffer.clear();
		_modified(0, removed, 0);
	}

	//___ _setText() ____________________________________________________________________

	void TextBase::_setText(const CharSeq& seq)
	{
		CharSeq::CharBasket chars = seq.getChars();

		if( chars.length == m_charBuffer.length() )
		{
			auto p1 = m_charBuffer.chars();
			auto p2 = chars.ptr;
			for( int i = 0 ; i < chars.length ; i++ )
				if( (* p1++).equals(* p2++) == false )
				{
					p2--;
					break;
				}

			if( p2 == chars.ptr + chars.length )
				return;
		}

		int removed = m_charBuffer.length();
		m_charBuffer = seq;
		_modified(0, removed, m_charBuffer.length());
	}

	void TextBase::_setText(const CharBuffer * buffer)
	{
		if( m_charBuffer.compareTo(buffer) == 0 )
			return;

		int removed = m_charBuffer.length();
		m_charBuffer = *buffer;
		_modified(0, removed, m_charBuffer.length());
	}

	void TextBase::_setText(const String& str)
	{
		if( TextTool::strcmp( m_charBuffer.chars(), str.chars() ) == 0 )
			return;

		int removed = m_charBuffer.length();
		m_charBuffer = str;
		_modified(0, removed, m_charBuffer.length());
	}

	//____ _append() ________________________________________________________________

	int TextBase::_append(const CharSeq& seq)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int ofs = m_charBuffer.length();
		int len = m_charBuffer.pushBack(seq);
		_modified(ofs, 0, len);
		return len;
	}

	//____ _insert() ________________________________________________________________

	int TextBase::_insert(int ofs, const CharSeq& seq)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int len = m_charBuffer.insert(ofs, seq);
		_modified(ofs, 0, seq.length());
		return len;
	}

	//____ _replace() ___________________________________________________________

	int TextBase::_replace(int ofs, int nDelete, const CharSeq& seq)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int diff = m_charBuffer.replace(ofs, nDelete, seq);
		_modified(ofs, nDelete, seq.length());
		return diff;
	}

	//____ _erase() ____________________________________________________________

	int TextBase::_erase(int ofs, int len)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int removed = m_charBuffer.remove(ofs, len);
		_modified(ofs, len, 0);
		return removed;
	}

	//____ _setCharStyle() ______________________________________________________

	void TextBase::_setCharStyle(TextStyle * pStyle)
	{
		m_charBuffer.setStyle(pStyle);
		_layout()->onCharStyleChanged(this);
	}

	void TextBase::_setCharStyle(TextStyle * pStyle, int ofs, int len)
	{
		m_charBuffer.setStyle(pStyle, ofs, len);
		_layout()->onCharStyleChanged(this, ofs, len);
	}

	//____ _clearCharStyle() ____________________________________________________

	void TextBase::_clearCharStyle()
	{
		m_charBuffer.clearStyle();
		_layout()->onCharStyleChanged(this);
	}

	void TextBase::_clearCharStyle(int ofs, int len)
	{
		m_charBuffer.clearStyle(ofs, len);
		_layout()->onCharStyleChanged(this, ofs, len);
	}

	//____ _mapperRequestRender() _______________________________________________

	void TextBase::_mapperRequestRender(const RectSPX& rect)
	{
		_requestRender(rect);
	}

	//____ _modified() _________________________________________________________

	void TextBase::_modified( int ofs, int deleted, int inserted )
	{
		_layout()->onTextModified(this, ofs, deleted, inserted);

		NotifData notif = { ofs, deleted, inserted };
		_notify(ComponentNotif::TextModified, 0, &notif);
	}

} // namespace wg
