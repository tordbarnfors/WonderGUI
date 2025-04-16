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

/* TODO: See if we can be better off using multiple size-objects instead of changing size all the time.
   TODO: Separate glyph bitmap from glyph struct, not render or prio cache slot until bitmap is accessed (probably has more to do with Pen).
*/

#include <array>
#include <algorithm>
#include <assert.h>

#include <wg_freetypefont.h>
#include <wg_surface.h>
#include <wg_surfacefactory.h>
#include <wg_gfxutil.h>


#include <ft2build.h>
#include FT_FREETYPE_H


#define SUPPORT_STEM_DARKENING

#ifdef SUPPORT_STEM_DARKENING
#	include FT_PARAMETER_TAGS_H			// If your compiler fails on this line, you likely have an old freetype library that does not support stem darkening. 
										// Comment out define above to compile anyway. Small fonts will look thin/bright and anti-alias won't be as smooth
										// without stem darkening when gamma correction is enabled.
#endif

namespace wg
{
	using namespace Util;

	const TypeInfo FreeTypeFont::TYPEINFO = { "FreeTypeFont", &Font::TYPEINFO };

	int				FreeTypeFont::s_nInstances = 0;

	FT_Library		FreeTypeFont::s_freeTypeLibrary;


	//____ create() ______________________________________________________________

	FreeTypeFont_p FreeTypeFont::create( Blob * pTTFBlob )
	{
		return FreeTypeFont_p( new FreeTypeFont(WGBP(FreeTypeFont, _.blob = pTTFBlob )) );
	}


	FreeTypeFont_p FreeTypeFont::create( const Blueprint& blueprint )
	{
		return FreeTypeFont_p(new FreeTypeFont(blueprint));
	}

	//____ constructor ____________________________________________________________

