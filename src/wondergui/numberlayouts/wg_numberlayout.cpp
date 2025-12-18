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
#include <wg_numberlayout.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>

namespace wg
{

	const TypeInfo NumberLayout::TYPEINFO = { "NumberLayout", &Object::TYPEINFO };

	using namespace Util;


	//____ typeInfo() _________________________________________________________

	const TypeInfo& NumberLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _renderString() ____________________________________________________

	void NumberLayout::_renderString(GfxDevice* pDevice, const Char* pChar, TextStyle* pStyle, CoordSPX pos, int scale, State state)
	{
		TextAttr		baseAttr;
		pStyle->exportAttr(state, &baseAttr, scale);

		TextAttr		attr;
		Font_p 			pFont;
		TextStyle_h		hStyle = 0xFFFF;

		HiColor	baseTint = pDevice->tintColor();
		HiColor	localTint = HiColor::White;

		BlendMode renderMode = pDevice->blendMode();

		// Render back colors

//		_renderBack(pText, pDevice, canvas);

		//

		const RectSPX& clip = pDevice->clipBounds();

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		bool bRecalcColor = false;
		bool bInSelection = false;

		while( !pChar->isEndOfText() )
		{
			// TODO: Include handling of special characters
			// TODO: Support char textcolor and background color and effects.

			if (pChar->styleHandle() != hStyle)
			{
				spx oldFontSize = attr.size;
				HiColor oldBackColor = attr.backColor;

				attr = baseAttr;

				if (pChar->styleHandle() != 0)
					pChar->stylePtr()->addToAttr(state, &attr, scale);

				if (pFont != attr.pFont || attr.size != oldFontSize)
				{

					if (!pFont || (pFont->isMonochrome() != attr.pFont->isMonochrome()))
						bRecalcColor = true;		// Font tint-color is changed.

					pFont = attr.pFont;
					pFont->setSize(attr.size);
					pPrevGlyph->advance = 0;								// No kerning across different fonts or character of different size.
				}

				if (attr.color != localTint)
				{
					localTint = attr.color;
					bRecalcColor = true;
				}
			}

			//
/*
			if (pChar == pSelBeg)
			{
				bInSelection = true;
				bRecalcColor = true;
			}
			else if (pChar == pSelEnd)
			{
				bInSelection = false;
				bRecalcColor = true;
			}
*/
			if (bRecalcColor)
			{
//				if (bInSelection)
//					pDevice->setTintColor(baseTint * HiColor::blend(localTint, m_selectionCharColor, m_selectionCharBlend));
//				else
					pDevice->setTintColor(baseTint * localTint);

				bRecalcColor = false;
			}

			//

			uint16_t charCode = pChar->code();
			pFont->getGlyphWithBitmap(charCode, * pGlyph);

			if (pGlyph && charCode > 32)
			{
				pos.x += pFont->kerning(* pPrevGlyph, * pGlyph);

				pDevice->setBlitSource(pGlyph->pSurface);
				pDevice->blit(CoordSPX(pos.x + pGlyph->bearingX, pos.y + pGlyph->bearingY), pGlyph->rect);
				pos.x += pGlyph->advance;
			}
			else if (pChar->code() == 32)
				pos.x += pFont->whitespaceAdvance();

			std::swap( pPrevGlyph, pGlyph );
			pChar++;
		}

		pDevice->setTintColor(baseTint);


		// Render caret (if there is any)
/*
		if (_caretVisible(pText))
		{
			Caret* pCaret = m_pCaret ? m_pCaret : Base::defaultCaret();

			if (pCaret)
			{
				pCaret->render(pDevice, charRect(pText, _caretOfs(pText)) + canvas.pos(), _scale(pText));
			}
		}
*/
	}


} // namespace wg
