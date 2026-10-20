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
#ifndef	WG_FLOWTEXTLAYOUT_DOT_H
#define	WG_FLOWTEXTLAYOUT_DOT_H
#pragma once

#include <wg_textlayout.h>
#include <wg_textstyle.h>
#include <wg_caret.h>

#include <vector>

namespace wg
{
	class FlowTextLayout;
	typedef	StrongPtr<FlowTextLayout>	FlowTextLayout_p;
	typedef	WeakPtr<FlowTextLayout>		FlowTextLayout_wp;

	//____ FlowTextLayout __________________________________________________________
	/**
		Base class for text layouts that flow UTF-8 text into lines.

		FlowTextLayout does all the work of breaking text into lines, rendering it,
		hit-testing and caret navigation. Subclasses decide how the text is interpreted
		by implementing _parse(), which splits the text into runs with their own styles,
		marks markup that should be hidden and groups lines into indented blocks.

		Text not covered by any run is displayed in the base style of the text.
	*/

	class FlowTextLayout : public TextLayout
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal __________________________________________

		void	addText( TextItem * pText ) override;
		void	removeText( TextItem * pText ) override;
		void	didMoveText(TextItem* pFrom, TextItem* pTo) override;

		int		charAtPos( const TextItem * pText, CoordSPX pos ) const override;
		CoordSPX charPos( const TextItem * pText, int charOfs ) const override;
		RectSPX	charRect( const TextItem * pText, int charOfs ) const override;
		int		charLine( const TextItem * pText, int charOfs ) const override;
		int		lineBegin( const TextItem * pText, int lineNb ) const override;
		int		lineEnd( const TextItem * pText, int lineNb ) const override;

		int		wordBegin( const TextItem * pText, int charOfs ) const override;
		int		wordEnd( const TextItem * pText, int charOfs ) const override;

		void 	render( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas ) override;

		void	caretMove( TextItem * pText, int newOfs, int oldOfs ) override;
		void	selectionChange( TextItem * pText, int selectOfs, int caretOfs, int oldSelectOfs, int oldCaretOfs ) override;

		void	onTextModified( TextItem * pText, int ofs, int charsRemoved, int charsAdded ) override;
		void	onResized( TextItem * pText, SizeSPX newSize, SizeSPX oldSize, int newScale, int oldScale ) override;
		void	onStateChanged( TextItem * pText, State newState, State oldState ) override;
		void	onStyleChanged( TextItem * pText, TextStyle * pNewStyle, TextStyle * pOldStyle ) override;

		RectSPX	rectForRange( const TextItem * pText, int ofs, int length ) const override;
		RectSPX	rectForCaret( const TextItem * pText ) const override;
		RectSPX	rectForCaretWithMargin( const TextItem * pText, int whitespacesBeforeAfter ) const override;

