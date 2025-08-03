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

#ifndef	WG_EDITABLETEXT_DOT_H
#define WG_EDITABLETEXT_DOT_H
#pragma once

#include <algorithm>

#include <wg_dynamictext.h>

namespace wg
{
	class EditableText;
	typedef	ComponentPtr<EditableText>	EditableText_p;


	//____ EditableText __________________________________________________________________

	class EditableText : public DynamicText
	{
	public:

		//.____ Blueprint ______________________________________________________

		struct Blueprint
		{
			TextEditMode	editMode = TextEditMode::Editable;
			TextLayout_p	layout;
			int				maxChars = 0;
			int				maxLines = 0;
			TextStyle_p		style;
			String			text;

		};

		EditableText(Widget * pWidget );
		virtual ~EditableText() {};

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ State __________________________________________________

		void			setEditMode(TextEditMode mode);
		TextEditMode	editMode() const { return m_editMode; };
		inline bool		isSelectable() const { return !(editMode() == TextEditMode::Static); }
		inline bool		isEditable() const { return editMode() == TextEditMode::Editable; }

		//.____ Content _____________________________________________

		int				caretPut(const CharSeq& seq);	// Will insert or overwrite depending on caret mode
		bool			caretPut(uint16_t c);			// " -

		inline int		eraseSelected();

		bool			setMaxLines(int maxLines);
		bool			setMaxChars(int maxChars);
		int				maxLines() const { return m_maxLines; }
		int				maxChars() const { return m_maxChars; }

		//.____ Control _____________________________________________

		// These methods will fail if editMode is Static

		bool			select(int begin, int end);			// Cursor (if enabled) gets end position. End can be smaller than begin.
		bool			selectAll();
		bool			unselect();

		inline bool		hasSelection() const { return m_editState.caretOfs != m_editState.selectOfs; }

		int				selectionBegin() const;				// Begin position of selection, might be after end position.
		int				selectionEnd() const;
		int				selectionSize() const;				// Number of characters that are selected.

		String			getSelection() const;

		// These methods will fail unless caret is present

		bool			setCaretOfs(int ofs);			// Move cursor to offset. Any selection will be unselected.
		int				caretOfs() const;

		bool			caretLineBegin();
		bool			caretLineEnd();
		bool			caretTextBegin();
		bool			caretTextEnd();
		void            requestFocus() { _requestFocus(true); }

		void			caretIntoView();			// Bring caret into view. Automatically done for all caret movements. 

		//.____ Internal ______________________________________________________

		void			_initFromBlueprint(const Blueprint& blueprint);
		void			_render(GfxDevice* pDevice, const RectSPX& _canvas) override;
		void			_receive(Msg* pMsg) override;
		void			_setState(State state) override;
	protected:

		// Overloaded so we can update caret and selection

		void			_clear() override;

		void			_setText(const CharSeq& seq) override;
		void			_setText(const CharBuffer * buffer) override;
		void			_setText(const String& str) override;

		int				_append(const CharSeq& seq) override;
		int				_insert(int ofs, const CharSeq& seq) override;
		int				_replace(int ofs, int nDelete, const CharSeq& seq) override;
		int				_erase(int ofs, int len) override;



		//

		bool			caretUp();
		bool			caretDown();
		bool			caretLeft();
		bool			caretRight();

		bool			caretPageUp();
		bool			caretPageDown();

		bool			caretNextWord();
		bool			caretPrevWord();

		bool			caretEraseNextChar();
		bool			caretErasePrevChar();
		bool			caretEraseNextWord();
		bool			caretErasePrevWord();

		bool			caretToPos(CoordSPX pos);			// Move caret as close as possible to the given position.
		bool			caretSelectWord();
		bool			caretSelectLine();				// Full line with hard line ending.

		//

		virtual bool	_caretVisible() const override { return m_editState.bCaret; }
		virtual int		_caretOffset() const override { return m_editState.caretOfs; }
		virtual std::tuple<int,int>	_selectedText() const override
		{
			int beg = std::min(m_editState.selectOfs,m_editState.caretOfs);
			int end = std::max(m_editState.selectOfs,m_editState.caretOfs);

			return std::make_tuple(beg,end);
		};

		//

		enum class MoveMethod
		{
			Keyboard,
			Mouse,
			ApiCall
		};


		bool			_moveCaret( int caretOfs, MoveMethod method );
		void			_updateDisplayArea();
		void			_updateInsertStyle();


		void			_caretToEnd();


		TextEditMode	m_editMode = TextEditMode::Editable;
		EditState		m_editState;
		int				m_maxLines = 0;
		int				m_maxChars = 0;
	};


} // namespace wg
#endif //WG_EDITABLETEXT_DOT_H
