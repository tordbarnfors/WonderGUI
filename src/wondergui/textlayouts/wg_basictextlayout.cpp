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
#include <wg_basictextlayout.h>
#include <wg_textstyle.h>
#include <wg_gfxdevice.h>
#include <wg_char.h>
#include <wg_msgrouter.h>
#include <wg_base.h>

#include <stdlib.h>
#include <algorithm>

namespace wg
{
	using namespace Util;

	const TypeInfo BasicTextLayout::TYPEINFO = { "BasicTextLayout", &TextLayout::TYPEINFO };

	//____ constructor _____________________________________________________________

	BasicTextLayout::BasicTextLayout(const Blueprint& bp)
	{
		m_pCaret				= bp.caret;
		m_placement				= bp.placement;

		m_selectionBackBlend	= bp.selectionBackBlend;
		m_selectionBackColor	= bp.selectionBackColor;
		m_selectionCharBlend	= bp.selectionCharBlend;
		m_selectionCharColor	= bp.selectionCharColor;

		m_softLineSpacing		= bp.lineSpacing;
		m_hardLineSpacing		= bp.lineSpacing + bp.paragraphSpacing;
        m_bAutoElipsis          = bp.autoEllipsis;

		m_bLineWrap				= bp.wrap;
		m_tabWidth				= bp.tabWidth;

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ Destructor ______________________________________________________________

	BasicTextLayout::~BasicTextLayout()
	{
		if( m_bReceivingUpdates )
		{
			Base::_stopReceiveUpdates(this);
			m_bReceivingUpdates = false;
		}
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& BasicTextLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ addText() _________________________________________________________

	void BasicTextLayout::addText( TextItem * pText )
	{
		const Char * pChars = _chars(pText);
		int nLines = _countLines( pText, pChars );

		_setTextDataBlock(pText,nullptr);					// Make sure pointer is null for the realloc call.
		void * pBlock = _reallocBlock(pText,nLines, _scale(pText), _hasCharStyles(pChars), {-1,-1}, {-1,-1} );

		_updateLineInfo( pText, pBlock, pChars, true );
	}

	//____ removeText() _________________________________________________________

	void BasicTextLayout::removeText( TextItem * pText )
	{
		free( _dataBlock(pText) );
		_setTextDataBlock(pText, nullptr);

		if( pText == m_pFocusedText )
		{
			m_pFocusedText = 0;
			if( m_bReceivingUpdates )
			{
				Base::_stopReceiveUpdates(this);
				m_bReceivingUpdates = false;
			}
		}
	}

	//____ didMoveText() _________________________________________________________

	void BasicTextLayout::didMoveText(TextItem* pFrom, TextItem* pTo)
	{
		// Do nothing.
	}


	//____ charAtPos() _________________________________________________________

	int BasicTextLayout::charAtPos( const TextItem * pText, CoordSPX pos ) const
	{
		int line = _lineAtPosY(pText, pos.y, SelectMode::Marked );
		if( line == -1 )
			return -1;

		return  _charAtPosX(pText, line, pos.x, SelectMode::Marked );
	}

	//_____ charPos() ______________________________________________________

	CoordSPX BasicTextLayout::charPos( const TextItem * pText, int charOfs ) const
	{
		int line = charLine(pText, charOfs);

		spx ofsX = _charPosX(pText, charOfs);
		spx ofsY = _linePosY(_dataBlock(pText), line, _size(pText).h );

		const LineInfo * pLine = _lineInfo( _dataBlock(pText) ) + line;
		ofsY += pLine->base;

		return CoordSPX(ofsX,ofsY);
	}

	//____ charRect() ________________________________________________________

	RectSPX BasicTextLayout::charRect( const TextItem * pText, int charOfs ) const
	{
		const void * pBlock = _dataBlock(pText);
		const BlockHeader * pHeader = _header(pBlock);
		const LineInfo * pLineInfo = _lineInfo(pBlock);

		int	scale = _scale(pText);
		State state = _state(pText);

		// Find correct line and determine yOfs

		spx yOfs = _textPosY( pHeader, _size(pText).h );
		while( pLineInfo->length <= charOfs )
		{
			yOfs += pLineInfo->spacing;
			charOfs-= pLineInfo->length;
			pLineInfo++;
		}

		// Determine xOfs by parsing line until character

		spx xOfs = _linePosX( pLineInfo, _size(pText).w );

		TextAttr		baseAttr;
		_baseStyle(pText)->exportAttr( state, &baseAttr, scale );

		const Char * pFirst = _chars(pText) + pLineInfo->offset;
		const Char * pLast = pFirst + charOfs;

		xOfs += _charDistance( pFirst, pLast, baseAttr, scale, state );

		// Get cell width

		spx width = 0;

		TextAttr	attr = baseAttr;

		if( pLast->styleHandle() != 0 )
			pLast->stylePtr()->addToAttr( state, &attr, scale );

		Font * pFont = attr.pFont.rawPtr();
		pFont->setSize(attr.size);

		Glyph glyph;


		_getGlyphWithoutBitmap( pFont, pLast->code(), glyph );

		if( glyph.advance > 0 )
			width = glyph.advance;				// Do not advance for last, just apply kerning.
		else if( pLast->code() == 32 )
			width = pFont->whitespaceAdvance();
		else if (pLast->code() == '\t')
			width = _tabWidthSPX(pFont->whitespaceAdvance(), xOfs );

		return RectSPX(xOfs,yOfs,width,pLineInfo->height);
	}

	//____ charLine() ________________________________________________________

	int BasicTextLayout::charLine( const TextItem * pText, int charOfs ) const
	{
		if( charOfs < 0 )
			return -1;

		const void * pBlock = _dataBlock(pText);
		const BlockHeader * pHeader = _header(pBlock);
		const LineInfo * pLineInfo = _lineInfo(pBlock);

		for( int i = 0 ; i < pHeader->nbLines ; i++ )
		{
			if( charOfs < pLineInfo[i].offset + pLineInfo[i].length )
				return i;
		}

		return -1;
	}

	//____ lineBegin() ________________________________________________________

	int BasicTextLayout::lineBegin( const TextItem * pText, int lineNb ) const
	{
		const void * pBlock = _dataBlock(pText);
		const BlockHeader * pHeader = _header(pBlock);
		const LineInfo * pLineInfo = _lineInfo(pBlock);

		if( lineNb < 0 || lineNb >= pHeader->nbLines )
			return -1;

		return pLineInfo[lineNb].offset;
	}

	//____ lineEnd() ___________________________________________________________

	int BasicTextLayout::lineEnd( const TextItem * pText, int lineNb ) const
	{
		const void * pBlock = _dataBlock(pText);
		const BlockHeader * pHeader = _header(pBlock);
		const LineInfo * pLineInfo = _lineInfo(pBlock);

		if( lineNb < 0 || lineNb >= pHeader->nbLines )
			return -1;

		return pLineInfo[lineNb].offset + pLineInfo[lineNb].length;
	}

	//____ wordBegin() _________________________________________________________

	int BasicTextLayout::wordBegin( const TextItem * pText, int charOfs ) const
	{
		//TODO: Implement!
		return charOfs;
	}

	//____ wordEnd() ___________________________________________________________

	int BasicTextLayout::wordEnd( const TextItem * pText, int charOfs ) const
	{
		//TODO: Implement!
		return charOfs+1;
	}


	//____ _charDistance() _____________________________________________________

	// Returns distance in points between beginning of first and beginning of last char.
	// Chars should be on the same line (or pLast could be first char on next line)

	spx BasicTextLayout::_charDistance( const Char * pFirst, const Char * pLast, const TextAttr& baseAttr, int scale, State state ) const
	{
		TextAttr		attr;
		Font_p 			pFont;
		TextStyle_h		hStyle = 0xFFFF;

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];
		const Char * pChar = pFirst;

		spx distance = 0;

		while( pChar <= pLast )
		{
			// TODO: Include handling of special characters
			// TODO: Support char textcolor and background color and effects.

			if( pChar->styleHandle() != hStyle )
			{
				int oldFontSize = attr.size;
				attr = baseAttr;

				if( pChar->styleHandle() != 0 )
					pChar->stylePtr()->addToAttr( state, &attr, scale );

				if( pFont != attr.pFont || attr.size != oldFontSize )
				{
					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning across different fonts or characters of different size.
				}
			}

			_getGlyphWithoutBitmap( pFont.rawPtr(), pChar->code(), * pGlyph );

			if( pGlyph->advance > 0 )
			{
				distance += pFont->kerning(*pPrevGlyph, *pGlyph);

				if( pChar != pLast )
					distance += pGlyph->advance;				// Do not advance for last, just apply kerning.
			}
			else if( pChar->code() == 32 && pChar != pLast )
				distance += pFont->whitespaceAdvance();
			else if (pChar->code() == '\t' && pChar != pLast )
				distance += _tabWidthSPX(pFont->whitespaceAdvance(), distance);

			std::swap(pGlyph, pPrevGlyph);
			pChar++;
		}

		return distance;
	}

	//____ _update() ___________________________________________________________

	void BasicTextLayout::_update(int microPassed, int64_t microsecTimestamp)
	{
		if( _caretVisible(m_pFocusedText) )
		{
			Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
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

	void BasicTextLayout::render( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas )
	{
		int scale = _scale(pText);

		void * pBlock = _dataBlock(pText);
		BlockHeader * pHeader = _header(pBlock);
		LineInfo * pLineInfo = _lineInfo(pBlock);
		const Char * pCharArray = _chars(pText);

		CoordSPX lineStart = canvas.pos();
		lineStart.y += _textPosY( pHeader, canvas.h );

		TextAttr		baseAttr;
		_baseStyle(pText)->exportAttr( _state(pText), &baseAttr, scale );

		TextAttr		attr;
		Font_p 			pFont = nullptr;
		TextStyle_h		hStyle = 0xFFFF;

		HiColor	baseTint = pDevice->tintColor();
		HiColor	localTint = HiColor::White;

		BlendMode renderMode = pDevice->blendMode();


		// Limit our cliplist if needed

		ClipPopData popData;
		if (pHeader->textSize.w > canvas.w || pHeader->textSize.h > canvas.h)
			popData = limitClipList(pDevice, canvas);

		Glyph	ellipsisGlyph;
		int		ellipsisLength;
		bool 	bNeedsEllipsis = false;

		if(m_bAutoElipsis && (pHeader->textSize.w > canvas.w || pHeader->textSize.h > canvas.h) && !_caretVisible(pText) )
		{
			bNeedsEllipsis = true;

			// Get glyph bitmap and length based on baseAttr.

			pFont = baseAttr.pFont;
			pFont->setSize(baseAttr.size);

			if( pFont->hasGlyph(c_ellipsisCode) )
			{
				_getGlyphWithBitmap( pFont.rawPtr(), c_ellipsisCode, ellipsisGlyph);
				ellipsisLength = ellipsisGlyph.advance;
			}
			else
			{
				_getGlyphWithBitmap( pFont.rawPtr(), '.', ellipsisGlyph);
				ellipsisLength = ellipsisGlyph.advance * 3;
			}
		}

		// Render back colors

		_renderBack( pText, pDevice, canvas );


		int selBeg, selEnd;
		std::tie(selBeg,selEnd) = _selection(pText);

		// Get selection start and end

		const Char * pSelBeg = nullptr;
		const Char * pSelEnd = nullptr;

		bool bInSelection = false;

		if( selBeg != selEnd )
		{
			if( m_selectionBackBlend != BlendMode::Ignore )
			{
				if( m_selectionBackBlend != BlendMode::Undefined )
					pDevice->setBlendMode( m_selectionBackBlend );
				_renderBackSection( pText, pDevice, canvas, selBeg, selEnd, m_selectionBackColor );
				pDevice->setBlendMode( renderMode );
			}

			pSelBeg = pCharArray + selBeg;
			pSelEnd = pCharArray + selEnd;
		}


		//

		const RectSPX& clip = pDevice->clipBounds();

		for( int i = 0 ; i < pHeader->nbLines ; i++ )
		{
			if( lineStart.y < clip.y + clip.h && lineStart.y + pLineInfo->height > clip.y )
			{
				lineStart.x = canvas.x + _linePosX( pLineInfo, canvas.w );
				const Char * pChar = pCharArray + pLineInfo->offset;

				Glyph glyph[2];
				Glyph* pGlyph		= &glyph[0];
				Glyph* pPrevGlyph	= &glyph[1];

				bool bLineHasEllipsis = false;
				if( bNeedsEllipsis )
				{
					if( pLineInfo->width > canvas.w )
						bLineHasEllipsis = true;
					else if( m_bLineWrap )
					{
						if( i+1 < pHeader->nbLines && lineStart.y + pLineInfo->spacing + pLineInfo[1].height > clip.y + clip.h )
						{
							bLineHasEllipsis = true;

							// With lineWrap we readjust text for line that gets ellipsis since that might make it all fit.

							lineStart.x = std::max( canvas.x, canvas.x + _linePosX( pLineInfo, canvas.w - ellipsisLength ));
						}
						else if( lineStart.y + pLineInfo->spacing + pLineInfo->height > clip.y + clip.h )
						{
							// Prevent partially displayed line from being displayed.
							// We might get here if previous line with ellipsis is skipped due to clipping.

							break;
						}
					}
				}

				CoordSPX pos = lineStart;
				pos.y += pLineInfo->base;

				bool 	bRecalcColor = false;
				spx		ellipsisX = pos.x;

				for( int x = 0 ; x < pLineInfo->length ; x++ )
				{
					// TODO: Include handling of special characters
					// TODO: Support char textcolor and background color and effects.

					if( pChar->styleHandle() != hStyle )
					{
						spx oldFontSize = attr.size;
						HiColor oldBackColor = attr.backColor;

						attr = baseAttr;

						if( pChar->styleHandle() != 0 )
							pChar->stylePtr()->addToAttr( _state(pText), &attr, scale );

						if( pFont != attr.pFont || attr.size != oldFontSize )
						{

							if( !pFont || (pFont->isMonochrome() != attr.pFont->isMonochrome()) )
								bRecalcColor = true;		// Font tint-color is changed.

							pFont = attr.pFont;
							pFont->setSize(attr.size);
							pPrevGlyph->advance = 0;								// No kerning against across different fonts or character of different size.
						}

						if( attr.color != localTint )
						{
							localTint = attr.color;
							bRecalcColor = true;
						}
					}

					//

					if( pChar == pSelBeg )
					{
						bInSelection = true;
						bRecalcColor = true;
					}
					else if( pChar == pSelEnd )
					{
						bInSelection = false;
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

					//

					_getGlyphWithBitmap( pFont.rawPtr(), pChar->code(), * pGlyph);

					if( pGlyph->advance > 0 )
					{
						if( bLineHasEllipsis && (pos.x - canvas.x) + pGlyph->advance + ellipsisLength > canvas.w )
							break;

						pos.x += pFont->kerning(*pPrevGlyph, *pGlyph);

						pDevice->setBlitSource(pGlyph->pSurface);
						pDevice->blit( CoordSPX(pos.x + pGlyph->bearingX, pos.y + pGlyph->bearingY), pGlyph->rect  );

						pos.x += pGlyph->advance;
						ellipsisX = pos.x;
					}
					else if( pChar->code() == 32 )
						pos.x += pFont->whitespaceAdvance();
					else if (pChar->code() == '\t')
						pos.x += _tabWidthSPX(pFont->whitespaceAdvance(), pos.x - canvas.x);

					std::swap(pPrevGlyph, pGlyph);
					pChar++;
				}

				//

				if( bLineHasEllipsis )
				{
					// Do kerning if font and size is same as for ellipsis.

					if( baseAttr.pFont == attr.pFont && baseAttr.size == attr.size )
						pos.x += pFont->kerning(*pPrevGlyph, ellipsisGlyph);

					// Blit ellipsis.

					pDevice->setBlitSource(ellipsisGlyph.pSurface);

					if( pFont->hasGlyph(c_ellipsisCode) )
					{
						pDevice->blit( CoordSPX(ellipsisX + ellipsisGlyph.bearingX, pos.y + ellipsisGlyph.bearingY), ellipsisGlyph.rect  );
					}
					else
					{
						pDevice->blit( CoordSPX(ellipsisX + ellipsisGlyph.bearingX, pos.y + ellipsisGlyph.bearingY), ellipsisGlyph.rect  );
						ellipsisX += ellipsisGlyph.advance;
						pDevice->blit( CoordSPX(ellipsisX + ellipsisGlyph.bearingX, pos.y + ellipsisGlyph.bearingY), ellipsisGlyph.rect  );
						ellipsisX += ellipsisGlyph.advance;
						pDevice->blit( CoordSPX(ellipsisX + ellipsisGlyph.bearingX, pos.y + ellipsisGlyph.bearingY), ellipsisGlyph.rect  );
					}

					// In linewrap mode we need to prevent us from drawing an extra half-hidden line below our ellipsis

					if( m_bLineWrap )
						break;
				}

			}

			lineStart.y += pLineInfo->spacing;
			pLineInfo++;
		}

		pDevice->setTintColor( baseTint );


		// Render caret (if there is any)

		if( _caretVisible(pText) )
		{
			Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();

			if( pCaret )
			{
				pCaret->render( pDevice, charRect(pText, _caretOfs(pText)) + canvas.pos(), _scale(pText) );
			}
		}
		// Pop any changes to cliplist.

		popClipList(pDevice, popData);
	}

	//____ _renderBack()___________________________________________________________

	void BasicTextLayout::_renderBack( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas )
	{
		const Char * pCharArray = _chars(pText);
		const Char * pBeg = pCharArray;
		const Char * pChar;

		TextStyle_h hStyle = 0xFFFF;

		HiColor		color = HiColor::Transparent;

		for( pChar = pCharArray ; !pChar->isEndOfText() ; pChar++ )
		{
			if( pChar->styleHandle() != hStyle )
			{
				State state = _state(pText);

				TextStyle_p p = pChar->stylePtr();
				HiColor newColor = p ? p->backColor(state) : HiColor::Undefined;

				if( newColor == HiColor::Undefined )
					newColor = _baseStyle(pText)->backColor(state);

				if( newColor != color )
				{
					if( color.a != 0 )
						_renderBackSection( pText, pDevice, canvas, int(pBeg - pCharArray), int(pChar - pCharArray), color );
					color = newColor;
					pBeg = pChar;
				}

				hStyle = pChar->styleHandle();
			}
		}

		if( color.a != 0 )
			_renderBackSection( pText, pDevice, canvas, int(pBeg - pCharArray), int(pChar - pCharArray), color );
	}

	//____ _renderBackSection() ________________________________________________

	void BasicTextLayout::_renderBackSection( TextItem * pText, GfxDevice * pDevice, const RectSPX& canvas,
											int begChar, int endChar, HiColor color )
	{

		CoordSPX begPos = charPos( pText, begChar );
		LineInfo * pBegLine = _lineInfo( _dataBlock(pText) ) + charLine( pText, begChar );

		CoordSPX endPos = charPos( pText, endChar );
		LineInfo * pEndLine = _lineInfo( _dataBlock(pText) ) + charLine( pText, endChar );


		if( pBegLine == pEndLine )
		{
			RectSPX area;
			area.x = canvas.x + begPos.x;
			area.y = canvas.y + begPos.y - pBegLine->base;
			area.w = endPos.x - begPos.x;
			area.h = pBegLine->height;

			pDevice->fill( area, color );
		}
		else
		{
			LineInfo * pLine = pBegLine;
			spx		yPos = canvas.y + begPos.y -pLine->base;

			RectSPX area;
			area.x = canvas.x + begPos.x;
			area.y = yPos;
			area.w = pLine->width - (begPos.x - _linePosX( pLine, canvas.w));
			area.h = pLine->height;

			pDevice->fill( area, color );

			yPos += pLine->spacing;
			pLine++;

			while( pLine != pEndLine )
			{
				area.x = canvas.x + _linePosX( pLine, canvas.w);
				area.y = yPos;
				area.w = pLine->width;
				area.h = pLine->height;

				pDevice->fill( area, color );

				yPos += pLine->spacing;
				pLine++;
			}


			area.x = canvas.x + _linePosX( pLine, canvas.w);
			area.y = yPos;
			area.w = endPos.x;
			area.h = pLine->height;

			pDevice->fill( area, color );
		}

	}

	//____ caretMove() ________________________________________________________

	void BasicTextLayout::caretMove( TextItem * pText, int newOfs, int oldOfs )
	{
		Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
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

	void BasicTextLayout::selectionChange( TextItem * pText, int selectOfs, int caretOfs, int oldSelectOfs, int oldCaretOfs )
	{
		int oldBeg = std::min(oldSelectOfs,oldCaretOfs);
		int oldEnd = std::max(oldSelectOfs,oldCaretOfs);

		int newBeg = std::min(selectOfs,caretOfs);
		int newEnd = std::max(selectOfs,caretOfs);

		// Two cases here, either new and old selection overlaps or they are completely disjointed

		if( (newBeg < oldBeg && newEnd > oldBeg) || (newBeg > oldBeg && oldEnd > newBeg) )
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
			if( newEnd != oldEnd > 0 )
				_setTextDirty( pText, rectForRange( pText, newBeg, newEnd - newBeg ));
		}

		// Update/redraw the caret

		caretMove( pText, caretOfs, oldCaretOfs );
	}

	//____ onTextModified() ____________________________________________________

	void BasicTextLayout::onTextModified( TextItem * pText, int ofs, int charsRemoved, int charsAdded )
	{
		_refresh(pText, true);
	}

	//____ onResized() ___________________________________________________________

	void BasicTextLayout::onResized( TextItem * pText, SizeSPX newSize, SizeSPX oldSize, int newScale, int oldScale )
	{
		if (m_bLineWrap || newScale != oldScale)
			_refresh(pText, false);


		///TODO: Implement!
	}

	//____ onStateChanged() ______________________________________________________

	void BasicTextLayout::onStateChanged( TextItem * pText, State newState, State oldState )
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
				m_pFocusedText = 0;
				if( m_bReceivingUpdates )
				{
					Base::_stopReceiveUpdates(this);
					m_bReceivingUpdates = false;
				}
			}
		}

		// Check if we might need to requestRender because of state change.

		//TODO: Optimize: Go through and check all char styles to see if we need to setTextDirty.

		auto bHasCharStyles = _header(_dataBlock(pText))->hasCharStyles;

		if( bHasCharStyles || !_baseStyle(pText)->isStateIdentical(newState, oldState) )
			_setTextDirty(pText);
	}

	//____ onStyleChanged() ______________________________________________________

	void BasicTextLayout::onStyleChanged( TextItem * pText, TextStyle * pNewStyle, TextStyle * pOldStyle )
	{
		_refresh(pText, true);
/*
		//TODO: Optimize: only update line info if textsize possibly affected.

		void * pBlock = _dataBlock(pText);

		_updateLineInfo( pText, pBlock, _chars(pText) );

		_setTextDirty(pText);
*/
	}

	//____ onCharStyleChanged() __________________________________________________

	void BasicTextLayout::onCharStyleChanged( TextItem * pText, int ofs, int len )
	{
		_refresh(pText, true);
/*
		void * pBlock = _dataBlock(pText);

		_updateLineInfo( pText, pBlock, _chars(pText) );

		_setTextDirty(pText);
*/
	}

	//____ _refresh() ___________________________________________________________

	void BasicTextLayout::_refresh( TextItem * pText, bool bAllowRequestResize )
	{
		const Char * pChars = _chars(pText);
		int nLines = _countLines( pText, pChars );

		void * pBlock = _dataBlock(pText);

		if( !pBlock || _header(pBlock)->nbLines != nLines )
			pBlock = _reallocBlock(pText,nLines, _scale(pText), _hasCharStyles(pChars), _header(pBlock)->defaultSize, _header(pBlock)->textSize);
		else
			_header(pBlock)->hasCharStyles = _hasCharStyles(pChars);

		_updateLineInfo( pText, pBlock, pChars, bAllowRequestResize );
		_setTextDirty(pText);
	}

	//___ rectForRange() _________________________________________________________

	RectSPX BasicTextLayout::rectForRange( const TextItem * pText, int ofs, int length ) const
	{
		int begChar = ofs;
		int endChar = ofs + length;

		SizeSPX canvas = _size(pText);


		CoordSPX begPos = charPos(pText, begChar);
		const LineInfo * pBegLine = _lineInfo(_dataBlock(pText)) + charLine(pText, begChar);

		CoordSPX endPos = charPos(pText, endChar);
		const LineInfo * pEndLine = _lineInfo(_dataBlock(pText)) + charLine(pText, endChar);

		spx x1, x2, y1, y2;

		if (pBegLine == pEndLine)
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

			pLine++;

			while (pLine != pEndLine)
			{
				spx nx1 = _linePosX(pLine, canvas.w);
				spx nx2 = nx1 + pLine->width;

				if (nx1 < x1)
					x1 = nx1;
				if (nx2 > x2)
					x2 = nx2;

				yPos += pLine->spacing;
				pLine++;
			}

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

	RectSPX BasicTextLayout::rectForCaret( const TextItem * pText ) const
	{
		Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
		if( !_caretVisible(pText) || !pCaret )
			return RectSPX();

		return pCaret->dirtyRect( charRect(pText, _caretOfs(pText) ), _scale(pText) );
	}

	//____ rectForCaretWithMargin() ______________________________________________

	RectSPX BasicTextLayout::rectForCaretWithMargin( const TextItem * pText, int whitespacesBeforeAfter ) const
	{
		Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
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

	Direction BasicTextLayout::textDirection( TextItem * pText, int charOfs ) const
	{
		return Direction::Right;
	}

	//____ caretToPos() _____________________________________________________

	int BasicTextLayout::caretToPos( TextItem * pText, CoordSPX pos, spx& wantedLineOfs ) const
	{
		wantedLineOfs = -1;

		int line = _lineAtPosY(pText, pos.y, SelectMode::Closest );
		return  _charAtPosX(pText, line, pos.x, SelectMode::ClosestBegin );
	}

	//____ caretUp() ___________________________________________________________

	int BasicTextLayout::caretUp( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
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

	int BasicTextLayout::caretDown( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		int line = charLine(pText, charOfs );

		auto pHeader = _header(_dataBlock(pText));

		if( line >= 0 && line < _header(_dataBlock(pText))->nbLines-1 )
		{
			if( wantedLineOfs == -1 )
				wantedLineOfs = _charPosX( pText, charOfs );

			charOfs = _charAtPosX(pText, line+1, wantedLineOfs, SelectMode::ClosestBegin );
		}
		return charOfs;
	}

	//____ caretLeft() _________________________________________________________

	int BasicTextLayout::caretLeft( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		if( charOfs > 0 )
			charOfs--;

		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretRight() ________________________________________________________

	int BasicTextLayout::caretRight( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		if( charOfs < _length(pText) )
			charOfs++;

		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretHome() ________________________________________________________

	int BasicTextLayout::caretHome( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		int line = charLine( pText, charOfs );

		if( line >= 0 )
		{
			const LineInfo * pLine = _lineInfo( _dataBlock(pText) ) + line;
			charOfs = pLine->offset;
		}
		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretEnd() ________________________________________________________

	int BasicTextLayout::caretEnd( TextItem * pText, int charOfs, spx& wantedLineOfs ) const
	{
		int line = charLine( pText, charOfs );

		if( line >= 0 )
		{
			const LineInfo * pLine = _lineInfo( _dataBlock(pText) ) + line;
			charOfs = pLine->offset+ pLine->length-1;
		}
		wantedLineOfs = -1;
		return charOfs;
	}

	//____ caretPrevWord() _____________________________________________________

	int BasicTextLayout::caretPrevWord( TextItem * pText, int charOfs ) const
	{
		//TODO: Implement!
		return charOfs;
	}

	//____ caretNextWord() _____________________________________________________

	int BasicTextLayout::caretNextWord( TextItem * pText, int charOfs ) const
	{
		//TODO: Implement!
		return charOfs;
	}


	//____ tooltip() _______________________________________________________________

	String BasicTextLayout::tooltip( const TextItem * pText ) const
	{
		//TODO: Return the text if it overflows the mapped area?

		return String();
	}


	//____ defaultSize() _________________________________________________________

	SizeSPX BasicTextLayout::defaultSize( const TextItem * pText, int scale ) const
	{
		if (scale != _header(_dataBlock(pText))->scaleUsed )
			return _calcDefaultSize(_chars(pText),_baseStyle(pText), scale, _state(pText));

		return _header(_dataBlock(pText))->defaultSize;
	}

	//____ matchingWidth() _________________________________________________________

	spx BasicTextLayout::matchingWidth( const TextItem * pText, spx height, int scale ) const
	{
		if (scale != _header(_dataBlock(pText))->scaleUsed )
			return _calcDefaultSize(_chars(pText),_baseStyle(pText), scale, _state(pText)).w;

		return	_header(_dataBlock(pText))->defaultSize.w;
	}

	//____ matchingHeight() ________________________________________________________

	spx BasicTextLayout::matchingHeight( const TextItem * pText, spx width, int scale ) const
	{
		return _calcMatchingHeight(_chars(pText), _baseStyle(pText), scale, _state(pText), width);
	}

	//____ _countLines() ___________________________________________________________

	int BasicTextLayout::_countLines( TextItem * pText, const Char * pChars ) const
	{
		if (m_bLineWrap)
			return _countWrapLines(pChars, _baseStyle(pText), _scale(pText), _state(pText), _size(pText).w);
		else
			return _countFixedLines(pChars);
	}

	//____ _hasCharStyles() ______________________________________________________

	bool BasicTextLayout::_hasCharStyles( const Char * pChars ) const
	{
		while( !pChars->isEndOfText() )
		{
			if( pChars->styleHandle() != 0 )
				return true;
			pChars++;
		}
		return false;
	}

	//____ _countFixedLines() ___________________________________________________________

	int BasicTextLayout::_countFixedLines(const Char * pChars) const
	{
		int lines = 0;
		while (true)
		{
			if (pChars->isEndOfLine())
			{
				lines++;
				if (pChars->isEndOfText())
					return lines;
			}
			pChars++;
		}
	}


	//____ _countWrapLines() ________________________________________________

	int BasicTextLayout::_countWrapLines(const Char * pChars, const TextStyle * pBaseStyle, int scale, State state, spx maxLineWidth) const
	{
		TextAttr		baseAttr;
		pBaseStyle->exportAttr(state, &baseAttr, scale);


		TextAttr		attr;
		Font_p 			pFont;

		TextStyle_h		hCharStyle = 0xFFFF;			// Force change on first character.

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		spx spaceAdv = 0;
		spx width = 0;
		spx potentialWidth = 0;
		spx eolCaretWidth = 0;
		//

		const Char * pBreakpoint = nullptr;

		int nLines = 0;												//

		while (true)
		{

			if (pChars->styleHandle() != hCharStyle)
			{

				spx oldFontSize = attr.size;

				attr = baseAttr;
				if (pChars->styleHandle() != 0)
					pChars->stylePtr()->addToAttr(state, &attr, scale);

				if (pFont != attr.pFont || spx(attr.size) != oldFontSize)
				{
					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning across different fonts or fontsizes.
				}

				spaceAdv = pFont->whitespaceAdvance();

				Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
				if (pCaret)
				{
					SizeSPX eolCellSize(1, pFont->maxAscend() + pFont->maxDescend());
					eolCaretWidth = pCaret->eolWidth(eolCellSize, scale);
				}
				else
					eolCaretWidth = 0;

			}

			// TODO: Include handling of special characters
			// TODO: Support sub/superscript.

			_getGlyphWithoutBitmap(pFont.rawPtr(), pChars->code(), *pGlyph);

			spx preGlyphWidth = width;

			if (pGlyph->advance > 0)
			{
				potentialWidth += pFont->kerning(*pPrevGlyph, *pGlyph);
				potentialWidth += pGlyph->advance;
				width = potentialWidth;
			}
			else if (pChars->code() == 32)
				potentialWidth += spaceAdv;
			else if (pChars->code() == '\t')
				potentialWidth += _tabWidthSPX(spaceAdv, potentialWidth);

			std::swap(pGlyph, pPrevGlyph);

			// Handle end of line

			if (pChars->isEndOfLine())
			{
				nLines++;

				//

				if (pChars->isEndOfText())
					break;

				// Prepare for next line

				pChars++;			// Line terminator belongs to previous line.

				width = 0;
				potentialWidth = 0;
				pBreakpoint = nullptr;
				pPrevGlyph->advance = 0;
			}
			else
			{
				if (width > maxLineWidth - eolCaretWidth)
				{
					nLines++;

					// Prepare for next line

					if( pBreakpoint )
						pChars = pBreakpoint;
					else if( preGlyphWidth == 0 )
						pChars++;

					width = 0;
					potentialWidth = 0;
					pBreakpoint = nullptr;
					pPrevGlyph->advance = 0;

				}
				else
				{
					int code = pChars->code();
					if (code == 32 || code == '\t' )
					{
						pBreakpoint = pChars + 1;
					}

					pChars++;
				}
			}
		}
		return nLines;
	}

	//____ _calcMatchingHeight() ________________________________________________

	spx BasicTextLayout::_calcMatchingHeight(const Char * pChars, const TextStyle * pBaseStyle, int scale, State state, spx maxLineWidth) const
	{
		Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();

		TextAttr		baseAttr;
		pBaseStyle->exportAttr(state, &baseAttr, scale);

		spx totalHeight = 0;

		TextAttr		attr;
		Font_p 			pFont;

		TextStyle_h		hCharStyle = 0xFFFF;			// Force change on first character.

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		spx maxAscend = 0;
		spx maxDescend = 0;
		spx maxDescendGap = 0;							// Including the line gap.
		spx spaceAdv = 0;
		spx width = 0;
		spx potentialWidth = 0;
		spx eolCaretWidth = 0;
		//

		const Char * pBreakpoint = nullptr;
		spx bpMaxAscend = 0;
		spx bpMaxDescend = 0;
		spx bpMaxDescendGap = 0;							// Including the line gap.

															//
		while (true)
		{

			if (pChars->styleHandle() != hCharStyle)
			{

				spx oldFontSize = attr.size;

				attr = baseAttr;
				if (pChars->styleHandle() != 0)
					pChars->stylePtr()->addToAttr(state, &attr, scale);

				if (pFont != attr.pFont || attr.size != oldFontSize)
				{
					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning across different fonts or fontsizes.
				}

				spx ascend = pFont->maxAscend();
				if (ascend > maxAscend)
					maxAscend = ascend;

				spx descend = pFont->maxDescend();
				if (descend > maxDescend)
					maxDescend = descend;

				spx descendGap = descend + pFont->lineGap();
				if (descendGap > maxDescendGap)
					maxDescendGap = descendGap;

				spaceAdv = pFont->whitespaceAdvance();

				if (pCaret)
				{
					SizeSPX eolCellSize(1, pFont->maxAscend() + pFont->maxDescend());
					eolCaretWidth = pCaret->eolWidth(eolCellSize, scale);
				}
				else
					eolCaretWidth = 0;

				hCharStyle = pChars->styleHandle();
			}

			// TODO: Include handling of special characters
			// TODO: Support sub/superscript.

			_getGlyphWithoutBitmap(pFont.rawPtr(), pChars->code(), * pGlyph);

			spx preGlyphWidth = width;

			if (pGlyph->advance > 0)
			{
				potentialWidth += pFont->kerning(* pPrevGlyph, * pGlyph);
				potentialWidth += pGlyph->advance;
				width = potentialWidth;
			}
			else if (pChars->code() == 32)
				potentialWidth += spaceAdv;
			else if (pChars->code() == '\t')
				potentialWidth += _tabWidthSPX(spaceAdv, potentialWidth);

			std::swap(pPrevGlyph, pGlyph);

			// Handle end of line

			if (pChars->isEndOfLine())
			{
				// Make sure we have space for eol caret

				if (pCaret)
				{
					SizeSPX eolCellSize(1, pFont->maxAscend() + pFont->maxDescend());
					spx w = pCaret->eolWidth(eolCellSize, scale);
					if (w > eolCellSize.w)
						width += w - eolCellSize.w;
				}

				// Update totalHeight

				totalHeight += pChars->isEndOfText() ? maxAscend + maxDescend : align( (maxAscend + maxDescendGap) * m_hardLineSpacing);

				if (pChars->isEndOfText())
					break;

				// Prepare for next line

				pChars++;			// Line terminator belongs to previous line.

				width = 0;
				potentialWidth = 0;
				pBreakpoint = nullptr;
				pPrevGlyph->advance = 0;

				if (pChars->styleHandle() == hCharStyle)
				{
					// Still need to reset these, don't include data from styles only on previous line.

					maxAscend = pFont->maxAscend();
					maxDescend = pFont->maxDescend();
					maxDescendGap = maxDescend + pFont->lineGap();
				}
				else
				{
					maxAscend = 0;
					maxDescend = 0;
					maxDescendGap = 0;
				}
			}
			else
			{
				if ( m_bLineWrap && (width > maxLineWidth - eolCaretWidth) )
				{
					// Update totalHeight

					if( pBreakpoint )
					{
						totalHeight += align( (bpMaxAscend + bpMaxDescendGap) * m_softLineSpacing);
						pChars = pBreakpoint;
					}
					else
					{
						totalHeight += 	align( (maxAscend + maxDescendGap) * m_softLineSpacing);
						if( preGlyphWidth == 0 )
							pChars++;
					}

					// Prepare for next line


					width = 0;
					potentialWidth = 0;
					pBreakpoint = nullptr;
					pPrevGlyph->advance = 0;

					if (pChars->styleHandle() == hCharStyle)
					{
						// Still need to reset these, don't include data from styles only on previous line.

						maxAscend = pFont->maxAscend();
						maxDescend = pFont->maxDescend();
						maxDescendGap = maxDescend + pFont->lineGap();
					}
					else
					{
						maxAscend = 0;
						maxDescend = 0;
						maxDescendGap = 0;
					}
				}
				else
				{
					int code = pChars->code();
					if ((code == 32 || code == '\t') && m_bLineWrap)
					{
						pBreakpoint = pChars + 1;
						bpMaxAscend = maxAscend;
						bpMaxDescend = maxDescend;
						bpMaxDescendGap = maxDescendGap;							// Including the line gap.
					}

					pChars++;
				}
			}
		}
		return totalHeight;
	}



	//____ _reallocBlock() _________________________________________________________

	void * BasicTextLayout::_reallocBlock( TextItem * pText, int nLines, int scaleUsed, bool bHasCharStyles, SizeSPX defaultSize, SizeSPX textSize )
	{
		void * pBlock = _dataBlock(pText);
		if( pBlock )
			free( pBlock );

		pBlock = malloc( sizeof(BlockHeader) + sizeof(LineInfo)*nLines);
		_setTextDataBlock(pText, pBlock);

		((BlockHeader *)pBlock)->nbLines = nLines;
		((BlockHeader *)pBlock)->scaleUsed = scaleUsed;
		((BlockHeader *)pBlock)->defaultSize = defaultSize;
		((BlockHeader *)pBlock)->textSize = textSize;
		((BlockHeader *)pBlock)->hasCharStyles = bHasCharStyles;

		return pBlock;
	}


	//____ _updateLineInfo() _______________________________________________________

	void BasicTextLayout::_updateLineInfo( TextItem * pText, void * pBlock, const Char * pChars, bool bAllowRequestResize )
	{
		BlockHeader * pHeader = _header(_dataBlock(pText));
		SizeSPX defaultSize;
		SizeSPX textSize;

		int scale = _scale(pText);

		if (m_bLineWrap)
		{
			//TODO: This is slow, calling both _updateFixedLineInfo and _updateWrapLineInfo if line is wrapped, just so we can update defaultSize.

			defaultSize = _updateFixedLineInfo( _lineInfo(pBlock), pChars, _baseStyle(pText), scale, _state(pText));
			textSize = _updateWrapLineInfo( _lineInfo(pBlock), pChars, _baseStyle(pText), scale, _state(pText), _size(pText).w);
		}
		else
		{
			defaultSize = _updateFixedLineInfo( _lineInfo(pBlock), pChars, _baseStyle(pText), scale, _state(pText));
			textSize = defaultSize;
		}

		pHeader->scaleUsed = scale;

		if (defaultSize != pHeader->defaultSize || textSize != pHeader->textSize)
		{
			pHeader->textSize = textSize;
			pHeader->defaultSize = defaultSize;
			if( bAllowRequestResize )
				_requestTextResize(pText);
		}
	}

	//____ _updateWrapLineInfo() ________________________________________________

	SizeSPX BasicTextLayout::_updateWrapLineInfo( LineInfo * pLines, const Char * pChars, const TextStyle * pBaseStyle, int scale, State state, spx maxLineWidth )
	{
		Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
		const Char * pTextStart = pChars;

		TextAttr		baseAttr;
		pBaseStyle->exportAttr(state, &baseAttr, scale);

		SizeSPX			size;

		TextAttr		attr;
		Font_p 			pFont;

		TextStyle_h		hCharStyle = 0xFFFF;			// Force change on first character.

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		spx maxAscend = 0;
		spx maxDescend = 0;
		spx maxDescendGap = 0;							// Including the line gap.
		spx spaceAdv = 0;
		spx width = 0;
		spx potentialWidth = 0;
		spx eolCaretWidth = 0;
		//

		const Char * pBreakpoint = nullptr;
		spx bpWidth = 0;
		spx bpMaxAscend = 0;
		spx bpMaxDescend = 0;
		spx bpMaxDescendGap = 0;							// Including the line gap.


		pLines->offset = int(pChars - pTextStart);

		while (true)
		{

			if (pChars->styleHandle() != hCharStyle)
			{

				spx oldFontSize = attr.size;

				attr = baseAttr;
				if (pChars->styleHandle() != 0)
					pChars->stylePtr()->addToAttr(state, &attr, scale);

				if (pFont != attr.pFont || attr.size != oldFontSize)
				{
					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning across different fonts or fontsizes.
				}

				spx ascend = pFont->maxAscend();
				if (ascend > maxAscend)
					maxAscend = ascend;

				spx descend = pFont->maxDescend();
				if (descend > maxDescend)
					maxDescend = descend;

				spx descendGap = descend + pFont->lineGap();
				if (descendGap > maxDescendGap)
					maxDescendGap = descendGap;

				spaceAdv = pFont->whitespaceAdvance();

				if (pCaret)
				{
					SizeSPX eolCellSize(1, pFont->maxAscend() + pFont->maxDescend());
					spx w = pCaret->eolWidth(eolCellSize, scale);
					eolCaretWidth = alignUp(std::max(w, eolCellSize.w));
				}
				else
					eolCaretWidth = 0;

				hCharStyle = pChars->styleHandle();
			}

			// TODO: Include handling of special characters
			// TODO: Support sub/superscript.

			_getGlyphWithoutBitmap(pFont.rawPtr(), pChars->code(), * pGlyph);

			int preGlyphWidth = width;
			if (pGlyph->advance > 0)
			{
				potentialWidth += pFont->kerning(* pPrevGlyph, * pGlyph);
				potentialWidth += pGlyph->advance;
				width = potentialWidth;
			}
			else if (pChars->code() == 32)
				potentialWidth += spaceAdv;
			else if (pChars->code() == '\t')
				potentialWidth += _tabWidthSPX(spaceAdv, potentialWidth);

			std::swap(pPrevGlyph, pGlyph);

			// Handle end of line

			if (pChars->isEndOfLine())
			{
				// Make sure we have space for eol caret

				if (pCaret)
				{
					SizeSPX eolCellSize(1, pFont->maxAscend() + pFont->maxDescend());
					spx w = pCaret->eolWidth(eolCellSize, scale);
					if (w > eolCellSize.w)
						width += w - eolCellSize.w;
				}

				// Finish this line

				pLines->length = int(pChars - (pTextStart + pLines->offset)) + 1; 		// +1 to include line terminator.

				pLines->width = width;
				pLines->height = maxAscend + maxDescend;
				pLines->base = maxAscend;
				pLines->spacing = align((maxAscend + maxDescendGap) * m_hardLineSpacing);
				pLines++;

				// Update size

				if (width > size.w)
					size.w = width;

				size.h += pChars->isEndOfText() ? maxAscend + maxDescend : align( (maxAscend + maxDescendGap)*m_hardLineSpacing);

				if (pChars->isEndOfText())
					break;

				// Prepare for next line

				pChars++;			// Line terminator belongs to previous line.

				pLines->offset = int(pChars - pTextStart);
				width = 0;
				potentialWidth = 0;
				pBreakpoint = nullptr;
				pPrevGlyph->advance = 0;

				if (pChars->styleHandle() == hCharStyle)
				{
					// Still need to reset these, don't include data from styles only on previous line.

					maxAscend = pFont->maxAscend();
					maxDescend = pFont->maxDescend();
					maxDescendGap = maxDescend + pFont->lineGap();
				}
				else
				{
					maxAscend = 0;
					maxDescend = 0;
					maxDescendGap = 0;
				}
			}
			else
			{
				if (width > maxLineWidth - eolCaretWidth )
				{
					if( pBreakpoint != nullptr )
					{
						pChars = pBreakpoint;
						maxAscend = bpMaxAscend;
						maxDescend = bpMaxDescend;
						width = bpWidth;
					}
					else if( preGlyphWidth > 0 )
						width = preGlyphWidth;
					else
						pChars++;

					width += eolCaretWidth;

					// Finish this line

					pLines->length = int(pChars - (pTextStart + pLines->offset));

					pLines->width = width;
					pLines->height = maxAscend + maxDescend;
					pLines->base = maxAscend;
					pLines->spacing = align((maxAscend + maxDescendGap) * m_softLineSpacing);
					pLines++;

					// Update size

					if (width > size.w)
						size.w = bpWidth;

					size.h += align( (maxAscend + maxDescendGap) * m_softLineSpacing);

					// Prepare for next line

					pLines->offset = int(pChars - pTextStart);
					width = 0;
					potentialWidth = 0;
					pBreakpoint = nullptr;
					pPrevGlyph->advance = 0;

					if (pChars->styleHandle() == hCharStyle)
					{
						// Still need to reset these, don't include data from styles only on previous line.

						maxAscend = pFont->maxAscend();
						maxDescend = pFont->maxDescend();
						maxDescendGap = maxDescend + pFont->lineGap();
					}
					else
					{
						maxAscend = 0;
						maxDescend = 0;
						maxDescendGap = 0;
					}

				}
				else
				{
					int code = pChars->code();
					if (code == 32 || code == '\t' )
					{
						pBreakpoint = pChars + 1;
						bpWidth = width;
						bpMaxAscend = maxAscend;
						bpMaxDescend = maxDescend;
						bpMaxDescendGap = maxDescendGap;							// Including the line gap.
					}

					pChars++;
				}
			}
		}
		return alignUp(size);
	}



	//____ _updateFixedLineInfo() ________________________________________________

	SizeSPX BasicTextLayout::_updateFixedLineInfo( LineInfo * pLines, const Char * pChars, const TextStyle * pBaseStyle,
												int scale, State state )
	{
		Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
		const Char * pTextStart = pChars;

		SizeSPX			size;

		TextAttr		baseAttr;
		pBaseStyle->exportAttr( state, &baseAttr, scale );

		TextAttr		attr;
		Font_p 			pFont;

		TextStyle_h		hCharStyle = 0xFFFF;			// Force change on first character.

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		spx maxAscend = 0;
		spx maxDescend = 0;
		spx maxDescendGap = 0;							// Including the line gap.
		spx spaceAdv = 0;
		spx width = 0;

		pLines->offset = int(pChars - pTextStart);

		attr.size = -1;								// Prevent reading uninitialized value further down.

		while( true )
		{
			if( pChars->styleHandle() != hCharStyle )
			{

				spx oldFontSize = attr.size;

				attr = baseAttr;
				if( pChars->styleHandle() != 0 )
					pChars->stylePtr()->addToAttr( state, &attr, scale );

				if( pFont != attr.pFont || attr.size != oldFontSize )
				{
					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning across different fonts or fontsizes.
				}

				spx ascend = pFont->maxAscend();
				if( ascend > maxAscend )
					maxAscend = ascend;

				spx descend = pFont->maxDescend();
				if( descend > maxDescend )
					maxDescend = descend;

				spx descendGap = descend + pFont->lineGap();
				if( descendGap > maxDescendGap )
					maxDescendGap = descendGap;

				spaceAdv = pFont->whitespaceAdvance();

				hCharStyle = pChars->styleHandle();
			}

			// TODO: Include handling of special characters
			// TODO: Support sub/superscript.

			_getGlyphWithoutBitmap( pFont.rawPtr(), pChars->code(), * pGlyph );

			if( pGlyph->advance > 0 )
			{
				width += pFont->kerning(* pPrevGlyph, * pGlyph);
				width += pGlyph->advance;
			}
			else if( pChars->code() == 32 )
				width += spaceAdv;
			else if (pChars->code() == '\t')
				width += _tabWidthSPX(spaceAdv, width);

			std::swap( pPrevGlyph, pGlyph);

			// Handle end of line

			if( pChars->isEndOfLine() )
			{
				// Make sure we have space for eol caret

				if( pCaret )
				{
					SizeSPX eolCellSize( 1, pFont->maxAscend() + pFont->maxDescend() );
					spx w = pCaret->eolWidth( eolCellSize, scale );
					width += alignUp(std::max(w, eolCellSize.w));

				}

				// Finish this line

				pLines->length = int(pChars - (pTextStart + pLines->offset)) +1; 		// +1 to include line terminator.

				pLines->width = width;
				pLines->height = maxAscend + maxDescend;
				pLines->base = maxAscend;
				pLines->spacing = align((maxAscend + maxDescendGap) * m_hardLineSpacing);
				pLines++;

				// Update size

				if (width > size.w)
					size.w = width;

				size.h += pChars->isEndOfText() ? maxAscend + maxDescend : align( (maxAscend + maxDescendGap) * m_hardLineSpacing);

				//

				if( pChars->isEndOfText() )
					break;

				// Prepare for next line

				pChars++;			// Line terminator belongs to previous line.

				pLines->offset = int(pChars - pTextStart);
				width = 0;
				pPrevGlyph->advance = 0;

				if (pChars->styleHandle() == hCharStyle)
				{
					// Still need to reset these, don't include data from styles only on previous line.

					maxAscend = pFont->maxAscend();
					maxDescend = pFont->maxDescend();
					maxDescendGap = maxDescend + pFont->lineGap();
				}
				else
				{
					maxAscend = 0;
					maxDescend = 0;
					maxDescendGap = 0;
				}
			}
			else
				pChars++;
		}


		return alignUp(size);
	}

//____ _calcDefaultSize() ________________________________________________
/*
	Should be identical to _updateFixedLineInfo() except that it doesn't write
	any line info.
 */


SizeSPX BasicTextLayout::_calcDefaultSize( const Char * pChars, const TextStyle * pBaseStyle,
											int scale, State state ) const
{
	Caret * pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();
	const Char * pTextStart = pChars;

	SizeSPX			size;

	TextAttr		baseAttr;
	pBaseStyle->exportAttr( state, &baseAttr, scale );

	TextAttr		attr;
	Font_p 			pFont;

	TextStyle_h		hCharStyle = 0xFFFF;			// Force change on first character.

	Glyph glyph[2];
	Glyph* pGlyph = &glyph[0];
	Glyph* pPrevGlyph = &glyph[1];

	spx maxAscend = 0;
	spx maxDescend = 0;
	spx maxDescendGap = 0;							// Including the line gap.
	spx spaceAdv = 0;
	spx width = 0;

	while( true )
	{
		if( pChars->styleHandle() != hCharStyle )
		{

			spx oldFontSize = attr.size;

			attr = baseAttr;
			if( pChars->styleHandle() != 0 )
				pChars->stylePtr()->addToAttr( state, &attr, scale );

			if( pFont != attr.pFont || attr.size != oldFontSize )
			{
				pFont = attr.pFont;
				pFont->setSize(attr.size);
				pPrevGlyph->advance = 0;								// No kerning across different fonts or fontsizes.
			}

			spx ascend = pFont->maxAscend();
			if( ascend > maxAscend )
				maxAscend = ascend;

			spx descend = pFont->maxDescend();
			if( descend > maxDescend )
				maxDescend = descend;

			spx descendGap = descend + pFont->lineGap();
			if( descendGap > maxDescendGap )
				maxDescendGap = descendGap;

			spaceAdv = pFont->whitespaceAdvance();

			hCharStyle = pChars->styleHandle();
		}

		// TODO: Include handling of special characters
		// TODO: Support sub/superscript.

		_getGlyphWithoutBitmap( pFont.rawPtr(), pChars->code(), * pGlyph );

		if( pGlyph->advance > 0 )
		{
			width += pFont->kerning(* pPrevGlyph, * pGlyph);
			width += pGlyph->advance;
		}
		else if( pChars->code() == 32 )
			width += spaceAdv;
		else if (pChars->code() == '\t')
			width += _tabWidthSPX(spaceAdv, width);

		std::swap( pPrevGlyph, pGlyph);

		// Handle end of line

		if( pChars->isEndOfLine() )
		{
			// Make sure we have space for eol caret

			if( pCaret )
			{
				SizeSPX eolCellSize( 1, pFont->maxAscend() + pFont->maxDescend() );
				spx w = pCaret->eolWidth( eolCellSize, scale );
				width += alignUp(std::max(w, eolCellSize.w));
			}

			// Update size

			if (width > size.w)
				size.w = width;

			size.h += pChars->isEndOfText() ? maxAscend + maxDescend : align( (maxAscend + maxDescendGap) * m_hardLineSpacing);

			//

			if( pChars->isEndOfText() )
				break;

			// Prepare for next line

			pChars++;			// Line terminator belongs to previous line.

			width = 0;
			pPrevGlyph->advance = 0;

			if (pChars->styleHandle() == hCharStyle)
			{
				// Still need to reset these, don't include data from styles only on previous line.

				maxAscend = pFont->maxAscend();
				maxDescend = pFont->maxDescend();
				maxDescendGap = maxDescend + pFont->lineGap();
			}
			else
			{
				maxAscend = 0;
				maxDescend = 0;
				maxDescendGap = 0;
			}
		}
		else
			pChars++;
	}
	return alignUp(size);
}



	//____ _linePosX() _______________________________________________________________

	spx BasicTextLayout::_linePosX( const LineInfo * pLine, spx canvasWidth ) const
	{
		switch( m_placement )
		{
			case Placement::NorthWest:
			case Placement::West:
			case Placement::SouthWest:
			default:
				return 0;
			case Placement::North:
			case Placement::Center:
			case Placement::South:
				return Util::align((canvasWidth - pLine->width) / 2);
			case Placement::NorthEast:
			case Placement::East:
			case Placement::SouthEast:
				return canvasWidth - pLine->width;
		}
	}

	//____ _linePosY() _______________________________________________________________

	spx BasicTextLayout::_linePosY( const void * pBlock, int line, spx canvasHeight ) const
	{
		spx ofsY = _textPosY( _header(pBlock), canvasHeight );

		const LineInfo * pL = _lineInfo(pBlock);
		for( int i = 0 ; i < line ; i++ )
			ofsY += pL[i].spacing;

		return ofsY;
	}

	//____ _textPosY() _____________________________________________________________

	spx	BasicTextLayout::_textPosY( const BlockHeader * pHeader, spx canvasHeight ) const
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
				return (canvasHeight - pHeader->textSize.h) / 2;
			case Placement::SouthWest:
			case Placement::South:
			case Placement::SouthEast:
				return canvasHeight - pHeader->textSize.h;
		}
	}

	//____ _charPosX() _________________________________________________________

	spx BasicTextLayout::_charPosX( const TextItem * pText, int charOfs ) const
	{
		const LineInfo * pLine = _lineInfo( _dataBlock(pText) ) + charLine(pText, charOfs);
		const Char * pChars = _chars(pText);

		TextAttr attr;
		_baseStyle(pText)->exportAttr( _state(pText), &attr, _scale(pText) );

		return _linePosX( pLine, _size(pText).w ) + _charDistance( pChars + pLine->offset, pChars + charOfs, attr, _scale(pText), _state(pText) );
	}

	//____ _lineAtPosY() _______________________________________________________

	int BasicTextLayout::_lineAtPosY( const TextItem * pText, spx posY, SelectMode mode ) const
	{
		const void * pBlock = _dataBlock(pText);
		const BlockHeader * pHead = _header(pBlock);
		spx linePosY = _textPosY( pHead, _size(pText).h );
		const LineInfo * pLine = _lineInfo( _dataBlock(pText) );

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

				for( int i = 1 ; i < pHead->nbLines ; i++ )
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
				return pHead->nbLines-1;
			}
			case SelectMode::ClosestEnd:
			{
				spx prevEnd = linePosY + pLine[0].height;
				linePosY += pLine[0].spacing;

				for( int i = 1 ; i < pHead->nbLines ; i++ )
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
				return pHead->nbLines-1;
			}
			default:
			case SelectMode::Closest:
			case SelectMode::Marked:
			{
				for( int i = 0 ; i < pHead->nbLines ; i++ )
				{
					if( posY >= linePosY && posY < linePosY + pLine[i].height )
						return i;

					linePosY += pLine[i].spacing;
				}

				if( mode == SelectMode::Closest )
					return pHead->nbLines-1;

				return -1;
			}
		}
	}

	//____ _charAtPosX() _______________________________________________________

	int BasicTextLayout::_charAtPosX( const TextItem * pText, int line, spx posX, SelectMode mode ) const
	{
		const LineInfo * pLine = _lineInfo( _dataBlock(pText) ) + line;


		spx distance = _linePosX( pLine, _size(pText).w );

		// Handle special case when we are left of line.

		if( posX < distance )
		{
			if( mode == SelectMode::Marked )
				return -1;
			else
				return pLine->offset;
		}

		// Handle special case when we are right of line.

		if( posX >= distance + pLine->width )
		{
			if( mode == SelectMode::Marked )
				return -1;
			else
				return pLine->offset + pLine->length-1;
		}

		// We are somewhere inside the line, lets loop through characters

		const Char * pTextBegin = _chars(pText);
		State state = _state(pText);
		int scale	= _scale(pText);

		TextAttr baseAttr;
		_baseStyle(pText)->exportAttr( state, &baseAttr, scale );


		TextAttr		attr;
		Font_p 			pFont;
		TextStyle_h		hStyle = 0xFFFF;

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		const Char * pChar = pTextBegin + pLine->offset;

		while( true )
		{
			// TODO: Include handling of special characters

			if( pChar->styleHandle() != hStyle )
			{
				spx oldFontSize = attr.size;
				attr = baseAttr;

				if( pChar->styleHandle() != 0 )
					pChar->stylePtr()->addToAttr( state, &attr, scale );

				if( pFont != attr.pFont || attr.size != oldFontSize )
				{
					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning across different fonts or characters of different size.
				}

				hStyle = pChar->styleHandle();
			}

			// Forward distance with the glyph

			_getGlyphWithoutBitmap( pFont.rawPtr(), pChar->code(), * pGlyph );
			spx charBeg = distance;

			if( pGlyph->advance > 0 )
			{
				distance += pFont->kerning(*pPrevGlyph, *pGlyph);

				distance += pGlyph->advance;
			}
			else if( pChar->code() == 32 )
				distance += pFont->whitespaceAdvance();
			else if (pChar->code() == '\t')
				distance += _tabWidthSPX(pFont->whitespaceAdvance(), distance);

			// Check if we have passed our mark.

			if( distance >= posX )
			{
				switch( mode )
				{
					case SelectMode::ClosestBegin:
					{
						if( posX - charBeg <= distance - posX )
							return int(pChar - pTextBegin);
						else
							return int(pChar+1 - pTextBegin);
					}
					case SelectMode::ClosestEnd:
					{
						if( posX - charBeg < distance - posX )
							return pChar == pTextBegin ? 0 : int(pChar-1 - pTextBegin);
						else
							return int(pChar - pTextBegin);
					}
					break;
					case SelectMode::Closest:
					case SelectMode::Marked:
					{
						return int(pChar - pTextBegin);
					}
				}
			}

			if (pChar->isEndOfLine())
				return int(pChar - pTextBegin);		// We have passed the end of the line, probably in lines cursor-margin.

			std::swap( pPrevGlyph, pGlyph );
			pChar++;
		}

		return -1;			// We should never get here...
	}

} // namespace wg
