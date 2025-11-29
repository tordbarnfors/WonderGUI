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
#include <algorithm>

#include <wg2_gfxdevice.h>
#include <wg2_base.h>

#include <wg_surface.h>
#include <wg2_geo.h>
#include <wg2_blockset.h>
#include <wg2_text.h>
#include <wg2_cursorinstance.h>
#include <wg_font.h>
#include <wg2_gfxanim.h>
#include <wg2_util.h>
#include <wg2_pen.h>
#include <wg_util.h>


using namespace wg::Util;

//____ BlitBlock() ________________________________________________________

void WgGfxDevice::BlitBlock( wg::GfxDevice * pDevice, const WgBlock& _block, const WgRect& _canvas, int scale, bool bTriLinear, float mipmapbias)
{
	if( !_block.Surface() )
		return;

	if( _block.IsSkipable() )
		return;

	const WgBorders&    sourceBorders = _block.SourceFrame();
	const WgBorders     canvasBorders = _block.CanvasFrame();

	// Shortcuts & optimizations for common special cases.

	WgRect canvas = _canvas;
	WgUtil::AdjustScaledArea(_block, canvas);

	const WgRect& src = _block.Rect();
	wg::Surface * pSurf = _block.Surface();

	pDevice->setBlitSource(pSurf);
    
	int srcScale =_block.Surface()->scale();
	
    wg::NinePatch patch;
    patch.block = wg::Rect(src)*64/srcScale;
    patch.frame = wg::Border(sourceBorders)*64/srcScale;
	pDevice->blitNinePatch(canvas*64, canvasBorders, patch, scale >> 6 );

}


//____ PrintText() ____________________________________________________________

bool WgGfxDevice::PrintText( wg::GfxDevice * pDevice, const WgText * pText, const WgRect& dest )
{
	if( !pText || dest.w <= 0 )
		return false;

	_drawTextBg(pDevice,pText, dest);

	WgPen pen;
	pen.SetDevice( pDevice );
	pen.SetScale( pText->Scale() );

	wg::TextAttr	attr;
	pText->GetBaseAttr(attr);

	if( attr.pFont == 0 )
		return false;

	pen.SetAttributes(attr);

	WgSize	textSize( pText->width(), pText->height() );

	// Limit cliplist if we might print outside our dest

	ClipPopData pop;

	if( textSize.w > dest.w || textSize.h > dest.h )
	   pop = limitClipList( pDevice, dest*64 );

//	if( dest.h < (int) textSize.h || dest.w < (int) textSize.w || !clip.contains( dest ) || pText->isCursorShowing() )

	const WgCursorInstance* pCursor = 0;
	int cursLine = -1, cursCol = -1;

	if( pText->isCursorShowing() )
	{
		pCursor = pText->GetCursor();
		pCursor->getSoftPos( cursLine, cursCol );
	}

	bool bEllipsisActive = false;
	if( pText->IsAutoEllipsis() && !pText->isCursorShowing() && (textSize.w > dest.w || textSize.h > dest.h) )
		bEllipsisActive = true;

	WgCoord	pos;
	pos.y = pText->LineStartY( 0, dest ) + pText->getSoftLine(0)->baseline;

	int					nLines = pText->nbSoftLines();
	const WgTextLine *	pLines = pText->getSoftLines();

	for( int i = 0 ; i < nLines ; i++ )
	{
		pos.x = pText->LineStartX( i, dest );
		pen.SetOrigo( pos );		// So tab positions will start counting from start of line.
		pen.SetPos( pos );
		pen.FlushChar();			// Remove kerning info for previous char.

		bool bLastFullyVisibleLine = false;
		if( (i < nLines-1) && (pos.y + pLines[i].lineSpacing + pLines[i+1].height - pLines[i+1].baseline > dest.y + dest.h) )
			bLastFullyVisibleLine = true;

		if( cursLine == i )
		{
			// Draw line parts, make space for cursor.

			_printTextSpan( pDevice, pen, pText, pLines[i].ofs, cursCol, false );
			WgCoord cursorPos = pen.GetPos();
			pen.AdvancePosCursor( *pCursor );
			_printTextSpan( pDevice, pen, pText, pLines[i].ofs + cursCol, pLines[i].nChars - cursCol, true );

			// Blit the cursor

			WgCoord restorePos = pen.GetPos();
			pen.SetPos( cursorPos );
			pen.BlitCursor( *pCursor );
			pen.SetPos( restorePos );
		}
		else
		{
			if( bEllipsisActive && (bLastFullyVisibleLine || pLines[i].width > dest.w) )
			{
				_printEllipsisTextSpan( pDevice, pen, pText, pLines[i].ofs, pLines[i].nChars, dest.x + dest.w );
				break;
			}
			else
				_printTextSpan( pDevice, pen, pText, pLines[i].ofs, pLines[i].nChars, true );
		}

		pos.y += pLines[i].lineSpacing;
	}

	popClipList( pDevice, pop );

	if( dest.w >= textSize.w && (dest.h >= textSize.h || nLines == 1) )
		return true;
	else
		return false;
}

