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
#ifndef	WG_BASICTEXTLAYOUT_DOT_H
#define	WG_BASICTEXTLAYOUT_DOT_H
#pragma once

#include <wg_textlayout.h>
#include <wg_textstyle.h>
#include <wg_caret.h>

namespace wg
{

	class BasicTextLayout;
	typedef	StrongPtr<BasicTextLayout>		BasicTextLayout_p;
	typedef	WeakPtr<BasicTextLayout>	BasicTextLayout_wp;

	class BasicTextLayout : public TextLayout
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
            bool        autoEllipsis         = true;

			Caret_p		caret = nullptr;
			Finalizer_p	finalizer = nullptr;

			float		lineSpacing			= 1.f;
			float		paragraphSpacing	= 0.f;					// Add to lineSpacing at hard line endings.
			Placement	placement = Placement::NorthWest;

			BlendMode	selectionBackBlend	= BlendMode::Invert;
			HiColor		selectionBackColor	= HiColor::White;
			BlendMode	selectionCharBlend	= BlendMode::Invert;
			HiColor		selectionCharColor	= HiColor::White;

			int			tabWidth			= 8;					// Number of whitespace characters a tab corresonds to.
			bool		wrap = false;
		};

		//.____ Creation __________________________________________

		static BasicTextLayout_p create( const Blueprint& blueprint ) { return BasicTextLayout_p(new BasicTextLayout(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal __________________________________________

		virtual void	addText( TextItem * pText ) override;
		virtual void	removeText( TextItem * pText ) override;
		virtual void	didMoveText(TextItem* pFrom, TextItem* pToFrom) override;

		virtual int		charAtPos( const TextItem * pText, CoordSPX pos ) const override;
		virtual CoordSPX charPos( const TextItem * pText, int charOfs ) const override;	// Note: characters position on the baseline, not upper left corner of rectangle!
		virtual RectSPX	charRect( const TextItem * pText, int charOfs ) const override;
		virtual int		charLine( const TextItem * pText, int charOfs ) const override;

//		virtual int		charLineOfs( const TextItem * pText, int charOfs ) const;
//		virtual CoordI	lineTopLeftPos( const TextItem * pText, int line ) const;
//		virtual CoordI	lineBaselinePos( const TextItem * pText, int line ) const;
//		virtual int		lineWidth( const TextItem * pText, int line ) const;
//		virtual int		lineHeight( const TextItem * pText, int line ) const;
//		virtual RectI	lineGeo( const TextItem * pText, int line ) const;
//		virtual int		lines( const TextItem * pText ) const;

		virtual int		lineBegin( const TextItem * pText, int lineNb ) const override;
		virtual int		lineEnd( const TextItem * pText, int lineNb ) const override;

		virtual int		wordBegin( const TextItem * pText, int charOfs ) const override;
		virtual int		wordEnd( const TextItem * pText, int charOfs ) const override;

		virtual void 	render( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas ) override;

		virtual void	caretMove( TextItem * pText, int newOfs, int oldOfs ) override;
		virtual void	selectionChange( TextItem * pText, int newBeg, int newLen, int oldBeg, int oldLen ) override;

		virtual void	onTextModified( TextItem * pText, int ofs, int charsRemoved, int charsAdded ) override;
		virtual void	onResized( TextItem * pText, SizeSPX newSize, SizeSPX oldSize, int newScale, int oldScale ) override;
		virtual void	onStateChanged( TextItem * pText, State newState, State oldState ) override;
		virtual void	onStyleChanged( TextItem * pText, TextStyle * pNewStyle, TextStyle * pOldStyle ) override;
		virtual void	onCharStyleChanged( TextItem * pText, int ofs, int len ) override;


		virtual SizeSPX	defaultSize( const TextItem * pText, int scale ) const override;
		virtual spx		matchingWidth( const TextItem * pText, spx height, int scale ) const override;
		virtual spx		matchingHeight( const TextItem * pText, spx width, int scale ) const override;

		virtual RectSPX	rectForRange( const TextItem * pText, int ofs, int length ) const override;
		virtual RectSPX	rectForCaret( const TextItem * pText ) const override;
		virtual RectSPX	rectForCaretWithMargin( const TextItem * pText, int whitespacesBeforeAfter ) const override;

		virtual String 	tooltip( const TextItem * pText ) const override;

		virtual Direction 	textDirection( TextItem * pText, int charOfs ) const override;
		virtual int		caretToPos( TextItem * pText, CoordSPX pos, spx& wantedLineOfs ) const override;
		virtual int		caretUp( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		virtual int		caretDown( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		virtual int		caretLeft( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		virtual int		caretRight( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		virtual int		caretHome( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		virtual int		caretEnd( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;

		virtual int		caretPrevWord( TextItem * pText, int charOfs ) const override;
		virtual int		caretNextWord( TextItem * pText, int charOfs ) const override;



	protected:
		BasicTextLayout( const Blueprint& blueprint );
		virtual ~BasicTextLayout();


		struct BlockHeader
		{
			int nbLines;
			int scaleUsed;				// Scale used for calculating defaultSize and textSize.
			SizeSPX defaultSize;
			SizeSPX textSize;
			bool hasCharStyles;
		};

		struct LineInfo
		{
			int offset;				// Line start as offset in characters from beginning of text.
			int length;				// Length of line in characters, incl. line terminator,
			spx width;				// Width of line.
			spx height;				// Height of line.
			spx base;				// Offset for baseline from top of line.
			spx spacing;			// Offset from start of line to start of next line.
		};

		void			_refresh(TextItem * pText, bool bAllowRequestResize );
		void			_update(int microPassed, int64_t microsecTimestamp) override;

		inline void		_getGlyphWithoutBitmap( Font * pFont, uint16_t charCode, Glyph& glyph ) const;
		inline void		_getGlyphWithBitmap(Font* pFont, uint16_t charCode, Glyph& glyph) const;

		int				_countLines( TextItem * pText, const Char * pChars ) const;
		bool			_hasCharStyles( const Char * pChars ) const;

		int				_countFixedLines(const Char * pChars) const;
		int				_countWrapLines(const Char * pChars, const TextStyle * pBaseStyle, int scale, State state, spx maxLineWidth) const;
		spx				_calcMatchingHeight(const Char * pChars, const TextStyle * pBaseStyle, int scale, State state, spx maxLineWidth) const;

		void *			_reallocBlock( TextItem * pText, int lines, int scaleUsed, bool bHasCharStyles, SizeSPX defaultSize, SizeSPX textSize );

		void			_updateLineInfo(TextItem * pText, void * pBlock, const Char * pChars, bool bAllowRequestResize );

		SizeSPX			_updateFixedLineInfo(LineInfo * pLines, const Char * pChars, const TextStyle * pBaseStyle, int scale, State state );
		SizeSPX			_updateWrapLineInfo(LineInfo * pLines, const Char * pChars, const TextStyle * pBaseStyle, int scale, State state, spx maxLineWidth);
		SizeSPX			_calcDefaultSize(const Char * pChars, const TextStyle * pBaseStyle, int scale, State state ) const;


		spx				_charDistance( const Char * pFirst, const Char * pLast, const TextAttr& baseAttr, int scale, State state ) const;

		inline BlockHeader *		_header( void * pBlock ) { return static_cast<BlockHeader*>(pBlock); }
		inline const BlockHeader *	_header( const void * pBlock ) const { return static_cast<const BlockHeader*>(pBlock); }
		inline LineInfo *			_lineInfo( void * pBlock ) { return reinterpret_cast<LineInfo*>(&(((BlockHeader *) pBlock)[1])); }
		inline const LineInfo *		_lineInfo( const void * pBlock ) const { return reinterpret_cast<const LineInfo*>(&(((const BlockHeader *) pBlock)[1])); }

		spx				_linePosX( const LineInfo * pLine, spx canvasWidth ) const;
		spx				_linePosY( const void * pBlock, int line, spx canvasHeight ) const;
		spx				_textPosY( const BlockHeader * pHeader, spx canvasHeight ) const;
		spx				_charPosX( const TextItem * pText, int charOfs ) const;

		void 			_renderBack( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas );
		void 			_renderBackSection( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas,
											int begChar, int endChar, HiColor color );

		inline spx		_tabWidthSPX(spx whitespaceWidth, spx ofs) const
		{
			spx tabWidth = whitespaceWidth * m_tabWidth;
			return tabWidth == 0 ? 0 : tabWidth - (ofs % tabWidth);
		}

		enum struct SelectMode
		{
			Marked,					// Only select character/line that position is within.
			Closest,				// Select character/line that position is within or closest to.
			ClosestBegin,			// Select character/line whose beginning is closest to offset.
			ClosestEnd				// Select character/line whose end is closest to offset.
		};

		int				_lineAtPosY( const TextItem * pText, spx posY, SelectMode mode ) const;
		int				_charAtPosX( const TextItem * pText, int line, spx posX, SelectMode mode ) const;

		Placement		m_placement;
		Caret_p			m_pCaret;
		bool			m_bLineWrap = false;
		bool			m_bReceivingUpdates = false;

		HiColor			m_selectionBackColor;
		BlendMode		m_selectionBackBlend;

		HiColor			m_selectionCharColor;
		BlendMode		m_selectionCharBlend;

		float			m_hardLineSpacing = 1.f;
		float			m_softLineSpacing = 1.f;

		int				m_tabWidth = 8;

		TextItem *		m_pFocusedText = nullptr;
        bool            m_bAutoElipsis = false;

		const static int c_ellipsisCode = 57368;
	};



inline void BasicTextLayout::_getGlyphWithoutBitmap( Font * pFont, uint16_t charCode, Glyph& glyph ) const
{
	if( charCode <= 32 )
	{
		glyph.advance = 0;
		return;
	}
	else
	{
		pFont->getGlyphWithoutBitmap(charCode,glyph);
		if( glyph.advance == 0 )
		{
			pFont->getGlyphWithoutBitmap(0x25A1,glyph);			// White square character
			if( glyph.advance == 0 )
				pFont->getGlyphWithoutBitmap('?',glyph);
		}
	}
}

inline void BasicTextLayout::_getGlyphWithBitmap(Font* pFont, uint16_t charCode, Glyph& glyph) const
{
	if (charCode <= 32)
	{
		glyph.advance = 0;
		return;
	}
	else
	{
		pFont->getGlyphWithBitmap(charCode, glyph);
		if (glyph.advance == 0)
		{
			pFont->getGlyphWithBitmap(0x25A1, glyph);			// White square character
			if (glyph.advance == 0)
				pFont->getGlyphWithBitmap('?', glyph);
		}
	}
}


} // namespace wg
#endif //WG_BASICTEXTLAYOUT_DOT_H