		Direction 	textDirection( TextItem * pText, int charOfs ) const override;
		int			caretToPos( TextItem * pText, CoordSPX pos, spx& wantedLineOfs ) const override;
		int			caretUp( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		int			caretDown( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		int			caretLeft( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		int			caretRight( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		int			caretHome( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;
		int			caretEnd( TextItem * pText, int charOfs, spx& wantedLineOfs ) const override;

		int			caretPrevWord( TextItem * pText, int charOfs ) const override;
		int			caretNextWord( TextItem * pText, int charOfs ) const override;

		TextLink_p	linkAt( const TextItem * pText, int charOfs ) const override;

		SizeSPX		defaultSize( const TextItem * pText, int scale ) const override;
		spx			matchingWidth( const TextItem * pText, spx height, int scale ) const override;
		spx			matchingHeight( const TextItem * pText, spx width, int scale ) const override;

	protected:

		//____ Run ____________________________________________________________

		const static int c_maxRunStyles = 6;

		/**
			A section of the text with its own styling, produced by _parse().

			Runs are sorted, never overlap and only span a line break if substituted.
			The styles are added on top of the base style of the text, in order.

			Hidden runs contain markup that is not displayed. Substituted runs are
			displayed as a single character, like a bullet replacing a list marker.
			The caret can be placed at the beginning or end of a hidden or substituted run,
			but not within it.
		*/

		struct Run
		{
			int					begin = 0;					// Byte offset of first character.
			int					end = 0;					// Byte offset following last character.
			const TextStyle *	styles[c_maxRunStyles] = {};
			int					nbStyles = 0;
			bool				bHidden = false;
			char32_t			substitute = 0;				// Display run as this character instead, if not 0.
			TextLink_p			pLink;						// Link for the run, overrides links in the styles.

			void				addStyle( const TextStyle * pStyle ) { if( pStyle && nbStyles < c_maxRunStyles ) styles[nbStyles++] = pStyle; }
		};

		//____ Block __________________________________________________________

		/**
			A group of lines with their own indentation, produced by _parse().
			Blocks are sorted, never overlap and always begin at the start of a line.
		*/

		struct Block
		{
			int					begin = 0;					// Byte offset of first character of first line.
			int					end = 0;					// Byte offset following last line.
			pts					indent = 0;					// Indentation of all lines.
			pts					wrapIndent = 0;				// Additional indentation of lines wrapped by the layout.
		};

		//____ Markup _________________________________________________________

		struct Markup
		{
			std::vector<Run>	runs;
			std::vector<Block>	blocks;

			void				clear() { runs.clear(); blocks.clear(); }
		};

		//____ Settings _______________________________________________________

		struct Settings
		{
			bool		autoEllipsis		= true;
			Caret_p		caret				= nullptr;
			float		lineSpacing			= 1.f;
			float		paragraphSpacing	= 0.f;
			Placement	placement			= Placement::NorthWest;
			BlendMode	selectionBackBlend	= BlendMode::Invert;
			HiColor		selectionBackColor	= HiColor::White;
			BlendMode	selectionCharBlend	= BlendMode::Invert;
			HiColor		selectionCharColor	= HiColor::White;
			int			tabWidth			= 8;
			bool		wrap				= false;
		};

		FlowTextLayout( const Settings& settings );
		virtual ~FlowTextLayout();

		/**
			Interpret the text. Default implementation produces no runs or blocks,
			displaying the whole text in its base style.

			@param text		The UTF-8 encoded text to parse.
			@param markup	Empty markup to fill with sorted runs and blocks.
		*/

		virtual void		_parse( const std::string& text, Markup& markup ) const;

		void				_update(int microPassed, int64_t microsecTimestamp) override;

		//

		struct LineInfo
		{
			int		begin;			// Byte offset of first character of line.
			int		end;			// Byte offset following line, including line terminator. Last line ends at text length + 1.
			spx		width;			// Width of line.
			spx		height;			// Height of line.
			spx		base;			// Offset for baseline from top of line.
			spx		spacing;		// Offset from start of line to start of next line.
			spx		indent;			// Indentation from block.
		};

		struct TextData
		{
			Markup					markup;
			std::vector<LineInfo>	lines;
			int			scaleUsed = 0;			// Scale used for calculating defaultSize and textSize.
			SizeSPX		defaultSize = {-1,-1};
			SizeSPX		textSize = {-1,-1};
			spx			matchingHeight = -1;	// Cached for non-wrapping text, which doesn't depend on width. -1 until calculated.
		};

		class Cursor;

		inline TextData *		_data( TextItem * pText ) const { return static_cast<TextData*>(_dataBlock(pText)); }
		inline const TextData *	_data( const TextItem * pText ) const { return static_cast<const TextData*>(_dataBlock(pText)); }

		void			_refresh( TextItem * pText, bool bReparse, bool bAllowRequestResize );
		SizeSPX			_layoutLines( const TextItem * pText, const TextData * pData, int scale, State state, spx maxLineWidth, std::vector<LineInfo> * pLines ) const;

		Caret *			_caret() const;
		spx				_eolCaretWidth( Font * pFont, int scale ) const;

		const Run *		_runAt( const TextData * pData, int ofs ) const;
		const Block *	_blockAt( const TextData * pData, int ofs ) const;
		int				_skipHidden( const TextData * pData, int ofs, bool bForward ) const;
		int				_skipHiddenForward( const TextData * pData, int ofs ) const;
		int				_skipHiddenBackward( const TextData * pData, int ofs ) const;

		spx				_linePosX( const LineInfo * pLine, spx canvasWidth ) const;
		spx				_linePosY( const TextData * pData, int line, spx canvasHeight ) const;
		spx				_textPosY( const TextData * pData, spx canvasHeight ) const;
		spx				_charPosX( const TextItem * pText, int charOfs ) const;
		spx				_charDistance( const TextItem * pText, int lineBegin, int charOfs ) const;

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

		static bool		_isWhitespace( char32_t c ) { return c == ' ' || c == '\t' || c == 0xA0; }

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
		bool			m_bAutoEllipsis = false;
	};

} // namespace wg
#endif //WG_FLOWTEXTLAYOUT_DOT_H
