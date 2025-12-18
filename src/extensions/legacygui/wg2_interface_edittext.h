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
#ifndef WG2_INTERFACE_EDITTEXT_DOT_H
#define WG2_INTERFACE_EDITTEXT_DOT_H

#ifndef WG2_INTERFACE_TEXTHOLDER_DOT_H
#	include	<wg2_interface_textholder.h>
#endif

class WgInterfaceEditText : public Wg_Interface_TextHolder
{
public:

	virtual void	SetEditMode(WgTextEditMode mode) = 0;

	virtual Uint32	InsertTextAtCursor( const wg::CharSeq& str ) = 0;
	virtual bool	InsertCharAtCursor( Uint16 c ) = 0;

	virtual void	GoBOL() = 0;
	virtual void	GoEOL() = 0;
	virtual void	GoBOF() = 0;
	virtual void	GoEOF() = 0;
};




#endif //WG2_INTERFACE_EDITTEXT_DOT_H