//____ _printTextSpan() ________________________________________________________

void WgGfxDevice::_printTextSpan( wg::GfxDevice * pDevice, WgPen& pen, const WgText * pText, int ofs, int len, bool bLineEnding )
{
	WgColor baseCol	= pDevice->tintColor();
	WgColor	color	= baseCol;

	const wg::Char * pChars = pText->getText();
	Uint16	hStyle	= 0xFFFF;		// Setting to impossible value forces setting of properties in first loop.
	wg::TextAttr		attr;

	WgRange	selection = pText->getSelection();

	// Print loop

	for( int i = ofs ; i < ofs + len ; i++ )
 	{
		// Act on possible change of character attributes.

		if( pChars[i].styleHandle() != hStyle || i == selection.Begin() || i == selection.End() )
		{
			bool bWasUnderlined = attr.decoration == wg::TextDecoration::Underline;

			hStyle = pChars[i].styleHandle();

			pText->GetCharAttr( attr, i );
			pen.SetAttributes(attr);
			if( !pen.GetFont() )
				return;											// Better not to print than to crash...

			// Set tint colors (if changed)

			if( pen.GetColor() != color )
			{
				color = pen.GetColor();
				pDevice->setTintColor( baseCol * color );
			}

			// Check if this is start of underlined text and in that case draw the underline.

			if( attr.decoration == wg::TextDecoration::Underline && (i==0 || !bWasUnderlined) )
			{
				_drawUnderline( pDevice, pText, pen.GetPosX(), pen.GetPosY(), i, (ofs+len)-i );
			}

		}

		// Calculate position and blit the glyph.

		Uint16 ch = pChars[i].code();

		bool bBlit = pen.SetChar( ch );
		pen.ApplyKerning();
		if( bBlit )
		{

/*			if(selStartX == -1 && i >= iSelStart)
				selStartX = pen.GetBlitPosX();

			if(selStartX >= 0 && i < iSelEnd)
				selEndX = pen.GetBlitPosX();
*/
			pen.BlitChar();
		}

		pen.AdvancePos();

 	}

	// Render line-endings.

	if( bLineEnding )
	{
		// If character after line-end was a WgExtChar::HyphenBreakPermitted we need
		// to render a normal hyphen.

		if( pChars[ofs+len].code() == int(WgExtChar::HyphenBreakPermitted) )
		{
			if( pen.SetChar( '-' ) )
			{
				pen.ApplyKerning();
				pen.BlitChar();
			}
		}

		// TODO: print LF-character if there is one following and properties says it should be displayed.

	}

	// Restore tint color.

	if( pDevice->tintColor() != wg::HiColor(baseCol) )
		pDevice->setTintColor(baseCol);
}


//____ _printEllipsisTextSpan() ________________________________________________________

