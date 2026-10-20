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
#include <wg_flowtextlayout.h>
#include <wg_textstyle.h>
#include <wax_device.h>
#include <wg_base.h>
#include <wax_utf8.h>
#include <wg_util.h>

#include <algorithm>
#include <climits>

namespace wg
{
	using namespace Util;

	const TypeInfo FlowTextLayout::TYPEINFO = { "FlowTextLayout", &TextLayout::TYPEINFO };

	static const char32_t c_ellipsisCode		= 0x2026;		// Unicode HORIZONTAL ELLIPSIS.
	static const char32_t c_legacyEllipsisCode	= 0xE018;		// Ellipsis in private use area of WonderGUI's own fonts.
	static const char32_t c_missingGlyphCode	= 0x25A1;		// Unicode WHITE SQUARE.

	//____ _getGlyph() ________________________________________________________

	static void _getGlyph( Font * pFont, char32_t code, Glyph& glyph, bool bWithBitmap )
	{
		if( code <= 32 || code == 0xA0 )
		{
			glyph.advance = 0;
			glyph.kerningIndex = 0;
			glyph.fontRef = nullptr;
			glyph.pSurface = nullptr;
			return;
		}

		if( bWithBitmap )
		{
			pFont->getGlyphWithBitmap(code, glyph);
			if( glyph.advance == 0 )
			{
				pFont->getGlyphWithBitmap(c_missingGlyphCode, glyph);
				if( glyph.advance == 0 )
					pFont->getGlyphWithBitmap('?', glyph);
			}
		}
		else
		{
			pFont->getGlyphWithoutBitmap(code, glyph);
			if( glyph.advance == 0 )
			{
				pFont->getGlyphWithoutBitmap(c_missingGlyphCode, glyph);
				if( glyph.advance == 0 )
					pFont->getGlyphWithoutBitmap('?', glyph);
			}
		}
	}


	//____ Cursor _____________________________________________________________
	/*
		Walks through the text codepoint by codepoint, keeping track of the style
		and font in effect and the glyphs. This is the only place where styles of
		runs are resolved, so layout, rendering and hit-testing always agree.

		Usage: moveTo() a position, then call sync() before each advance().
		sync() applies the style for the current position and skips hidden runs.
	*/

	class FlowTextLayout::Cursor
	{
	public:
		Cursor( const FlowTextLayout * pLayout, const TextItem * pText, const TextData * pData, int scale, State state )
			: m_text(pLayout->_text(pText)), m_runs(pData->markup.runs), m_scale(scale), m_state(state)
		{
			pLayout->_baseStyle(pText)->exportAttr( state, &m_baseAttr, scale );
			m_length = int(m_text.size());
			m_pGlyph = &m_glyphs[0];
			m_pPrevGlyph = &m_glyphs[1];
		}

		void moveTo( int ofs )
		{
			m_pos = ofs;
			m_styleEnd = -1;							// Force sync() to resolve style.

			auto it = std::upper_bound( m_runs.begin(), m_runs.end(), ofs, [](int ofs, const Run& run) { return ofs < run.end; } );
			m_runIdx = int(it - m_runs.begin());

			m_pPrevGlyph->advance = 0;
			m_pPrevGlyph->fontRef = nullptr;
		}

		// Returns true if style changed.

		bool sync()
		{
			bool bChanged = false;

			while( m_pos >= m_styleEnd )
			{
				int nbRuns = int(m_runs.size());
				while( m_runIdx < nbRuns && m_runs[m_runIdx].end <= m_pos )
					m_runIdx++;

				const Run * pRun = nullptr;
				if( m_runIdx < nbRuns && m_runs[m_runIdx].begin <= m_pos )
				{
					pRun = &m_runs[m_runIdx];
					m_styleEnd = pRun->end;

					if( pRun->bHidden )
					{
						m_pos = pRun->end;
						continue;
					}
				}
				else
					m_styleEnd = m_runIdx < nbRuns ? m_runs[m_runIdx].begin : INT_MAX;

				if( pRun != m_pRun || !m_bInitialized )
				{
					TextAttr attr = m_baseAttr;
					if( pRun )
					{
						for( int i = 0 ; i < pRun->nbStyles ; i++ )
							pRun->styles[i]->addToAttr( m_state, &attr, m_scale );
					}

					if( attr.pFont != m_attr.pFont || attr.size != m_attr.size || !m_bInitialized )
					{
						attr.pFont->setSize(attr.size);
						m_pPrevGlyph->advance = 0;					// No kerning across different fonts or sizes.
						m_pPrevGlyph->fontRef = nullptr;
					}

					m_attr = attr;
					m_pRun = pRun;
					m_bInitialized = true;
					bChanged = true;
				}
			}
			return bChanged;
		}

		char32_t peek() const
		{
			if( m_pos >= m_length )
				return 0;

			if( m_pRun && m_pRun->substitute && m_pos == m_pRun->begin )
				return m_pRun->substitute;

			int ofs = m_pos;
			return Utf8::decode(m_text, ofs);
		}

		// Process codepoint at current position, updating x with its advance.

		void advance( spx& x, bool bWithBitmap, int tabWidth )
		{
			int ofs = m_pos;
			char32_t code;

			if( m_pRun && m_pRun->substitute && m_pos == m_pRun->begin )
			{
				code = m_pRun->substitute;
				ofs = m_pRun->end;						// Whole run is displayed as one character.
			}
			else
				code = Utf8::decode(m_text, ofs);

			Font * pFont = m_attr.pFont.rawPtr();
			_getGlyph( pFont, code, *m_pGlyph, bWithBitmap );

			if( m_pGlyph->advance > 0 )
			{
				m_glyphX = x + pFont->kerning(*m_pPrevGlyph, *m_pGlyph);
				x = m_glyphX + m_pGlyph->advance;
			}
			else
			{
				m_glyphX = x;
				if( code == ' ' || code == 0xA0 )
					x += pFont->whitespaceAdvance();
				else if( code == '\t' )
				{
					spx width = pFont->whitespaceAdvance() * tabWidth;
					if( width > 0 )
						x += width - (x % width);
				}
			}

			std::swap(m_pGlyph, m_pPrevGlyph);
			m_pos = ofs;
		}

		int				pos() const { return m_pos; }
		Font *			font() const { return m_attr.pFont.rawPtr(); }
		const TextAttr&	attr() const { return m_attr; }
		const Glyph&	glyph() const { return *m_pPrevGlyph; }			// Glyph of last advance().
		spx				glyphX() const { return m_glyphX; }			// Position of last glyph, kerning applied.

	private:
		const std::string&		m_text;
		const std::vector<Run>&	m_runs;
		int						m_length;
		int						m_scale;
		State					m_state;

		TextAttr				m_baseAttr;
		TextAttr				m_attr;
		const Run *				m_pRun = nullptr;
		bool					m_bInitialized = false;

		int						m_pos = 0;
		int						m_runIdx = 0;
		int						m_styleEnd = -1;

