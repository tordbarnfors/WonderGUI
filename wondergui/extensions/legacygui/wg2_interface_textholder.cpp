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
#include <wg2_interface_textholder.h>

//____ Constructor ____________________________________________________________

Wg_Interface_TextHolder::Wg_Interface_TextHolder()
{
	m_pText		= 0;
}


//____ SetTextState() __________________________________________________________

void Wg_Interface_TextHolder::SetTextState( wg::State _state )
{
	if( _state != m_pText->state() )
	{
		m_pText->setState(_state);
		_textModified();
	}
}

//____ SetTextWrap() __________________________________________________________
bool Wg_Interface_TextHolder::SetTextWrap( bool bWrap )
{
	if( bWrap != m_pText->IsWrap() )
	{
		m_pText->SetWrap(bWrap);
		_textModified();
	}
	return true;
}

//____ SetAutoEllipsis() __________________________________________________________
bool Wg_Interface_TextHolder::SetAutoEllipsis( bool bAutoEllipsis )
{
	if( bAutoEllipsis != m_pText->IsAutoEllipsis() )
	{
		m_pText->SetAutoEllipsis(bAutoEllipsis);
		_textModified();
	}
	return true;
}

//____ GetStdString() _____________________________________________________

std::string Wg_Interface_TextHolder::GetStdString() const
{
    char* buf = m_pText->getTextUTF8(); // This calls new, we need to explicitly call delete[] below.
    std::string str = std::string(buf);
    delete[] buf;

    return str;
}

//____ SetTextAlignment() _____________________________________________________

void Wg_Interface_TextHolder::SetTextAlignment( const WgOrigo alignment )
{
	if( alignment != m_pText->alignment() )
	{
		m_pText->setAlignment(alignment);
		_textModified();
	}
}

//____ SetTextTintMode() _____________________________________________________

void Wg_Interface_TextHolder::SetTextTintMode( WgTintMode mode )
{
	if( mode != m_pText->tintMode() )
	{
		m_pText->setTintMode(mode);
		_textModified();
	}
}


//____ Wg_Interface_TextHolder::ClearText() ___________________________________

void Wg_Interface_TextHolder::ClearText()
{
	if( m_pText )
	{
		m_pText->clear();
		_textModified();
	}
}


void Wg_Interface_TextHolder::SetText( const wg::CharBuffer * pBuffer )
{
	m_pText->setText( pBuffer );
	_textModified();
}

void Wg_Interface_TextHolder::SetText( const wg::String& str )
{
	m_pText->setText( str );
	_textModified();
}

void Wg_Interface_TextHolder::SetText( const WgText * pText )
{
	m_pText->setText( pText );
	_textModified();
}

int Wg_Interface_TextHolder::AddText( const wg::CharSeq& seq )
{
	int nAdded = m_pText->addText( seq );
	_textModified();
	return nAdded;
}

int Wg_Interface_TextHolder::InsertText( int ofs, const wg::CharSeq& seq )
{
	int nAdded = m_pText->insertText( ofs, seq );
	_textModified();
	return nAdded;
}

int Wg_Interface_TextHolder::ReplaceText( int ofs, int nDelete, const wg::CharSeq& seq )
{
	int nAdded = m_pText->replaceText( ofs, nDelete, seq );
	_textModified();
	return nAdded;
}

int Wg_Interface_TextHolder::DeleteText( int ofs, int len )
{
	int nDeleted = m_pText->deleteText( ofs, len );
	_textModified();
	return nDeleted;
}

void Wg_Interface_TextHolder::DeleteSelectedText()
{
	m_pText->deleteSelectedText();
	_textModified();
}

int Wg_Interface_TextHolder::AddChar( const wg::Char& character )
{
	int nAdded = m_pText->addChar( character );
	_textModified();
	return nAdded;
}

int Wg_Interface_TextHolder::InsertChar( int ofs, const wg::Char& character )
{
	int nAdded = m_pText->insertChar( ofs, character );
	_textModified();
	return nAdded;
}

int Wg_Interface_TextHolder::ReplaceChar( int ofs, const wg::Char& character )
{
	int nAdded = m_pText->replaceChar( ofs, character );
	_textModified();
	return nAdded;
}

int Wg_Interface_TextHolder::DeleteChar( int ofs )
{
	int nDeleted = m_pText->deleteChar( ofs );
	_textModified();
	return nDeleted;
}



//____ SetLineSpaceAdjustment() __________________________________________________

void Wg_Interface_TextHolder::SetLineSpaceAdjustment( float adjustment )
{
	m_pText->setLineSpaceAdjustment( adjustment );
	_textModified();
}

//____ GetLineSpaceAdjustment() __________________________________________________

float Wg_Interface_TextHolder::GetLineSpaceAdjustment() const
{
	return m_pText->lineSpaceAdjustment();
}


//____ _cloneInterface() _______________________________________________________

//DEPRECATED! To be removed once all widgets and items are widgets.

void Wg_Interface_TextHolder::_cloneInterface( Wg_Interface_TextHolder * _pClone )
{
}

//____ _onCloneContent() _______________________________________________________

void Wg_Interface_TextHolder::_onCloneContent( const Wg_Interface_TextHolder * pOrg )
{
}