void WgGfxDevice::_printEllipsisTextSpan( wg::GfxDevice * pDevice, WgPen& pen, const WgText * pText, int ofs, int len, int endX )
{
	WgColor baseCol	= pDevice->tintColor();
	WgColor	color	= baseCol;

	const wg::Char * pChars = pText->getText();
	Uint16	hStyle	= 0xFFFF;		// Setting to impossible value forces setting of properties in first loop.
	wg::TextAttr		attr;
	wg::TextAttr		baseAttr;

	WgRange	selection = pText->getSelection();
	int		ellipsisWidth = 0;


	// Get the width of an ellipsis

	pText->GetBaseAttr( baseAttr );	// Ellipsis are always rendered using the base attributes.
	pen.SetAttributes( baseAttr );

	Uint16	ellipsisChar = int(WgExtChar::Ellipsis);
	ellipsisWidth = 0;
	wg::Glyph ellipsis;
	pen.GetFont()->getGlyphWithBitmap( int(WgExtChar::Ellipsis), ellipsis );

	if( !ellipsis.fontRef )
	{
		pen.GetFont()->getGlyphWithBitmap( '.', ellipsis );
		ellipsisChar = '.';
	}

	if( ellipsis.pSurface )
	{
		if( ellipsisChar == int(WgExtChar::Ellipsis) )
			ellipsisWidth = ellipsis.rect.w/64 + ellipsis.bearingX/64;
		else
			ellipsisWidth = ellipsis.advance/64*2 + ellipsis.rect.w/64 + ellipsis.bearingX/64;
	}

	// Print loop

	for( int i = ofs ; i < ofs + len ; i++ )
 	{
		// Act on possible change of character attributes.

		if( pChars[i].styleHandle() != hStyle || i == selection.Begin() || i == selection.End() )
		{
			bool bWasUnderlined = attr.decoration == wg::TextDecoration::Underline;

			hStyle = pChars[i].styleHandle();

			pText->GetCharAttr( attr, i );
			pen.SetAttributes(attr);
			if( !pen.GetFont() )
				return;											// Better not to print than to crash...

			// Set tint colors (if changed)

			if( pen.GetColor() != color )
			{
				color = pen.GetColor();
				pDevice->setTintColor( baseCol * color );
			}

			// Check if this is start of underlined text and in that case draw the underline.

			if( attr.decoration == wg::TextDecoration::Underline && (i==0 || !bWasUnderlined) )
			{
				_drawUnderline( pDevice, pText, pen.GetPosX(), pen.GetPosY(), i, (ofs+len)-i );
			}
		}

		// Calculate position and blit the glyph.

		Uint16 ch = pChars[i].code();

		bool bBlit = pen.SetChar( ch );

		WgCoord savedPos = pen.GetPos();
		pen.ApplyKerning();
		wg::Glyph glyph = pen.GetGlyph();
        if( pen.GetPosX() +  glyph.advance/64 + ellipsisWidth > endX )
		{
			pen.SetPos( savedPos );
			break;
		}

		if( bBlit )
			pen.BlitChar();

		pen.AdvancePos();

 	}

	// Render ellipsis.

	pen.SetAttributes(baseAttr);		// Ellipsis are always rendered using the base attributes.
	pen.SetChar( ellipsisChar );

	// Set tint colors (if changed)

	if( pen.GetColor() != color )
	{
		color = pen.GetColor();
		pDevice->setTintColor( baseCol * color );
	}

	if( ellipsisChar == '.' )
	{
		pen.SetChar( ellipsisChar );		// Set once more to get kerning between dots to work.
		pen.BlitChar();
		pen.AdvancePos();
		pen.ApplyKerning();
		pen.BlitChar();
		pen.AdvancePos();
		pen.ApplyKerning();
		pen.BlitChar();
	}
	else
		pen.BlitChar();						// We could have kerning here but we have screwed up previous glyph...

	// Restore tint color.

	if( pDevice->tintColor() != wg::HiColor(baseCol) )
		pDevice->setTintColor(baseCol);
}


//____ _drawTextBg() ___________________________________________________________