		Glyph					m_glyphs[2];
		Glyph *					m_pGlyph;
		Glyph *					m_pPrevGlyph;
		spx						m_glyphX = 0;
	};


	//____ constructor _____________________________________________________________

	FlowTextLayout::FlowTextLayout(const Settings& bp)
	{
		m_pCaret				= bp.caret;
		m_placement				= bp.placement;

		m_selectionBackBlend	= bp.selectionBackBlend;
		m_selectionBackColor	= bp.selectionBackColor;
		m_selectionCharBlend	= bp.selectionCharBlend;
		m_selectionCharColor	= bp.selectionCharColor;

		m_softLineSpacing		= bp.lineSpacing;
		m_hardLineSpacing		= bp.lineSpacing + bp.paragraphSpacing;
		m_bAutoEllipsis			= bp.autoEllipsis;

		m_bLineWrap				= bp.wrap;
		m_tabWidth				= bp.tabWidth;
	}

	//____ Destructor ______________________________________________________________

	FlowTextLayout::~FlowTextLayout()
	{
		if( m_bReceivingUpdates )
		{
			Base::_stopReceiveUpdates(this);
			m_bReceivingUpdates = false;
		}
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FlowTextLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ addText() _________________________________________________________

	void FlowTextLayout::addText( TextItem * pText )
	{
		auto pData = new TextData();
		_setTextDataBlock(pText, pData);

		_parse( _text(pText), pData->markup );

		int scale = _scale(pText);
		State state = _state(pText);

		if( m_bLineWrap )
		{
			pData->defaultSize = _layoutLines( pText, pData, scale, state, -1, nullptr );
			pData->textSize = _layoutLines( pText, pData, scale, state, _size(pText).w, &pData->lines );
		}
		else
		{
			pData->defaultSize = _layoutLines( pText, pData, scale, state, -1, &pData->lines );
			pData->textSize = pData->defaultSize;
		}
		pData->scaleUsed = scale;

		_requestTextResize(pText);
	}

	//____ removeText() _________________________________________________________

	void FlowTextLayout::removeText( TextItem * pText )
	{
		delete _data(pText);
		_setTextDataBlock(pText, nullptr);

		if( pText == m_pFocusedText )
		{
			m_pFocusedText = nullptr;
			if( m_bReceivingUpdates )
			{
				Base::_stopReceiveUpdates(this);
				m_bReceivingUpdates = false;
			}
		}
	}

	//____ didMoveText() _________________________________________________________

	void FlowTextLayout::didMoveText(TextItem* pFrom, TextItem* pTo)
	{
		if( m_pFocusedText == pFrom )
			m_pFocusedText = pTo;
	}

	//____ _parse() ___________________________________________________________

	void FlowTextLayout::_parse( const std::string& text, Markup& markup ) const
	{
		// Plain text, nothing to do.
	}

	//____ charAtPos() _________________________________________________________

	int FlowTextLayout::charAtPos( const TextItem * pText, CoordSPX pos ) const
	{
		int line = _lineAtPosY(pText, pos.y, SelectMode::Marked );
		if( line == -1 )
			return -1;

		return  _charAtPosX(pText, line, pos.x, SelectMode::Marked );
	}

	//_____ charPos() ______________________________________________________

	CoordSPX FlowTextLayout::charPos( const TextItem * pText, int charOfs ) const
	{
		int line = charLine(pText, charOfs);
		if( line < 0 )
			return CoordSPX();

		const TextData * pData = _data(pText);

		spx ofsX = _charPosX(pText, charOfs);
		spx ofsY = _linePosY(pData, line, _size(pText).h ) + pData->lines[line].base;

		return CoordSPX(ofsX,ofsY);
	}

	//____ charRect() ________________________________________________________

	RectSPX FlowTextLayout::charRect( const TextItem * pText, int charOfs ) const
	{
		int line = charLine(pText, charOfs);
		if( line < 0 )
			return RectSPX();

		const TextData * pData = _data(pText);
		const LineInfo * pLine = &pData->lines[line];

		spx lineX = _charDistance(pText, pLine->begin, charOfs);
		spx xOfs = _linePosX( pLine, _size(pText).w ) + lineX;
		spx yOfs = _linePosY( pData, line, _size(pText).h );

		// Get cell width

		spx width = 0;

		if( charOfs < _length(pText) )
		{
			Cursor cursor( this, pText, pData, _scale(pText), _state(pText) );
			cursor.moveTo(charOfs);
			cursor.sync();

			if( cursor.pos() == charOfs && cursor.peek() != '\n' )
			{
				spx x = lineX;
				cursor.advance(x, false, m_tabWidth);
				width = x - lineX;
			}
		}

		return RectSPX(xOfs,yOfs,width,pLine->height);
	}

	//____ charLine() ________________________________________________________

	int FlowTextLayout::charLine( const TextItem * pText, int charOfs ) const
	{
		if( charOfs < 0 )
			return -1;

		const auto& lines = _data(pText)->lines;

		auto it = std::upper_bound( lines.begin(), lines.end(), charOfs, [](int ofs, const LineInfo& line) { return ofs < line.end; } );
		if( it == lines.end() )
			return -1;

		return int(it - lines.begin());
	}

	//____ lineBegin() ________________________________________________________

	int FlowTextLayout::lineBegin( const TextItem * pText, int lineNb ) const
	{
		const auto& lines = _data(pText)->lines;

		if( lineNb < 0 || lineNb >= int(lines.size()) )
			return -1;

		return lines[lineNb].begin;
	}

	//____ lineEnd() ___________________________________________________________

	int FlowTextLayout::lineEnd( const TextItem * pText, int lineNb ) const
	{
		const auto& lines = _data(pText)->lines;

		if( lineNb < 0 || lineNb >= int(lines.size()) )
			return -1;

		return lines[lineNb].end;
	}

	//____ wordBegin() _________________________________________________________

	int FlowTextLayout::wordBegin( const TextItem * pText, int charOfs ) const
	{
		const std::string& text = _text(pText);

		while( charOfs > 0 )
		{
			int prev = Utf8::prev(text, charOfs);
			char32_t c = Utf8::codepointAt(text, prev);
			if( _isWhitespace(c) || c == '\n' )
				break;
			charOfs = prev;
		}
		return charOfs;
	}

	//____ wordEnd() ___________________________________________________________

	int FlowTextLayout::wordEnd( const TextItem * pText, int charOfs ) const
	{
		const std::string& text = _text(pText);
		int len = int(text.size());

		while( charOfs < len )
		{
			char32_t c = Utf8::codepointAt(text, charOfs);
			if( _isWhitespace(c) || c == '\n' )
				break;
			charOfs = Utf8::next(text, charOfs);
		}
		return charOfs;
	}

	//____ _charDistance() _____________________________________________________

	// Returns distance between beginning of line and beginning of character at charOfs.

	spx FlowTextLayout::_charDistance( const TextItem * pText, int lineBegin, int charOfs ) const
	{
		Cursor cursor( this, pText, _data(pText), _scale(pText), _state(pText) );
		cursor.moveTo(lineBegin);

		int len = _length(pText);
		spx distance = 0;

		while( true )
		{
			cursor.sync();
			if( cursor.pos() >= charOfs || cursor.pos() >= len || cursor.peek() == '\n' )
				break;

			cursor.advance(distance, false, m_tabWidth);
		}

		return distance;
	}

	//____ _update() ___________________________________________________________

	void FlowTextLayout::_update(int microPassed, int64_t microsecTimestamp)
	{
		if( m_pFocusedText && _caretVisible(m_pFocusedText) )
		{
			Caret * pCaret = _caret();
			if( pCaret )
			{
				int ms = microPassed / 1000;

				bool bDirty = pCaret->tick( ms );
				if( bDirty )
				{
					int caretOfs = _caretOfs(m_pFocusedText);
					_setTextDirty( m_pFocusedText, pCaret->dirtyRect(charRect(m_pFocusedText, caretOfs), _scale(m_pFocusedText)) );
				}
			}
		}
	}

	//____ render()___________________________________________________________

	void FlowTextLayout::render( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas )
	{
		int		scale = _scale(pText);
		State	state = _state(pText);
		int		textLen = _length(pText);

		TextData * pData = _data(pText);

		CoordSPX lineStart = canvas.pos();
		lineStart.y += _textPosY( pData, canvas.h );

		TextAttr		baseAttr;
		_baseStyle(pText)->exportAttr( state, &baseAttr, scale );

		HiColor	baseTint = pDevice->tintColor();
		HiColor	localTint = HiColor::White;

		BlendMode renderMode = pDevice->blendMode();

		// Limit our cliplist if needed

		bool bOverflow = pData->textSize.w > canvas.w || pData->textSize.h > canvas.h;

		ClipPopData popData;
		if( bOverflow )
			popData = limitClipList(pDevice, canvas);

		// Prepare ellipsis

		Glyph	ellipsisGlyph;
		int		nbEllipsisGlyphs = 0;
		spx		ellipsisLength = 0;
		bool 	bNeedsEllipsis = false;

		if( m_bAutoEllipsis && bOverflow && !_caretVisible(pText) )
		{
			bNeedsEllipsis = true;

			Font * pFont = baseAttr.pFont.rawPtr();
			pFont->setSize(baseAttr.size);

			if( pFont->hasGlyph(c_ellipsisCode) )
			{
				pFont->getGlyphWithBitmap(c_ellipsisCode, ellipsisGlyph);
				nbEllipsisGlyphs = 1;
			}
			else if( pFont->hasGlyph(c_legacyEllipsisCode) )
			{
				pFont->getGlyphWithBitmap(c_legacyEllipsisCode, ellipsisGlyph);
				nbEllipsisGlyphs = 1;
			}
			else
			{
				pFont->getGlyphWithBitmap('.', ellipsisGlyph);
				nbEllipsisGlyphs = 3;
			}
			ellipsisLength = ellipsisGlyph.advance * nbEllipsisGlyphs;
		}

		// Render back colors

		_renderBack( pText, pDevice, canvas );

		// Render selection background

		int selBeg, selEnd;
		std::tie(selBeg,selEnd) = _selection(pText);

		if( selBeg != selEnd && m_selectionBackBlend != BlendMode::Ignore )
		{
			if( m_selectionBackBlend != BlendMode::Undefined )
				pDevice->setBlendMode( m_selectionBackBlend );
			_renderBackSection( pText, pDevice, canvas, selBeg, selEnd, m_selectionBackColor );
			pDevice->setBlendMode( renderMode );
		}

		// Render the lines

		const RectSPX& clip = pDevice->clipBounds();

		Cursor	cursor( this, pText, pData, scale, state );
		bool	bInSelection = false;
		bool	bRecalcColor = true;

		struct DecorationLine				// Underline or strikethrough.
		{
			spx		begin;
			spx		end;
			spx		y;
			spx		thickness;
			HiColor	color;
		};

		std::vector<DecorationLine> decorationLines;

		int nbLines = int(pData->lines.size());
		for( int i = 0 ; i < nbLines ; i++ )
		{
			const LineInfo * pLine = &pData->lines[i];

			if( lineStart.y < clip.y + clip.h && lineStart.y + pLine->height > clip.y )
			{
				lineStart.x = canvas.x + _linePosX( pLine, canvas.w );

				bool bLineHasEllipsis = false;
				if( bNeedsEllipsis )
				{
					if( pLine->width > canvas.w )
						bLineHasEllipsis = true;
					else if( m_bLineWrap )
					{
						if( i+1 < nbLines && lineStart.y + pLine->spacing + pLine[1].height > clip.y + clip.h )
						{
							bLineHasEllipsis = true;

							// With lineWrap we readjust text for line that gets ellipsis since that might make it all fit.

							lineStart.x = std::max( canvas.x, canvas.x + _linePosX( pLine, canvas.w - ellipsisLength ));
						}
						else if( lineStart.y + pLine->spacing + pLine->height > clip.y + clip.h )
						{
							// Prevent partially displayed line from being displayed.
							// We might get here if previous line with ellipsis is skipped due to clipping.

							break;
						}
					}
				}

				spx		baseline = lineStart.y + pLine->base;
				spx		lineX = 0;							// Position relative to start of line.
				spx		ellipsisX = 0;

				decorationLines.clear();

				cursor.moveTo(pLine->begin);

				while( true )
				{
					if( cursor.sync() )
					{
						localTint = cursor.attr().color;
						bRecalcColor = true;
					}

					int charOfs = cursor.pos();
					if( charOfs >= pLine->end || charOfs >= textLen || cursor.peek() == '\n' )
						break;

					bool bSelected = charOfs >= selBeg && charOfs < selEnd;
					if( bSelected != bInSelection )
					{
						bInSelection = bSelected;
						bRecalcColor = true;
					}

					if( bRecalcColor )
					{
						if (bInSelection)
							pDevice->setTintColor(baseTint * HiColor::blend(localTint, m_selectionCharColor, m_selectionCharBlend));
						else
							pDevice->setTintColor( baseTint * localTint );

						bRecalcColor = false;
					}

					spx x = lineX;
					cursor.advance(x, true, m_tabWidth);

					const Glyph& glyph = cursor.glyph();
					if( glyph.advance > 0 )
					{
						if( bLineHasEllipsis && lineX + glyph.advance + ellipsisLength > canvas.w - (lineStart.x - canvas.x) )
							break;

						if( glyph.pSurface )
						{
							pDevice->setBlitSource(glyph.pSurface);
							pDevice->blit( CoordSPX(lineStart.x + cursor.glyphX() + glyph.bearingX, baseline + glyph.bearingY), glyph.rect );
						}

						ellipsisX = x;
					}

					TextDecoration decoration = cursor.attr().decoration;

					if( (decoration == TextDecoration::Underline || decoration == TextDecoration::Strikethrough) && x > lineX )
					{
						const TextAttr& attr = cursor.attr();

						HiColor color = bInSelection ? HiColor::blend(localTint, m_selectionCharColor, m_selectionCharBlend) : localTint;
						spx thickness = std::max( spx(64), align(attr.size / 16) );

						spx y;
						if( decoration == TextDecoration::Underline )
							y = baseline + std::max( spx(64), align(attr.size / 10) );
						else
							y = baseline - align(attr.size * 3 / 10 + thickness / 2);		// Through middle of lowercase letters.

						if( !decorationLines.empty() && decorationLines.back().end == lineX && decorationLines.back().color == color && decorationLines.back().y == y )
							decorationLines.back().end = x;
						else
							decorationLines.push_back( { lineX, x, y, thickness, color } );
					}

					lineX = x;
				}

				// Draw underlines and strikethroughs

				if( !decorationLines.empty() )
				{
					pDevice->setTintColor( baseTint );
					for( auto& d : decorationLines )
						pDevice->fill( RectSPX( lineStart.x + d.begin, d.y, d.end - d.begin, d.thickness ), d.color );
					bRecalcColor = true;
				}

				if( bLineHasEllipsis )
				{
					pDevice->setTintColor( baseTint * baseAttr.color );
					bRecalcColor = true;

					if( ellipsisGlyph.pSurface )
					{
						pDevice->setBlitSource(ellipsisGlyph.pSurface);

						spx x = lineStart.x + ellipsisX;
						for( int n = 0 ; n < nbEllipsisGlyphs ; n++ )
						{
							pDevice->blit( CoordSPX(x + ellipsisGlyph.bearingX, baseline + ellipsisGlyph.bearingY), ellipsisGlyph.rect );
							x += ellipsisGlyph.advance;
						}
					}

					// In linewrap mode we need to prevent us from drawing an extra half-hidden line below our ellipsis

					if( m_bLineWrap )
						break;
				}
			}

			lineStart.y += pLine->spacing;
		}

		pDevice->setTintColor( baseTint );

		// Render caret (if there is any)

		if( _caretVisible(pText) )
		{
			Caret * pCaret = _caret();
			if( pCaret )
				pCaret->render( pDevice, charRect(pText, _caretOfs(pText)) + canvas.pos(), scale );
		}

		// Pop any changes to cliplist.

		popClipList(pDevice, popData);
	}

	//____ _renderBack()___________________________________________________________

	void FlowTextLayout::_renderBack( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas )
	{
		const TextData * pData = _data(pText);
		State state = _state(pText);
		int textLen = _length(pText);

		HiColor baseColor = _baseStyle(pText)->backColor(state);
		if( baseColor == HiColor::Undefined )
			baseColor = HiColor::Transparent;

		if( pData->markup.runs.empty() )
		{
			if( baseColor.a != 0 )
				_renderBackSection( pText, pDevice, canvas, 0, textLen, baseColor );
			return;
		}

		// Render sections of text with different background colors, merging neighbours with same color.

		HiColor	sectionColor = baseColor;
		int		sectionBegin = 0;

		auto addSection = [&](int begin, HiColor color)
		{
			if( color == sectionColor )
				return;

			if( sectionColor.a != 0 && begin > sectionBegin )
				_renderBackSection( pText, pDevice, canvas, sectionBegin, begin, sectionColor );

			sectionColor = color;
			sectionBegin = begin;
		};

		for( auto& run : pData->markup.runs )
		{
			HiColor color = baseColor;
			for( int i = 0 ; i < run.nbStyles ; i++ )
			{
				HiColor c = run.styles[i]->backColor(state);
				if( c != HiColor::Undefined )
					color = c;
			}

			addSection( run.begin, color );
			addSection( run.end, baseColor );
		}

		if( sectionColor.a != 0 && textLen > sectionBegin )
			_renderBackSection( pText, pDevice, canvas, sectionBegin, textLen, sectionColor );
	}

	//____ _renderBackSection() ________________________________________________

	void FlowTextLayout::_renderBackSection( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas,
											int begChar, int endChar, HiColor color )
	{
		RectSPX area = rectForRange( pText, begChar, endChar - begChar );

		int begLine = charLine( pText, begChar );
		int endLine = charLine( pText, endChar );

		if( begLine < 0 || endLine < 0 )
			return;

		if( begLine == endLine )
		{
			pDevice->fill( area + canvas.pos(), color );
			return;
		}

		// Spanning multiple lines, fill each line separately.

		const TextData * pData = _data(pText);

		CoordSPX begPos = charPos( pText, begChar );
		CoordSPX endPos = charPos( pText, endChar );

		const LineInfo * pLine = &pData->lines[begLine];
		spx yPos = canvas.y + begPos.y - pLine->base;

		RectSPX fill;
		fill.x = canvas.x + begPos.x;
		fill.y = yPos;
		fill.w = pLine->width - (begPos.x - _linePosX( pLine, canvas.w));
		fill.h = pLine->height;
		pDevice->fill( fill, color );

		yPos += pLine->spacing;

		for( int line = begLine + 1 ; line < endLine ; line++ )
		{
			pLine = &pData->lines[line];

			fill.x = canvas.x + _linePosX( pLine, canvas.w);
			fill.y = yPos;
			fill.w = pLine->width;
			fill.h = pLine->height;
			pDevice->fill( fill, color );

			yPos += pLine->spacing;
		}

		pLine = &pData->lines[endLine];
		spx lineX = _linePosX( pLine, canvas.w);

		fill.x = canvas.x + lineX;
		fill.y = yPos;
		fill.w = endPos.x - lineX;
		fill.h = pLine->height;
		pDevice->fill( fill, color );
	}

	//____ caretMove() ________________________________________________________

	void FlowTextLayout::caretMove( TextItem * pText, int newOfs, int oldOfs )
	{
		Caret * pCaret = _caret();
		if( _caretVisible(pText) && pCaret )
		{
			bool bDirty = pCaret->restartCycle();

			if( bDirty || oldOfs != newOfs )
			{
				_setTextDirty( pText, pCaret->dirtyRect( charRect(pText, newOfs), _scale(pText) ));

				if( oldOfs != newOfs )
					_setTextDirty( pText, pCaret->dirtyRect( charRect(pText, oldOfs), _scale(pText)) );
			}
		}
	}

	//____ selectionChange() __________________________________________________

	void FlowTextLayout::selectionChange( TextItem * pText, int selectOfs, int caretOfs, int oldSelectOfs, int oldCaretOfs )
	{
		int oldBeg = std::min(oldSelectOfs,oldCaretOfs);
		int oldEnd = std::max(oldSelectOfs,oldCaretOfs);

		int newBeg = std::min(selectOfs,caretOfs);
		int newEnd = std::max(selectOfs,caretOfs);

		// Two cases here, either new and old selection overlaps or they are completely disjointed

		if( (newBeg < oldBeg && newEnd > oldBeg) || (newBeg >= oldBeg && oldEnd > newBeg) )
		{
			//  Selection is overlapping, we just need to modify it at the edges

			if( oldBeg != newBeg )
			{
				int beg = std::min(oldBeg,newBeg);
				int len = std::max(oldBeg,newBeg) - beg;

				_setTextDirty( pText, rectForRange( pText, beg, len ) );
			}

			if( oldEnd != newEnd )
			{
				int beg = std::min(oldEnd,newEnd);
				int len = std::max(oldEnd,newEnd) - beg;

				_setTextDirty( pText, rectForRange( pText, beg, len ) );
			}
		}
		else
		{
			// Selection is not overlapping, we have two dirty areas

			if( oldEnd != oldBeg )
				_setTextDirty( pText, rectForRange( pText, oldBeg, oldEnd - oldBeg ));
			if( newEnd != newBeg )
				_setTextDirty( pText, rectForRange( pText, newBeg, newEnd - newBeg ));
		}

		// Update/redraw the caret

		caretMove( pText, caretOfs, oldCaretOfs );
	}

	//____ onTextModified() ____________________________________________________

	void FlowTextLayout::onTextModified( TextItem * pText, int ofs, int charsRemoved, int charsAdded )
	{
		_refresh(pText, true, true);
	}

	//____ onResized() ___________________________________________________________

	void FlowTextLayout::onResized( TextItem * pText, SizeSPX newSize, SizeSPX oldSize, int newScale, int oldScale )
	{
		if (m_bLineWrap || newScale != oldScale)
			_refresh(pText, false, false);
	}

	//____ onStateChanged() ______________________________________________________

	void FlowTextLayout::onStateChanged( TextItem * pText, State newState, State oldState )
	{
		// TODO: Support for more than one input device, focusing different (or same) texts.

		if( newState.isFocused() != oldState.isFocused() )
		{
			if( newState.isFocused() )
			{
				m_pFocusedText = pText;
				if (!m_bReceivingUpdates)
				{
					Base::_startReceiveUpdates(this);
					m_bReceivingUpdates = true;
				}
			}
			else
			{
				m_pFocusedText = nullptr;
				if( m_bReceivingUpdates )
				{
					Base::_stopReceiveUpdates(this);
					m_bReceivingUpdates = false;
				}
			}
		}

		// Check if the change affects size or appearance of the text.

		TextData * pData = _data(pText);
		pData->matchingHeight = -1;

		bool bIdentical = true;
		bool bSizeChanged = false;

		auto checkStyle = [&](const TextStyle * pStyle)
		{
			if( !pStyle->isStateIdentical(newState, oldState) )
			{
				bIdentical = false;
				if( pStyle->size(newState) != pStyle->size(oldState) )
					bSizeChanged = true;
			}
		};

		checkStyle( _baseStyle(pText) );

		for( auto& run : pData->markup.runs )
			for( int i = 0 ; i < run.nbStyles ; i++ )
				checkStyle( run.styles[i] );

		if( bSizeChanged )
			_refresh(pText, false, true);
		else if( !bIdentical )
			_setTextDirty(pText);
	}

	//____ onStyleChanged() ______________________________________________________

	void FlowTextLayout::onStyleChanged( TextItem * pText, TextStyle * pNewStyle, TextStyle * pOldStyle )
	{
		_refresh(pText, false, true);
	}

	//____ _refresh() ___________________________________________________________

	void FlowTextLayout::_refresh( TextItem * pText, bool bReparse, bool bAllowRequestResize )
	{
		TextData * pData = _data(pText);

		if( bReparse )
		{
			pData->markup.clear();
			_parse( _text(pText), pData->markup );
		}

		int scale = _scale(pText);
		State state = _state(pText);

		SizeSPX defaultSize;
		SizeSPX textSize;

		if( m_bLineWrap )
		{
			defaultSize = _layoutLines( pText, pData, scale, state, -1, nullptr );
			textSize = _layoutLines( pText, pData, scale, state, _size(pText).w, &pData->lines );
		}
		else
		{
			defaultSize = _layoutLines( pText, pData, scale, state, -1, &pData->lines );
			textSize = defaultSize;
		}

		pData->scaleUsed = scale;
		pData->matchingHeight = -1;

		if (defaultSize != pData->defaultSize || textSize != pData->textSize)
		{
			pData->textSize = textSize;
			pData->defaultSize = defaultSize;
			if( bAllowRequestResize )
				_requestTextResize(pText);
		}

		_setTextDirty(pText);
	}

	//____ _layoutLines() ______________________________________________________
	/*
		Breaks text into lines and calculates its size. Lines are wrapped if
		maxLineWidth >= 0. Line info is written to pLines if not null.
	*/

	SizeSPX FlowTextLayout::_layoutLines( const TextItem * pText, const TextData * pData, int scale, State state,
										  spx maxLineWidth, std::vector<LineInfo> * pLines ) const
	{
		bool bWrap = maxLineWidth >= 0;
		const std::string& text = _text(pText);
		int textLen = int(text.size());

		if( pLines )
			pLines->clear();

		Cursor cursor( this, pText, pData, scale, state );

		SizeSPX	size;

		spx		maxAscend, maxDescend, maxDescendGap;		// maxDescendGap includes the line gap.
		spx		eolCaretWidth;

		spx		bpMaxAscend = 0, bpMaxDescend = 0, bpMaxDescendGap = 0;
		int		breakpoint = -1;
		spx		bpWidth = 0;

		int		lineBegin = 0;
		spx		indent = 0;							// Indentation of line from its block.
		spx		width = 0;							// Width up to end of last glyph.
		spx		potentialWidth = 0;					// Width including trailing whitespace.

		auto startLine = [&](int ofs, bool bWrapped)
		{
			lineBegin = ofs;

			const Block * pBlock = _blockAt(pData, ofs);
			indent = pBlock ? ptsToSpx( pBlock->indent + (bWrapped ? pBlock->wrapIndent : 0), scale ) : 0;

			cursor.moveTo(ofs);
			cursor.sync();

			Font * pFont = cursor.font();
			maxAscend = pFont->maxAscend();
			maxDescend = pFont->maxDescend();
			maxDescendGap = maxDescend + pFont->lineGap();
			eolCaretWidth = _eolCaretWidth(pFont, scale);

			width = 0;
			potentialWidth = 0;
			breakpoint = -1;
		};

		auto finishLine = [&](int end, spx lineWidth, bool bHardBreak, bool bLastLine)
		{
			spx height = maxAscend + maxDescend;
			spx spacing = align( (maxAscend + maxDescendGap) * (bHardBreak ? m_hardLineSpacing : m_softLineSpacing) );

			if( pLines )
				pLines->push_back( { lineBegin, end, lineWidth, height, maxAscend, spacing, indent } );

			if( indent + lineWidth > size.w )
				size.w = indent + lineWidth;

			size.h += bLastLine ? height : spacing;
		};

		startLine(0, false);

		while( true )
		{
			if( cursor.sync() )
			{
				Font * pFont = cursor.font();

				maxAscend = std::max( maxAscend, pFont->maxAscend() );
				maxDescend = std::max( maxDescend, pFont->maxDescend() );
				maxDescendGap = std::max( maxDescendGap, pFont->maxDescend() + pFont->lineGap() );
				eolCaretWidth = _eolCaretWidth(pFont, scale);
			}

			int charOfs = cursor.pos();

			// Handle end of text

			if( charOfs >= textLen )
			{
				finishLine( textLen + 1, width + eolCaretWidth, true, true );
				break;
			}

			// Handle hard line break

			char32_t code = cursor.peek();

			if( code == '\n' )
			{
				finishLine( charOfs + 1, width + eolCaretWidth, true, false );
				startLine( charOfs + 1, false );
				continue;
			}

			// Add character

			spx preGlyphWidth = width;

			cursor.advance(potentialWidth, false, m_tabWidth);
			if( cursor.glyph().advance > 0 )
				width = potentialWidth;

			// Handle soft line break

			if( bWrap && width > maxLineWidth - indent - eolCaretWidth )
			{
				int lineEnd;

				if( breakpoint >= 0 )
				{
					lineEnd = breakpoint;
					width = bpWidth;
					maxAscend = bpMaxAscend;
					maxDescend = bpMaxDescend;
					maxDescendGap = bpMaxDescendGap;
				}
				else if( preGlyphWidth > 0 )
				{
					lineEnd = charOfs;
					width = preGlyphWidth;
				}
				else
				{
					// Glyph wider than line, we have to keep it anyway. Trailing whitespace
					// takes no space so we keep that too instead of starting next line with it.

					lineEnd = cursor.pos();
					while( lineEnd < textLen && (text[lineEnd] == ' ' || text[lineEnd] == '\t') )
						lineEnd++;
				}

				// No soft break if line ends here anyway.

				if( lineEnd < textLen && text[lineEnd] != '\n' )
				{
					finishLine( lineEnd, width + eolCaretWidth, false, false );
					startLine( lineEnd, true );
				}
			}
			else if( code == ' ' || code == '\t' )
			{
				breakpoint = cursor.pos();
				bpWidth = width;
				bpMaxAscend = maxAscend;
				bpMaxDescend = maxDescend;
				bpMaxDescendGap = maxDescendGap;
			}
		}

		return alignUp(size);
	}

	//____ _eolCaretWidth() ____________________________________________________

	spx FlowTextLayout::_eolCaretWidth( Font * pFont, int scale ) const
	{
		Caret * pCaret = _caret();
		if( !pCaret )
			return 0;

		SizeSPX eolCellSize(1, pFont->maxAscend() + pFont->maxDescend());
		spx w = pCaret->eolWidth(eolCellSize, scale);
		return alignUp(std::max(w, eolCellSize.w));
	}

	//____ _caret() ____________________________________________________________

	Caret * FlowTextLayout::_caret() const
	{
		return m_pCaret ? m_pCaret.rawPtr() : Base::defaultCaret().rawPtr();
	}

	//____ _runAt() ____________________________________________________________

	const FlowTextLayout::Run * FlowTextLayout::_runAt( const TextData * pData, int ofs ) const
	{
		const auto& runs = pData->markup.runs;
		auto it = std::upper_bound( runs.begin(), runs.end(), ofs, [](int ofs, const Run& run) { return ofs < run.end; } );

		if( it != runs.end() && it->begin <= ofs )
			return &(*it);

		return nullptr;
	}

	//____ _blockAt() __________________________________________________________

	const FlowTextLayout::Block * FlowTextLayout::_blockAt( const TextData * pData, int ofs ) const
	{
		const auto& blocks = pData->markup.blocks;
		auto it = std::upper_bound( blocks.begin(), blocks.end(), ofs, [](int ofs, const Block& block) { return ofs < block.end; } );

		if( it != blocks.end() && it->begin <= ofs )
			return &(*it);

		return nullptr;
	}

	//____ _skipHidden() _______________________________________________________

	// Moves offset out of hidden or substituted run it is within (not at the edge of).

	int FlowTextLayout::_skipHidden( const TextData * pData, int ofs, bool bForward ) const
	{
		const Run * pRun = _runAt(pData, ofs);

		if( pRun && (pRun->bHidden || pRun->substitute) && ofs > pRun->begin )
			return bForward ? pRun->end : pRun->begin;

		return ofs;
	}

	//____ _skipHiddenForward() ________________________________________________

	// Moves offset to end of any hidden runs beginning at or containing it.

	int FlowTextLayout::_skipHiddenForward( const TextData * pData, int ofs ) const
	{
		const Run * pRun = _runAt(pData, ofs);
		while( pRun && pRun->bHidden )
		{
			ofs = pRun->end;
			pRun = _runAt(pData, ofs);
		}
		return ofs;
	}

	//____ _skipHiddenBackward() _______________________________________________

	// Moves offset to beginning of any hidden runs ending at or containing it.

	int FlowTextLayout::_skipHiddenBackward( const TextData * pData, int ofs ) const
	{
		while( ofs > 0 )
		{
			const Run * pRun = _runAt(pData, ofs-1);
			if( !pRun || !pRun->bHidden )
				break;
			ofs = pRun->begin;
		}
		return ofs;
	}

	//____ rectForRange() _________________________________________________________

	RectSPX FlowTextLayout::rectForRange( const TextItem * pText, int ofs, int length ) const
	{
		int begChar = ofs;
		int endChar = ofs + length;

		const TextData * pData = _data(pText);
		SizeSPX canvas = _size(pText);

		int begLine = charLine(pText, begChar);
		int endLine = charLine(pText, endChar);

		if( begLine < 0 || endLine < 0 )
			return RectSPX();

		CoordSPX begPos = charPos(pText, begChar);
		CoordSPX endPos = charPos(pText, endChar);

		const LineInfo * pBegLine = &pData->lines[begLine];

		spx x1, x2, y1, y2;

		if (begLine == endLine)
		{
			x1 = begPos.x;
			y1 = begPos.y - pBegLine->base;
			x2 = endPos.x;
			y2 = y1 + pBegLine->height;
		}
		else
		{
			const LineInfo * pLine = pBegLine;

			spx yPos = begPos.y - pLine->base;

			x1 = begPos.x;
			y1 = yPos;
			x2 = pLine->width + _linePosX(pLine, canvas.w);

			yPos += pLine->spacing;

			for( int line = begLine + 1 ; line < endLine ; line++ )
			{
				pLine = &pData->lines[line];

				spx nx1 = _linePosX(pLine, canvas.w);
				spx nx2 = nx1 + pLine->width;

				if (nx1 < x1)
					x1 = nx1;
				if (nx2 > x2)
					x2 = nx2;

				yPos += pLine->spacing;
			}

			pLine = &pData->lines[endLine];

			spx nx1 = _linePosX(pLine, canvas.w);
			spx nx2 = endPos.x;

			if (nx1 < x1)
				x1 = nx1;
			if (nx2 > x2)
				x2 = nx2;

			y2 = yPos + pLine->height;
		}

		return RectSPX( x1, y1, x2-x1, y2-y1);
	}

	//____ rectForCaret() ______________________________________________________

	// Includes left/right margin where applicable.

	RectSPX FlowTextLayout::rectForCaret( const TextItem * pText ) const
	{
		Caret * pCaret = _caret();
		if( !_caretVisible(pText) || !pCaret )
			return RectSPX();

		return pCaret->dirtyRect( charRect(pText, _caretOfs(pText) ), _scale(pText) );
	}

	//____ rectForCaretWithMargin() ______________________________________________

	RectSPX FlowTextLayout::rectForCaretWithMargin( const TextItem * pText, int whitespacesBeforeAfter ) const
	{
		Caret * pCaret = _caret();
		if( !_caretVisible(pText) || !pCaret )
			return RectSPX();

		RectSPX cursorRect = pCaret->dirtyRect( charRect(pText, _caretOfs(pText) ), _scale(pText) );

		TextAttr		baseAttr;
		_baseStyle(pText)->exportAttr( _state(pText), &baseAttr, _scale(pText) );

		baseAttr.pFont->setSize(baseAttr.size);

		spx whitespace = baseAttr.pFont->whitespaceAdvance();

		cursorRect.x -= whitespace * whitespacesBeforeAfter;
		cursorRect.w += whitespace * whitespacesBeforeAfter*2;

		spx maxLength = _size(pText).w;

		limit( cursorRect.x, 0, maxLength );
		limit( cursorRect.w, 0, maxLength - cursorRect.x );

		return cursorRect;
	}


	//____ textDirection() ____________________________________________________

	Direction FlowTextLayout::textDirection( TextItem * pText, int charOfs ) const
	{
		return Direction::Right;
	}

	//____ caretToPos() _____________________________________________________

	int FlowTextLayout::caretToPos( TextItem * pText, CoordSPX pos, spx& wantedLineOfs ) const
	{
		wantedLineOfs = -1;

		int line = _lineAtPosY(pText, pos.y, SelectMode::Closest );
		return  _charAtPosX(pText, line, pos.x, SelectMode::ClosestBegin );
	}

	//____ caretUp() ___________________________________________________________

	int FlowTextLayout::caretUp( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		int line = charLine(pText, charOfs );

		if( line > 0 )
		{
			if( wantedLineOfs == -1 )
				wantedLineOfs = _charPosX( pText, charOfs );

			charOfs = _charAtPosX(pText, line-1, wantedLineOfs, SelectMode::ClosestBegin );
		}
		return charOfs;
	}

	//____ caretDown() _________________________________________________________

	int FlowTextLayout::caretDown( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		int line = charLine(pText, charOfs );

		if( line >= 0 && line < int(_data(pText)->lines.size())-1 )
		{
			if( wantedLineOfs == -1 )
				wantedLineOfs = _charPosX( pText, charOfs );

			charOfs = _charAtPosX(pText, line+1, wantedLineOfs, SelectMode::ClosestBegin );
		}
		return charOfs;
	}

	//____ caretLeft() _________________________________________________________

	// Hidden runs take no space, so positions on both sides of them look the same.
	// We step over them so the caret always moves one visible character.

	int FlowTextLayout::caretLeft( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		const TextData * pData = _data(pText);

		charOfs = _skipHiddenBackward( pData, charOfs );
		if( charOfs > 0 )
		{
			charOfs = _skipHidden( pData, Utf8::prev( _text(pText), charOfs ), false );
			charOfs = _skipHiddenBackward( pData, charOfs );
		}

		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretRight() ________________________________________________________

	int FlowTextLayout::caretRight( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		const TextData * pData = _data(pText);

		charOfs = _skipHiddenForward( pData, charOfs );
		if( charOfs < _length(pText) )
		{
			charOfs = _skipHidden( pData, Utf8::next( _text(pText), charOfs ), true );
			charOfs = _skipHiddenForward( pData, charOfs );
		}

		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretHome() ________________________________________________________

	int FlowTextLayout::caretHome( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		int line = charLine( pText, charOfs );

		if( line >= 0 )
			charOfs = _data(pText)->lines[line].begin;

		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretEnd() ________________________________________________________

	int FlowTextLayout::caretEnd( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		int line = charLine( pText, charOfs );

		if( line >= 0 )
		{
			const LineInfo& lineInfo = _data(pText)->lines[line];
			charOfs = _skipHidden( _data(pText), Utf8::prev( _text(pText), lineInfo.end ), false );
		}
		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretPrevWord() _____________________________________________________

	int FlowTextLayout::caretPrevWord( TextItem * pText, int charOfs ) const
	{
		const std::string& text = _text(pText);

		if( charOfs <= 0 )
			return 0;

		// Step over whitespace, then to beginning of word.

		int ofs = Utf8::prev(text, charOfs);
		if( Utf8::codepointAt(text, ofs) == '\n' )
			return ofs;

		while( ofs > 0 && _isWhitespace(Utf8::codepointAt(text, ofs)) )
			ofs = Utf8::prev(text, ofs);

		return _skipHidden( _data(pText), wordBegin(pText, ofs), false );
	}

	//____ caretNextWord() _____________________________________________________

	int FlowTextLayout::caretNextWord( TextItem * pText, int charOfs ) const
	{
		const std::string& text = _text(pText);
		int len = int(text.size());

		if( charOfs >= len )
			return len;

		if( Utf8::codepointAt(text, charOfs) == '\n' )
			return Utf8::next(text, charOfs);

		// Step to end of word, then over whitespace.

		int ofs = wordEnd(pText, charOfs);

		while( ofs < len && _isWhitespace(Utf8::codepointAt(text, ofs)) )
			ofs = Utf8::next(text, ofs);

		return _skipHidden( _data(pText), ofs, true );
	}

	//____ linkAt() ____________________________________________________________

	TextLink_p FlowTextLayout::linkAt( const TextItem * pText, int charOfs ) const
	{
		const Run * pRun = _runAt( _data(pText), charOfs );

		if( pRun )
		{
			if( pRun->pLink )
				return pRun->pLink;

			for( int i = pRun->nbStyles-1 ; i >= 0 ; i-- )
				if( pRun->styles[i]->link() )
					return pRun->styles[i]->link();
		}

		return _baseStyle(pText)->link();
	}

	//____ defaultSize() _________________________________________________________

	SizeSPX FlowTextLayout::defaultSize( const TextItem * pText, int scale ) const
	{
		const TextData * pData = _data(pText);

		if (scale != pData->scaleUsed )
			return _layoutLines( pText, pData, scale, _state(pText), -1, nullptr );

		return pData->defaultSize;
	}

	//____ matchingWidth() _________________________________________________________

	spx FlowTextLayout::matchingWidth( const TextItem * pText, spx height, int scale ) const
	{
		return defaultSize(pText, scale).w;
	}

	//____ matchingHeight() ________________________________________________________

	spx FlowTextLayout::matchingHeight( const TextItem * pText, spx width, int scale ) const
	{
		const TextData * pData = _data(pText);

		// Without line wrap the width doesn't matter, so the height only changes
		// with the text, its styles or its state. Containers ask for it at every
		// level of a layout, so we keep it until then.

		if( !m_bLineWrap )
		{
			if( scale == pData->scaleUsed )
			{
				if( pData->matchingHeight < 0 )
					const_cast<TextData*>(pData)->matchingHeight = _layoutLines( pText, pData, scale, _state(pText), -1, nullptr ).h;

				return pData->matchingHeight;
			}

			return _layoutLines( pText, pData, scale, _state(pText), -1, nullptr ).h;
		}

		return _layoutLines( pText, pData, scale, _state(pText), width, nullptr ).h;
	}

	//____ _linePosX() _______________________________________________________________

	spx FlowTextLayout::_linePosX( const LineInfo * pLine, spx canvasWidth ) const
	{
		switch( m_placement )
		{
			case Placement::NorthWest:
			case Placement::West:
			case Placement::SouthWest:
			default:
				return pLine->indent;
			case Placement::North:
			case Placement::Center:
			case Placement::South:
				return pLine->indent + Util::align((canvasWidth - pLine->indent - pLine->width) / 2);
			case Placement::NorthEast:
			case Placement::East:
			case Placement::SouthEast:
				return canvasWidth - pLine->width;
		}
	}

	//____ _linePosY() _______________________________________________________________

	spx FlowTextLayout::_linePosY( const TextData * pData, int line, spx canvasHeight ) const
	{
		spx ofsY = _textPosY( pData, canvasHeight );

		for( int i = 0 ; i < line ; i++ )
			ofsY += pData->lines[i].spacing;

		return ofsY;
	}

	//____ _textPosY() _____________________________________________________________

	spx	FlowTextLayout::_textPosY( const TextData * pData, spx canvasHeight ) const
	{
		switch( m_placement )
		{
			case Placement::NorthWest:
			case Placement::North:
			case Placement::NorthEast:
			default:
				return 0;
			case Placement::West:
			case Placement::Center:
			case Placement::East:
				return Util::align((canvasHeight - pData->textSize.h) / 2);
			case Placement::SouthWest:
			case Placement::South:
			case Placement::SouthEast:
				return canvasHeight - pData->textSize.h;
		}
	}

	//____ _charPosX() _________________________________________________________

	spx FlowTextLayout::_charPosX( const TextItem * pText, int charOfs ) const
	{
		int line = charLine(pText, charOfs);
		if( line < 0 )
			return 0;

		const LineInfo * pLine = &_data(pText)->lines[line];

		return _linePosX( pLine, _size(pText).w ) + _charDistance( pText, pLine->begin, charOfs );
	}

	//____ _lineAtPosY() _______________________________________________________

	int FlowTextLayout::_lineAtPosY( const TextItem * pText, spx posY, SelectMode mode ) const
	{
		const TextData * pData = _data(pText);
		const LineInfo * pLine = pData->lines.data();
		int nbLines = int(pData->lines.size());

		spx linePosY = _textPosY( pData, _size(pText).h );

		if( posY < linePosY )
		{
			if( mode == SelectMode::Marked )
				return -1;

			return 0;
		}

		switch( mode )
		{
			case SelectMode::ClosestBegin:
			{
				spx prevBeg = linePosY;
				linePosY += pLine[0].spacing;

				for( int i = 1 ; i < nbLines ; i++ )
				{
					spx beg = linePosY;

					if( posY <= beg )
					{
						if( posY - prevBeg < beg - posY )
							return i-1;
						else
							return i;
					}
					prevBeg = beg;
					linePosY += pLine[i].spacing;
				}
				return nbLines-1;
			}
			case SelectMode::ClosestEnd:
			{
				spx prevEnd = linePosY + pLine[0].height;
				linePosY += pLine[0].spacing;

				for( int i = 1 ; i < nbLines ; i++ )
				{
					spx end = linePosY + pLine[i].height;

					if( posY <= end )
					{
						if( posY - prevEnd < end - posY )
							return i-1;
						else
							return i;
					}
					prevEnd = end;
					linePosY += pLine[i].spacing;
				}
				return nbLines-1;
			}
			default:
			case SelectMode::Closest:
			case SelectMode::Marked:
			{
				for( int i = 0 ; i < nbLines ; i++ )
				{
					if( posY >= linePosY && posY < linePosY + pLine[i].height )
						return i;

					linePosY += pLine[i].spacing;
				}

				if( mode == SelectMode::Closest )
					return nbLines-1;

				return -1;
			}
		}
	}

	//____ _charAtPosX() _______________________________________________________

	int FlowTextLayout::_charAtPosX( const TextItem * pText, int line, spx posX, SelectMode mode ) const
	{
		const TextData * pData = _data(pText);
		const std::string& text = _text(pText);
		int textLen = int(text.size());

		const LineInfo * pLine = &pData->lines[line];

		int lastCaretPos = _skipHidden( pData, Utf8::prev( text, pLine->end ), false );

		spx lineX = _linePosX( pLine, _size(pText).w );

		// Handle special case when we are left of line.

		if( posX < lineX )
		{
			if( mode == SelectMode::Marked )
				return -1;
			else
				return pLine->begin;
		}

		// Handle special case when we are right of line.

		if( posX >= lineX + pLine->width )
		{
			if( mode == SelectMode::Marked )
				return -1;
			else
				return lastCaretPos;
		}

		// We are somewhere inside the line, lets loop through characters

		Cursor cursor( this, pText, pData, _scale(pText), _state(pText) );
		cursor.moveTo(pLine->begin);

		spx distance = lineX;

		while( true )
		{
			cursor.sync();

			int charOfs = cursor.pos();
			if( charOfs >= pLine->end || charOfs >= textLen || cursor.peek() == '\n' )
				return lastCaretPos;			// We have passed the end of the line, probably in lines cursor-margin.

			spx charBeg = distance;
			cursor.advance(distance, false, m_tabWidth);

			// Check if we have passed our mark.

			if( distance >= posX )
			{
				switch( mode )
				{
					case SelectMode::ClosestBegin:
					{
						if( posX - charBeg <= distance - posX )
							return charOfs;
						else
							return std::min( cursor.pos(), lastCaretPos );
					}
					case SelectMode::ClosestEnd:
					{
						if( posX - charBeg < distance - posX )
							return _skipHidden( pData, Utf8::prev(text, charOfs), false );
						else
							return charOfs;
					}
					case SelectMode::Closest:
					case SelectMode::Marked:
					default:
						return charOfs;
				}
			}
		}
	}

} // namespace wg
