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
#include <wax_device.h>
#include <wg_util.h>
#include <wax_utf8.h>

#include <algorithm>

namespace wg
{

	const TypeInfo NumberLayout::TYPEINFO = { "NumberLayout", &Object::TYPEINFO };

	using namespace Util;


	//____ typeInfo() _________________________________________________________

	const TypeInfo& NumberLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _renderSegments() ____________________________________________________

	void NumberLayout::_renderSegments(GfxDevice* pDevice, const std::vector<Segment>& segments, const TextStyle * pBaseStyle, CoordSPX pos, int scale, State state) const
	{
		TextAttr		baseAttr;
		pBaseStyle->exportAttr(state, &baseAttr, scale);

		HiColor	baseTint = pDevice->tintColor();

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		Font_p pFont;
		spx fontSize = -1;

		for( auto& segment : segments )
		{
			TextAttr attr = baseAttr;
			if( segment.pStyle )
				segment.pStyle->addToAttr(state, &attr, scale);

			if( pFont != attr.pFont || fontSize != attr.size )
			{
				pFont = attr.pFont;
				fontSize = attr.size;
				pFont->setSize(attr.size);
				pPrevGlyph->advance = 0;					// No kerning across different fonts or character of different size.
			}

			pDevice->setTintColor(baseTint * attr.color);

			const char * p = segment.text.c_str();
			const char * pEnd = p + segment.text.size();

			while( p < pEnd )
			{
				char32_t charCode = Utf8::decodeUnchecked(p);

				if( charCode > 32 )
				{
					pFont->getGlyphWithBitmap(charCode, * pGlyph);

					pos.x += pFont->kerning(* pPrevGlyph, * pGlyph);

					if( pGlyph->pSurface )
					{
						pDevice->setBlitSource(pGlyph->pSurface);
						pDevice->blit(CoordSPX(pos.x + pGlyph->bearingX, pos.y + pGlyph->bearingY), pGlyph->rect);
					}
					pos.x += pGlyph->advance;
				}
				else
				{
					pGlyph->advance = 0;
					if (charCode == 32)
						pos.x += pFont->whitespaceAdvance();
				}

				std::swap( pPrevGlyph, pGlyph );
			}
		}

		pDevice->setTintColor(baseTint);
	}

	//____ _segmentsSizeBaseline() ____________________________________________

	std::tuple<SizeSPX, spx> NumberLayout::_segmentsSizeBaseline(const std::vector<Segment>& segments, const TextStyle * pBaseStyle, int scale, State state) const
	{
		TextAttr		baseAttr;
		pBaseStyle->exportAttr(state, &baseAttr, scale);

		Glyph glyph[2];
		Glyph* pGlyph = &glyph[0];
		Glyph* pPrevGlyph = &glyph[1];

		Font_p pFont;
		spx fontSize = -1;

		spx maxAscend = 0;
		spx maxDescend = 0;
		spx width = 0;

		for( auto& segment : segments )
		{
			TextAttr attr = baseAttr;
			if( segment.pStyle )
				segment.pStyle->addToAttr(state, &attr, scale);

			if( pFont != attr.pFont || fontSize != attr.size )
			{
				pFont = attr.pFont;
				fontSize = attr.size;
				pFont->setSize(attr.size);
				pPrevGlyph->advance = 0;					// No kerning across different fonts or fontsizes.
			}

			maxAscend = std::max(maxAscend, pFont->maxAscend());
			maxDescend = std::max(maxDescend, pFont->maxDescend());

			const char * p = segment.text.c_str();
			const char * pEnd = p + segment.text.size();

			while( p < pEnd )
			{
				char32_t charCode = Utf8::decodeUnchecked(p);

				if( charCode > 32 )
				{
					pFont->getGlyphWithoutBitmap(charCode, *pGlyph);
					width += pFont->kerning(* pPrevGlyph, * pGlyph);
					width += pGlyph->advance;
				}
				else
				{
					pGlyph->advance = 0;
					if (charCode == 32)
						width += pFont->whitespaceAdvance();
				}

				std::swap( pPrevGlyph, pGlyph );
			}
		}

		SizeSPX size( width, maxAscend + maxDescend );
		return  std::make_tuple(Util::alignUp(size),maxAscend);
	}

} // namespace wg