void WgGfxDevice::_drawTextBg( wg::GfxDevice * pDevice, const WgText * pText, const WgRect& dest )
{
	wg::State state = pText->state();

	// Take care of selection background color (if we have any)

	int selStart, selEnd;

	int startLine, startCol, endLine, endCol;
	pText->getSelection( startLine, startCol, endLine, endCol );
	selStart = pText->LineColToOffset(startLine, startCol);
	selEnd = pText->LineColToOffset(endLine,endCol);

	wg::TextStyle * pStyle = pText->getStyle();
	if( !pStyle )
		pStyle = WgBase::defaultStyle();

    wg::Color selectionBgColor = pStyle->backColor(state + wg::State::Selekted);
    
	if( selStart != selEnd && selectionBgColor.a != 0 )
	{
		_drawTextSectionBg( pDevice, pText, dest, selStart, selEnd, selectionBgColor );
	}
	else
	{
		selStart = -1;
		selEnd = -1;
	}

	// Scan through the text, drawing character specific backgrounds
	// (and in the future punching holes in the general background?)

	Uint16	hStyle = 0xFFFF;
	WgColor	color;
	int		startOfs = 0;

	const wg::Char * pChars = pText->getText();
	int nChars = pText->nbChars();

	for( int ofs = 0 ; ofs < nChars ; ofs++ )
	{
		if( ofs == selStart )
		{
			if( color.a != 0 )
				_drawTextSectionBg( pDevice, pText, dest, startOfs, ofs, color );

			startOfs = selEnd;			//TODO: We should not skip rendering background color, there migth be on link or character (or selection might not have one).
			ofs = startOfs;
		}

		if( pChars[ofs].styleHandle() != hStyle )
		{
			// Update hProp and get background color

			hStyle = pChars[ofs].styleHandle();

			WgColor newColor = pText->GetCharBgColor(ofs);

			if( newColor != color )
			{
				// Draw previous bg section which now ended

				if( ofs != startOfs && color.a != 0 )
					_drawTextSectionBg( pDevice, pText, dest, startOfs, ofs, color );

				// Set start and color of current background section

				color = newColor;
				startOfs = ofs;
			}

		}
	}

	// Draw last background section if it is colored

	if( startOfs != nChars && color.a != 0 )
		_drawTextSectionBg( pDevice, pText, dest, startOfs, nChars, color );
}

//____ _drawTextSectionBg() ___________________________________________________

void WgGfxDevice::_drawTextSectionBg( wg::GfxDevice * pDevice, const WgText * pText, const WgRect& dstRect,
									  int iStartOfs, int iEndOfs, WgColor color )
{
	const WgTextLine *	pLines = pText->getSoftLines();

	WgTextPos startPos = pText->OfsToPos( iStartOfs );
	WgTextPos endPos = pText->OfsToPos( iEndOfs );

	int xs = pText->PosToCoordX( startPos, dstRect );
	int xe = pText->PosToCoordX( endPos, dstRect );

	int dstPosY = pText->LineStartY( 0, dstRect );

	WgRect r;

	if(startPos.line == endPos.line)
	{
		r.x = xs;
		r.y = dstPosY + pText->getLineOfsY(startPos.line);
		r.w = xe - xs;
		r.h = pLines[startPos.line].height;
		pDevice->fill(wg::RectI::overlap(r, dstRect)*64, color);
	}
	else
	{
		r.x = xs;
		r.y = dstPosY + pText->getLineOfsY(startPos.line);
		r.w = pText->LineStartX(startPos.line, dstRect) + pText->getSoftLineSelectionWidth(startPos.line) - xs;
		r.h = pLines[startPos.line].height;
		pDevice->fill(wg::RectI::overlap(r, dstRect)*64, color);
		r.y += pLines[startPos.line].lineSpacing;

		++startPos.line;
		for(; startPos.line < endPos.line; ++startPos.line)
		{
			r.x = pText->LineStartX( startPos.line, dstRect );
			r.w = pText->getSoftLineSelectionWidth(startPos.line);
			r.h = pLines[startPos.line].height;
			pDevice->fill(wg::RectI::overlap(r,dstRect)*64, color);
			r.y += pLines[startPos.line].lineSpacing;
		}

		r.x = pText->LineStartX( startPos.line, dstRect );
		r.w = xe - r.x;
		r.h = pLines[startPos.line].height;
		pDevice->fill(wg::RectI::overlap(r,dstRect)*64, color);
	}
}