	FreeTypeFont::FreeTypeFont( const Blueprint& bp ) : Font(bp.backupFont)
	{
        if( s_nInstances == 0 )
        {
            FT_Error err = FT_Init_FreeType(&s_freeTypeLibrary);
            if (err != 0)
            {
                //TODO: Error handling!
            }
        }
        s_nInstances++;

		m_pFontFile = bp.blob;
		m_pCache 	=  bp.cache ? BitmapCache_p(bp.cache) : GfxBase::defaultBitmapCache();
		m_size 			= 0;
		m_xDPI		= bp.xDPI;
		m_yDPI		= bp.yDPI;

//		_growCachedFontSizes(c_maxFontSize);

		FT_Error err = FT_New_Memory_Face(	s_freeTypeLibrary,
											(const FT_Byte *)m_pFontFile->data(),
											m_pFontFile->size(),
											bp.faceIndex,
											&m_ftFace );
		if( err )
		{
			//TODO: Error handling...
		}

		m_renderMode = bp.renderMode;
		_refreshRenderFlags();

		setSize( 10*64 );
		
		// Darken the stem if we have gammaCorrection enabled.

#ifdef SUPPORT_STEM_DARKENING
		if (bp.stemDarkening)
		{
			FT_Parameter         property1;
			FT_Bool              darken_stems = 1;
			property1.tag = FT_PARAM_TAG_STEM_DARKENING;
			property1.data = &darken_stems;

			FT_Parameter         properties[1] = { property1 };

			FT_Face_Properties(m_ftFace, 1, properties);
		}
#endif
		m_pCache->addListener(this);

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ Destructor _____________________________________________________________

	FreeTypeFont::~FreeTypeFont()
	{
		for( int size = 0 ; size < m_nCachedFontSizes ; size++ )
		{
			if (m_pCachedFontSizes[size] != nullptr)
				delete m_pCachedFontSizes[size];
		}

        if(m_pCachedFontSizes != nullptr)
            delete [] m_pCachedFontSizes;

		FT_Done_Face( m_ftFace );

		s_nInstances--;
        if( s_nInstances == 0 )
            FT_Done_FreeType(s_freeTypeLibrary);

		m_pCache->removeListener(this);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FreeTypeFont::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setSize() __________________________________________________________

	bool FreeTypeFont::setSize( spx _size )
	{
		spx size = align(_size);

		if( size == m_size )
			return true;

		if (size < 0)
			return false;

		int pxSize = size/64;

		// Sanity check


		FT_Error err = FT_Set_Char_Size( m_ftFace, size, 0, m_xDPI, m_yDPI );
//		FT_Error err = FT_Set_Pixel_Sizes( m_ftFace, 0, size );
		if( err )
		{
			m_size = 0;
			return false;
		}


		if (pxSize >= m_nCachedFontSizes)
			_growCachedFontSizes(pxSize + 1);

		m_size = size;
		return true;
	}


	//____ _refreshRenderFlags() _______________________________________________

	void FreeTypeFont::_refreshRenderFlags()
	{
		switch( m_renderMode )
		{
			case RenderMode::Monochrome:
				m_renderFlags = FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO;
				break;
			case RenderMode::CrispEdges:
				m_renderFlags = FT_LOAD_TARGET_NORMAL;
				break;
			case RenderMode::BestShapes:
				m_renderFlags = FT_LOAD_TARGET_LIGHT;
				break;

			default:
				break;
		}
	}


	//____ kerning() ___________________________________________________________

	spx FreeTypeFont::kerning( Glyph& leftGlyph, Glyph& rightGlyph )
	{
		if( leftGlyph.advance == 0 || rightGlyph.advance == 0 || leftGlyph.advance == 0 || rightGlyph.advance == 0 )
			return 0;

		// Get kerning info

		FT_Vector	delta;
		delta.x = 0;
		FT_Get_Kerning( m_ftFace, leftGlyph.kerningIndex, rightGlyph.kerningIndex, FT_KERNING_DEFAULT, &delta );

		return int(delta.x);
	}

	//____ whitespaceAdvance() _________________________________________________

	spx FreeTypeFont::whitespaceAdvance()
	{
        int pxSize = m_size/64;
        
		if (m_pCachedFontSizes[pxSize] == nullptr)
		{
			m_pCachedFontSizes[pxSize] = new CachedFontSize();
		}


		if( !m_pCachedFontSizes[pxSize]->whitespaceAdvance )
		{
			FT_Error err;

			// Load whitespace glyph

			err = FT_Load_Char( m_ftFace, ' ', FT_LOAD_RENDER );
			if( err )
				return 0;

			// Get and return advance
			m_pCachedFontSizes[pxSize]->whitespaceAdvance = align(int(m_ftFace->glyph->advance.x));
		}

        return m_pCachedFontSizes[pxSize]->whitespaceAdvance;
	}

	//____ lineGap() ____________________________________________________________

	spx FreeTypeFont::lineGap()
	{
		return align(int(m_ftFace->size->metrics.height - m_ftFace->size->metrics.ascender + m_ftFace->size->metrics.descender));
	}


	//____ maxAscend() ____________________________________________________________

	spx FreeTypeFont::maxAscend()
	{
		return int(m_ftFace->size->metrics.ascender);
	}

	//____ maxDescend() ____________________________________________________________

	spx FreeTypeFont::maxDescend()
	{
		return -int(m_ftFace->size->metrics.descender);
	}


	//____ nbGlyphs() __________________________________________________________

	int FreeTypeFont::nbGlyphs()
	{
		return int(m_ftFace->num_glyphs);
	}

	//____ hasGlyphs() ____________________________________________________________

	bool FreeTypeFont::hasGlyphs()
	{
		return m_ftFace->num_glyphs?true:false;
	}

	//____ isMonospace() __________________________________________________________

	bool FreeTypeFont::isMonospace()
	{
		return FT_IS_FIXED_WIDTH( m_ftFace )>0?true:false;
	}

	//____ maxAdvance() ___________________________________________________

	spx FreeTypeFont::maxAdvance()
	{
		return align(int(m_ftFace->size->metrics.max_advance));
	}


	//____ hasGlyph() _____________________________________________________________

	bool FreeTypeFont::hasGlyph( uint16_t ch )
	{
		int index = FT_Get_Char_Index( m_ftFace, ch );
		if( index == 0 )
			return false;

		return true;
	}

	//____ getGlyphWithoutBitmap() _____________________________________________________________

	void FreeTypeFont::getGlyphWithoutBitmap(uint16_t ch, Glyph& glyph)
	{
		// Get cached glyph if we have one

		MyGlyph * pGlyph = _findGlyph( ch, m_size );
		if( pGlyph == 0 )
		{
			FT_Error err;

			// Load MyGlyph

			FT_UInt char_index = FT_Get_Char_Index( m_ftFace, ch );
			if( char_index == 0 )
            {
                // Glyph is missing, try to find it in backup font.

				if (!m_pBackupFont)
					goto no_glyph;

				if (m_pBackupFont->size() == m_size)
					m_pBackupFont->getGlyphWithoutBitmap(ch, glyph);
				else
				{
					spx sz = m_pBackupFont->size();
					m_pBackupFont->setSize(m_size);
					m_pBackupFont->getGlyphWithoutBitmap(ch, glyph);
					m_pBackupFont->setSize(sz);
				}
	            return;            	
			}

			err = FT_Load_Glyph( m_ftFace, char_index, m_renderFlags );
			if (err)
				goto no_glyph;

			// Get some details about the glyph

			spx advance = align(int(m_ftFace->glyph->advance.x));

			// Get a MyGlyph object and fill in details

			pGlyph = _addGlyph( ch, m_size, advance, char_index );
		}

		glyph.bearingX		= pGlyph->bearingX;
		glyph.bearingY		= pGlyph->bearingY;
		glyph.advance		= pGlyph->advance;
		glyph.fontRef		= this;
		glyph.kerningIndex	= pGlyph->kerningIndex;
		return;

	no_glyph:
		glyph.bearingX		= 0;
		glyph.bearingY		= 0;
		glyph.advance = 0;
		glyph.fontRef = nullptr;
		glyph.kerningIndex = 0;
		return;

	}

	//____ getGlyphWithBitmap() ______________________________________________

	void FreeTypeFont::getGlyphWithBitmap(uint16_t ch, Glyph& glyph)
	{
		// Get cached glyph if we have one

		MyGlyph* pGlyph = _findGlyph(ch, m_size);
		if (pGlyph == 0)
		{
			FT_Error err;

			// Load MyGlyph

			FT_UInt char_index = FT_Get_Char_Index(m_ftFace, ch);
			if (char_index == 0)
			{
				// Glyph is missing, try to find it in backup font.

				if (!m_pBackupFont)
					goto no_glyph;

				if (m_pBackupFont->size() == m_size)
					m_pBackupFont->getGlyphWithBitmap(ch, glyph);
				else
				{
					spx sz = m_pBackupFont->size();
					m_pBackupFont->setSize(m_size);
					m_pBackupFont->getGlyphWithBitmap(ch, glyph);
					m_pBackupFont->setSize(sz);
				}
				return;
			}

			err = FT_Load_Glyph(m_ftFace, char_index, m_renderFlags);
			if (err)
				goto no_glyph;

			// Get some details about the glyph

			spx advance = align(int(m_ftFace->glyph->advance.x));

			// Get a MyGlyph object and fill in details

			pGlyph = _addGlyph(ch, m_size, advance, char_index);
		}

		glyph.fontRef = this;
		glyph.advance = pGlyph->advance;
		glyph.kerningIndex = pGlyph->kerningIndex;

		if (pGlyph->pSurface == nullptr)
			_generateBitmap(pGlyph);

		glyph.pSurface = pGlyph->pSurface;
		glyph.rect = pGlyph->rect;
		glyph.bearingX = pGlyph->bearingX;
		glyph.bearingY = pGlyph->bearingY;
		return;

	no_glyph:
		glyph.fontRef = 0;
		glyph.advance = 0;
		glyph.kerningIndex = 0;
		glyph.pSurface = nullptr;
		return;
	}



	//____ _generateBitmap() _______________________________________________________

	void FreeTypeFont::_generateBitmap( MyGlyph * pGlyph )
	{
		FT_Error err;

        //

        bool bDifferentSize = pGlyph->size != m_size;

        if( bDifferentSize )
             FT_Set_Char_Size( m_ftFace, align(pGlyph->size), 0, 0,0 );

		// Load MyGlyph

		err = FT_Load_Glyph( m_ftFace, pGlyph->kerningIndex, FT_LOAD_RENDER | FT_LOAD_COLOR | m_renderFlags );
		if( err )
        {
			//TODO: Error handling!
			
            if( bDifferentSize )
                FT_Set_Char_Size( m_ftFace, m_size, 0, 0,0 );
            return;
        }

		// Get some details about the glyph

		int width = m_ftFace->glyph->bitmap.width;
		int height = m_ftFace->glyph->bitmap.rows;

		// Get a cache slot

		_getCacheSlot( width, height, pGlyph );

		pGlyph->bearingX = m_ftFace->glyph->bitmap_left*64;
		pGlyph->bearingY = -m_ftFace->glyph->bitmap_top*64;


		_copyBitmap( &m_ftFace->glyph->bitmap, pGlyph );	// Copy our glyph bitmap to the slot

        if( bDifferentSize )
            FT_Set_Char_Size( m_ftFace, m_size, 0, 0,0 );
	}


	//____ _copyBitmap() ____________________________________________________________

	void FreeTypeFont::_copyBitmap( FT_Bitmap * pFTBitmap, MyGlyph * pGlyph )
	{
		Surface_p pSurf = pGlyph->pSurface;

		auto pixbuf = pSurf->allocPixelBuffer(pGlyph->rect/64);

		assert( pSurf->pixelFormat() == PixelFormat::Alpha_8);

		// Copy glyph bitmap into alpha channel of slot, making sure to clear any
		// left over area of slots alpha channel.


		switch( pFTBitmap->pixel_mode )
		{
			case (FT_PIXEL_MODE_MONO):
				_copyA1ToA8( pFTBitmap->buffer, pFTBitmap->width, pFTBitmap->rows, pFTBitmap->pitch, pixbuf.pixels, pixbuf.pitch );
				break;
			case (FT_PIXEL_MODE_GRAY):
				_copyA8ToA8( pFTBitmap->buffer, pFTBitmap->width, pFTBitmap->rows, pFTBitmap->pitch, pixbuf.pixels, pixbuf.pitch );
				break;
			case (FT_PIXEL_MODE_BGRA):
				_copyBGRA8ToBGRA8(pFTBitmap->buffer, pFTBitmap->width, pFTBitmap->rows, pFTBitmap->pitch, pixbuf.pixels, pixbuf.pitch);
				break;
			default:
				assert( false );
		}

		pSurf->pullPixels(pixbuf);
		pSurf->freePixelBuffer(pixbuf);
	}


	//____ _copyA8ToA8() _____________________________________________________

	void FreeTypeFont::_copyA8ToA8( const uint8_t * pSrc, int src_width, int src_height, int src_pitch,
										uint8_t * pDest, int dest_pitch )
	{
		for( int y = 0 ; y < src_height ; y++ )
		{
			for( int x = 0 ; x < src_width ; x++ )
				pDest[x] = pSrc[x];

			pSrc  += src_pitch;
			pDest += dest_pitch;
		}

	}

	//____ _copyA1ToA8() _____________________________________________________

	void FreeTypeFont::_copyA1ToA8( const uint8_t * pSrc, int src_width, int src_height, int src_pitch,
										uint8_t * pDest, int dest_pitch )
	{
		uint8_t lookup[2] = { 0, 255 };

		for( int y = 0 ; y < src_height ; y++ )
		{
			for( int x = 0 ; x < src_width ; x++ )
				pDest[x] = lookup[(((pSrc[x>>3])<<(x&7))&0xFF)>>7];
		
			pSrc  += src_pitch;
			pDest += dest_pitch;
		}
	}

	//____ _copyBGRA8ToBGRA8() _____________________________________________________

	void FreeTypeFont::_copyBGRA8ToBGRA8(const uint8_t* pSrc, int src_width, int src_height, int src_pitch,
		uint8_t* pDest, int dest_pitch)
	{
		for (int y = 0; y < src_height; y++)
		{
			for (int x = 0; x < src_width*4; x++)
				pDest[x] = pSrc[x];

			pSrc += src_pitch;
			pDest += dest_pitch;
		}

	}


	//___ _addGlyph() ________________________________________________________

	FreeTypeFont::MyGlyph * FreeTypeFont::_addGlyph( uint16_t ch, spx size, spx advance, uint32_t kerningIndex )
	{
		int szOfs = size/64;

		if (m_pCachedFontSizes[szOfs] == nullptr)
			m_pCachedFontSizes[szOfs] = new CachedFontSize();

		if (m_pCachedFontSizes[szOfs]->page[ch >> 7] == nullptr)
			m_pCachedFontSizes[szOfs]->page[ch >> 7] = new MyGlyph[128];

		assert( !m_pCachedFontSizes[szOfs]->page[ch>>7][ch & 0x7F].isInitialized() );

		auto pGlyph = &m_pCachedFontSizes[szOfs]->page[ch >> 7][ch & 0x7F]; 
		
		pGlyph->character = ch;
		pGlyph->size = size;
		pGlyph->advance = advance;
		pGlyph->kerningIndex = kerningIndex;
		
		return &m_pCachedFontSizes[szOfs]->page[ch >> 7][ch & 0x7F];
	}

	//____ _cacheCleared() _____________________________________________

	void FreeTypeFont::_cacheCleared()
	{
		for (int size = 0; size < m_nCachedFontSizes; size++)
		{
			if (m_pCachedFontSizes[size] != nullptr)
			{
				auto pPages = m_pCachedFontSizes[size]->page;

				for (int page = 0; page < 512; page++)
				{
					if (pPages[page] != nullptr)
					{
						auto pGlyphs = pPages[page];
						for( int glyph = 0 ; glyph < 128 ; glyph++ )
							pGlyphs[glyph].bitmapLost();
					}
				}
			}
		}
	}

	//____ _cacheSurfaceAdded() __________________________________________________

	void FreeTypeFont::_cacheSurfaceAdded(Surface* pSurface)
	{
		// Do nothing.
	}

	//____ _cacheSurfacesRemoved() _____________________________________________

	void FreeTypeFont::_cacheSurfacesRemoved( int nRemovedSurfaces, Surface * pRemovedSurfaces[] )
	{
		auto pSizes = m_pCachedFontSizes;

		for (int size = 0; size < m_nCachedFontSizes; size++)
		{
			if (m_pCachedFontSizes[size] != nullptr)
			{
				auto pPages = m_pCachedFontSizes[size]->page;

				for (int page = 0; page < 512; page++)
				{
					if (pPages[page] != nullptr)
					{
						auto pGlyphs = pPages[page];
						for (int glyph = 0; glyph < 128; glyph++)
						{
							Surface* pGlyphSurf = pGlyphs[glyph].pSurface;
							if (pGlyphSurf)
							{
								for (int i = 0; i < nRemovedSurfaces; i++)
								{
									if (pGlyphSurf == pRemovedSurfaces[i])
									{
										pGlyphs[glyph].bitmapLost();
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}


	//____ _getCacheSlot() _________________________________________________________

	void FreeTypeFont::_getCacheSlot( int width, int height, MyGlyph * pGlyph )
	{
		Surface_p	pSurface;
		CoordI		ofs;
				
		std::tie(pSurface,ofs) = m_pCache->getCacheSlot( {width,height} );
				
		pGlyph->pSurface = pSurface;
		pGlyph->rect.x = ofs.x*64;
		pGlyph->rect.y = ofs.y*64;
		pGlyph->rect.w = width*64;
		pGlyph->rect.h = height*64;
	}

	//____ _growCachedFontSizes() ________________________________________________

	void FreeTypeFont::_growCachedFontSizes(int newSize)
	{
		auto pNew = new CachedFontSize*[newSize];

		for (int i = 0; i < m_nCachedFontSizes; i++)
			pNew[i] = m_pCachedFontSizes[i];

		for (int i = m_nCachedFontSizes; i < newSize; i++)
			pNew[i] = nullptr;

		delete [] m_pCachedFontSizes;
		m_pCachedFontSizes = pNew;
		m_nCachedFontSizes = newSize;
	}

/*
	FreeTypeFont::MyGlyph::MyGlyph( uint16_t character, spx size, spx advance, uint32_t kerningIndex, Font * pFont )
	: Glyph( advance, kerningIndex, pFont )
	{
		m_size = size;
		m_character = character;
	}
*/

} // namespace wg
