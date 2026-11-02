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
#ifndef	WG_TEXTLAYOUT_DOT_H
#define	WG_TEXTLAYOUT_DOT_H
#pragma once

#include <wax_pointers.h>
#include <wax_geo.h>
#include <wg_state.h>
#include <wg_textstyle.h>
#include <wg_textlink.h>

#include <string>
#include <wg_key.h>
#include <wg_receiver.h>

namespace wax
{
	class Device;
}

namespace wg
{
	class EditState;

	class TextStyle;

	//____ TextItem ___________________________________________________________
	/**
		Interface between a text component and its TextLayout.

		The text is UTF-8 encoded and may contain markup that the TextLayout
		interprets. All character offsets passed between TextItem and TextLayout
		are byte offsets into the text. The text is always followed by a
		virtual terminator at offset length(), which is a valid caret position.
	*/

	class TextItem
	{
		friend class TextLayout;

	private:
		virtual SizeSPX		_textSize() const = 0;
		virtual State		_textState() const = 0;
		virtual TextStyle * _textStyle() const = 0;
		
		virtual const std::string& _text() const = 0;

		virtual bool		_caretVisible() const = 0;
		virtual int			_caretOffset() const = 0;
		virtual std::tuple<int,int>	_selectedText() const = 0;		// Begin/end of selection

		virtual void		_mapperRequestRender() = 0;
		virtual void		_mapperRequestRender(const RectSPX& rect) = 0;
		virtual void		_mapperRequestResize() = 0;
		
		virtual int			_scale() const = 0;

		union
		{
			void *			m_pMapperData;
			int				m_mapperData;
		};
		
	};




	class TextLayout;
	typedef	StrongPtr<TextLayout>	TextLayout_p;
	typedef	WeakPtr<TextLayout>	TextLayout_wp;

	//____ TextLayout _________________________________________________________
	/**
		Base class for objects laying out, rendering and navigating texts.

		A TextLayout is shared between any number of texts. It is responsible for
		interpreting any markup in the text, deciding where lines break, rendering
		the text and translating between positions and character offsets.

		All character offsets are byte offsets into the UTF-8 text. A TextLayout only
		returns offsets on codepoint boundaries that are valid caret positions.
	*/

	class TextLayout : public Receiver
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal __________________________________________

		virtual void	addText( TextItem * pText ) = 0;
		virtual void	removeText( TextItem * pText ) = 0;
		virtual void	didMoveText(TextItem* pFrom, TextItem* pTo) = 0;

		virtual int		charAtPos( const TextItem * pText, CoordSPX pos ) const = 0;
		virtual CoordSPX charPos( const TextItem * pText, int charOfs ) const = 0;		// Note: characters position on the baseline, not upper left corner of rectangle!
		virtual RectSPX	charRect( const TextItem * pText, int charOfs ) const = 0;
		virtual int		charLine( const TextItem * pText, int charOfs ) const = 0;
		virtual int		lineBegin( const TextItem * pText, int lineNb ) const = 0;
		virtual int		lineEnd( const TextItem * pText, int lineNb ) const = 0;

		virtual int		wordBegin( const TextItem * pText, int charOfs ) const = 0;
		virtual int		wordEnd( const TextItem * pText, int charOfs ) const = 0;


		virtual void 	render( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas ) = 0;

		// Caret/selection update notification methods, only one of these needs to be called.
		// A selection change implies a caret move.

		virtual void	caretMove( TextItem * pText, int newOfs, int oldOfs ) = 0;
		virtual void	selectionChange( TextItem * pText, int selectOfs, int caretOfs, int oldSelectOfs, int oldCaretOfs ) = 0;

		//

		virtual void	onTextModified( TextItem * pText, int ofs, int charsRemoved, int charsAdded ) = 0;
		virtual void	onResized( TextItem * pText, SizeSPX newSize, SizeSPX oldSize, int newScale, int oldScale ) = 0;
		virtual void	onStateChanged( TextItem * pText, State newState, State oldState ) = 0;
		virtual void	onStyleChanged( TextItem * pText, TextStyle * pNewStyle, TextStyle * pOldStyle ) = 0;


		virtual SizeSPX	defaultSize( const TextItem * pText, int scale ) const = 0;
		virtual spx		matchingWidth( const TextItem * pText, spx height, int scale ) const = 0;
		virtual spx		matchingHeight( const TextItem * pText, spx width, int scale ) const = 0;

		virtual RectSPX	rectForRange( const TextItem * pText, int ofs, int length ) const = 0;
		virtual RectSPX	rectForCaret( const TextItem * pText ) const = 0;
		virtual RectSPX	rectForCaretWithMargin( const TextItem * pText, int whitespacesBeforeAfter ) const = 0;

		virtual std::string	tooltip( const TextItem * pText ) const;
		virtual TextLink_p	linkAt( const TextItem * pText, int charOfs ) const;		// Link at character, or nullptr.


		// Methods for supporting a caret

		virtual Direction 	textDirection( TextItem * pText, int charOfs ) const = 0;
		virtual int			caretToPos( TextItem * pText, CoordSPX pos, spx& wantedLineOfs ) const = 0;
		virtual int			caretUp( TextItem * pText, int charOfs, spx& wantedLineOfs ) const = 0;
		virtual int			caretDown( TextItem * pText, int charOfs, spx& wantedLineOfs ) const = 0;
		virtual int			caretLeft( TextItem * pText, int charOfs, spx& wantedLineOfs ) const = 0;
		virtual int			caretRight( TextItem * pText, int charOfs, spx& wantedLineOfs ) const = 0;
		virtual int			caretHome( TextItem * pText, int charOfs, spx& wantedLineOfs ) const = 0;
		virtual int			caretEnd( TextItem * pText, int charOfs, spx& wantedLineOfs ) const = 0;

		virtual int			caretPrevWord( TextItem * pText, int charOfs ) const = 0;
		virtual int			caretNextWord( TextItem * pText, int charOfs ) const = 0;


	protected:
		TextLayout() {}
		virtual ~TextLayout() {};

		SizeSPX				_size( const TextItem * pText ) const;
		int					_length( const TextItem * pText ) const;			// Length of text in bytes.
		int					_scale(const TextItem * pText) const;

		bool				_caretVisible( const TextItem * pText ) const;
		int					_caretOfs( const TextItem * pText ) const;
		std::tuple<int,int>	_selection( const TextItem * pText ) const;		// Begin/end of selection

		const std::string&	_text( const TextItem * pText ) const;
		const void *		_dataBlock( const TextItem * pText) const;
		void *				_dataBlock( TextItem * pText) const;
		int					_dataInt( const TextItem * pText ) const;


		const TextStyle *	_baseStyle( const TextItem * pText ) const;
		State				_state( const TextItem * pText ) const;

		void			_setTextDataBlock( TextItem * pText, void * pBlock );
		void			_setTextDataInt( TextItem * pText, int data );

		void			_setTextDirty( TextItem * pText );
		void			_setTextDirty( TextItem * pText, const RectSPX& rect );
		void			_requestTextResize( TextItem * pText );

	};





} // namespace wg
#endif //WG_TEXTLAYOUT_DOT_H
