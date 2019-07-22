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

#ifndef	WG_CTEXTEDITOR_DOT_H
#define WG_CTEXTEDITOR_DOT_H
#pragma once


#include <wg_ctextdisplay.h>

namespace wg
{

	//____ CTextEditor __________________________________________________________________

	class CTextEditor : public CTextDisplay	/** @private */
	{
	public:
		CTextEditor( ComponentHolder * pHolder );

		void			receive( Msg * pMsg );

		// Overloaded so we can update caret and selection

		void			clear();
		bool			setMaxLines( int maxLines );
		bool			setMaxChars( int maxChars );
		int				maxLines() const { return m_maxLines; }
		int				maxChars() const { return m_maxChars; }

		void			set( const CharSeq& seq );
		void			set( const CharBuffer * buffer );
		void			set( const String& str );

		int				append( const CharSeq& seq );
		int				insert( int ofs, const CharSeq& seq );
		int				replace( int ofs, int nDelete, const CharSeq& seq );
		int				erase( int ofs, int len );

		void			setState( State state );
		void			render( GfxDevice * pDevice, const RectI& _canvas );


		void			setEditMode( TextEditMode mode );
		TextEditMode	editMode() const { return m_editMode; };


		// These methods will fail if editMode is Static

		bool			select( int begin, int end );		// Cursor (if enabled) gets end position. End can be smaller than begin.
		bool			selectAll();
		bool			unselect();
		int				eraseSelected();

		int				selectionBegin() const;				// Begin position of selection, might be after end position.
		int				selectionEnd() const;
		int				selectionSize() const;				// Number of characters that are selected.

		inline bool		hasSelection() const { return m_editState.caretOfs != m_editState.selectOfs; }

		// These methods will fail unless caret is present

		bool			setCaretOfs( int ofs );			// Move cursor to offset. Any selection will be unselected.
		int				caretOfs() const;

		int				caretPut( const CharSeq& seq );	// Will insert or overwrite depending on caret mode
		bool			caretPut( uint16_t c );			// " -

		bool			caretUp();
		bool			caretDown();
		bool			caretLeft();
		bool			caretRight();

		bool			caretNextWord();
		bool			caretPrevWord();

		bool			caretEraseNextChar();
		bool			caretErasePrevChar();
		bool			caretEraseNextWord();
		bool			caretErasePrevWord();

		bool			caretLineBegin();
		bool			caretLineEnd();
		bool			caretTextBegin();
		bool			caretTextEnd();

		bool			caretToPos( CoordI pos);			// Move caret as close as possible to the given position.
		bool			caretSelectWord();
		bool			caretSelectLine();				// Full line with hard line ending.




	protected:

		enum class MoveMethod
		{
			Keyboard,
			Mouse,
			ApiCall
		};


		bool				_moveCaret( int caretOfs, MoveMethod method );
		void				_updateDisplayArea();
		void				_updateInsertStyle();

		const EditState * 	_editState() const;


		void			_caretToEnd();

		TextEditMode	m_editMode;
		EditState		m_editState;
		int				m_maxLines;
		int				m_maxChars;
	};

} // namespace wg
#endif //WG_CTEXTEDITOR_DOT_H