//____ PrintLine() ________________________________________________________

void WgGfxDevice::PrintLine( wg::GfxDevice * pDevice, WgPen& pen, const wg::TextAttr& baseAttr, const wg::Char * _pLine, int maxChars, wg::State state )
{
	if( !_pLine )
		return;

	WgColor baseCol	= pDevice->tintColor();
	WgColor	color	= baseCol;

	Uint16	hStyle		= 0xFFFF;		// Setting to impossible value forces setting of properties in first loop.
	wg::TextAttr	attr;

	pen.FlushChar();

	// Print loop

	int i;
	for( i = 0 ; i < maxChars && !_pLine[i].isEndOfLine(); i++ )
 	{
		// Act on possible change of character attributes.

		if( _pLine[i].styleHandle() != hStyle )
		{

			attr = baseAttr;

			if(_pLine[i].styleHandle())
				_pLine[i].stylePtr()->addToAttr(state, &attr, pen.GetScale() >> 6);

			hStyle = _pLine[i].styleHandle();

			pen.SetAttributes( attr );
			if( !pen.GetFont() )
				return;											// No glyphset, better to leave than to crash...

			// Set tint colors (if changed)

			if( pen.GetColor() != color )
			{
				color = pen.GetColor();
				pDevice->setTintColor( baseCol * color );
			}

			// Check if this is start of underlined text and in that case draw the underline.
/*
		TODO: Figure out how to do this properly, taking mode and char-props correctly into account.

			if( attr.bUnderlined && (i==0 || !bWasUnderlined) )
			{
				WgRect clip = pen.HasClipRect()?pen.GetClipRect():WgRect(0,0,65535,65535);
				DrawUnderline( clip, pText, pen.GetPosX(), pen.GetPosY(), i, len-i );
			}
*/
		}

		// Calculate position and blit the glyph.

		Uint16 ch = _pLine[i].code();

		bool bBlit = pen.SetChar( ch );
		pen.ApplyKerning();
		if( bBlit )
		{

/*			if(selStartX == -1 && i >= iSelStart)
				selStartX = pen.GetBlitPosX();

			if(selStartX >= 0 && i < iSelEnd)
				selEndX = pen.GetBlitPosX();
*/
			pen.BlitChar();
		}

		pen.AdvancePos();

 	}

	// Restore tint color.

	if( pDevice->tintColor() != wg::HiColor(baseCol) )
		pDevice->setTintColor(baseCol);
}

//____ _drawUnderline() ________________________________________________________

void WgGfxDevice::_drawUnderline( wg::GfxDevice * pDevice, const WgText * pText, int _x, int _y, int ofs, int maxChars )
{

	Uint32 hStyle = 0xFFFF;

	WgPen pen;
	const wg::Char * pChars = pText->getText();
	pen.SetScale( pText->Scale() );

	for( int i = ofs ; i < ofs + maxChars && !pChars[i].isEndOfLine() ; i++ )
	{
		if( pChars[i].styleHandle() != hStyle )
		{
			wg::TextAttr attr;
			pText->GetCharAttr( attr, i );

			if( attr.decoration == wg::TextDecoration::Underline )
			{
				hStyle = pChars[i].styleHandle();
				pen.SetAttributes( attr );
			}
			else
				break;
		}

		pen.SetChar( pChars[i].code() );
		pen.ApplyKerning();
		pen.AdvancePos();
	}

	// Niklas is almost certain we don't use underline anywhere so I'll skip migrating this.
/*
	const WgUnderline * pUnderline = pen.GetFont()->GetUnderline( pen.GetSize() );

	auto p = pDevice->blitSource();
	pDevice->setBlitSource(pUnderline->pSurf);
	pDevice->blitHorrBar(pUnderline->rect, WgBorders( 0, pUnderline->rightBorder, 0, pUnderline->leftBorder ), false, WgCoord(_x + pUnderline->bearingX, _y + pUnderline->bearingY), pen.GetPosX());
	pDevice->setBlitSource(p);
*/
}
