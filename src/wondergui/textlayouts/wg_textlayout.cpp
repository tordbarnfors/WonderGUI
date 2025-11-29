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
#include <wg_textlayout.h>
#include <wg_base.h>

namespace wg
{

	const TypeInfo TextLayout::TYPEINFO = { "TextLayout", &Receiver::TYPEINFO };


	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ tooltip() _______________________________________________________________

	String TextLayout::tooltip( const TextItem * pText ) const
	{
		return String();
	}

	//____ _chars() ___________________________________________________________

	const Char * TextLayout::_chars( const TextItem * pText ) const
	{
		return pText->_textBegin();
	}

	//____ _dataBlock() _______________________________________________________

	void * TextLayout::_dataBlock( TextItem * pText) const
	{
		return pText->m_pMapperData;
	}

	const void * TextLayout::_dataBlock( const TextItem * pText) const
	{
		return pText->m_pMapperData;
	}


	//____ _dataInt() _________________________________________________________

	int TextLayout::_dataInt( const TextItem * pText ) const
	{
		return pText->m_mapperData;
	}

	//____ _baseStyle() _________________________________________________________

	const TextStyle * TextLayout::_baseStyle( const TextItem * pText ) const
	{
		TextStyle * pStyle = pText->_textStyle();
		if( pStyle == 0 )
			pStyle = Base::defaultStyle().rawPtr();

		return pStyle;
	}

	//____ _state() _________________________________________________________

	State TextLayout::_state( const TextItem * pText ) const
	{
		return pText->_textState();
	}

	//____ _setTextDataBlock() ____________________________________________________

	void  TextLayout::_setTextDataBlock( TextItem * pText, void * pBlock )
	{
		pText->m_pMapperData = pBlock;
	}

	//____ _setTextDataInt() ______________________________________________________

	void  TextLayout::_setTextDataInt( TextItem * pText, int data )
	{
		pText->m_mapperData = data;
	}

	//____ _setTextDirty() ____________________________________________________

	void  TextLayout::_setTextDirty( TextItem * pText )
	{
		pText->_mapperRequestRender();
	}

	void  TextLayout::_setTextDirty( TextItem * pText, const RectSPX& rect )
	{
		pText->_mapperRequestRender( rect );
	}

	//____ _requestTextResize() _______________________________________________

	void TextLayout::_requestTextResize( TextItem * pText )
	{
		pText->_mapperRequestResize();
	}

	//____ _size() _____________________________________________________________

	SizeSPX TextLayout::_size( const TextItem * pText ) const
	{
		return pText->_textSize();
	}

	//____ _length() ___________________________________________________________

	int TextLayout::_length( const TextItem * pText ) const
	{
		return pText->_textLength();
	}


	//____ _caretVisible() _____________________________________________________

	bool TextLayout::_caretVisible( const TextItem * pText ) const
	{
		return pText->_caretVisible();
	}

	//____ _caretOfs() ____________________________________________________________

	int TextLayout::_caretOfs( const TextItem * pText ) const
	{
		return pText->_caretOffset();
	}

	//____ _selection() ________________________________________________________
	
	std::tuple<int,int>	TextLayout::_selection( const TextItem * pText ) const
	{
		return pText->_selectedText();
	}

	//____ _scale() ___________________________________________________________

	int TextLayout::_scale(const TextItem * pText) const
	{
		return pText->_scale();
	}



} // namespace wg
