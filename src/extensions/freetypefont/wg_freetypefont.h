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
#ifndef WG_FREETYPEFONT_DOT_H
#define WG_FREETYPEFONT_DOT_H
#pragma once

#include <vector>

#include <wg_gfxtypes.h>
#include <wg_chain.h>


#include <wg_font.h>
#include <wg_blob.h>
#include <wg_bitmapcache.h>
#include <wg_gfxbase.h>


typedef struct FT_LibraryRec_  *FT_Library;

struct	FT_FaceRec_;
typedef struct FT_FaceRec_*	FT_Face;
typedef struct  FT_Bitmap_ FT_Bitmap;

namespace wg
{
	class FreeTypeFont;
	typedef	StrongPtr<FreeTypeFont>		FreeTypeFont_p;
	typedef	WeakPtr<FreeTypeFont>	FreeTypeFont_wp;

	class FreeTypeFont : public Font, protected BitmapCacheListener
	{
	public:

		enum class RenderMode
		{
			Monochrome,
			CrispEdges,
			BestShapes
		};

		struct Blueprint
		{
			Font_p			backupFont;
			Blob_p			blob;
			BitmapCache_p	cache;
			int				faceIndex = 0;
			Finalizer_p		finalizer = nullptr;
			RenderMode		renderMode = RenderMode::BestShapes;
			bool			stemDarkening = GfxBase::defaultToSRGB();
			int				xDPI = 72;
			int				yDPI = 72;
		};
		
		
		//.____ Creation __________________________________________

		static FreeTypeFont_p	create( Blob * pTTFBlob );
		static FreeTypeFont_p	create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ______________________________________________________

		bool		setSize( spx size ) override;
		inline spx	size() override { return m_size; }

		void		getGlyphWithoutBitmap(uint16_t chr, Glyph& glyph) override;
		void		getGlyphWithBitmap(uint16_t chr, Glyph& glyph) override;
		spx			kerning(Glyph& leftGlyph, Glyph& rightGlyph) override;

		spx			lineGap() override;
		spx			whitespaceAdvance() override;
		spx			maxAdvance() override;
		spx			maxAscend() override;
		spx			maxDescend() override;

		//.____ Misc ___________________________________________________________

		int			nbGlyphs() override;
		bool		hasGlyphs() override;
		bool		hasGlyph( uint16_t chr ) override;
		bool		isMonospace() override;

		inline BitmapCache_p bitmapCache() { return m_pCache; }
		
		//.____ Appearance ___________________________________________

		inline RenderMode	renderMode() const { return m_renderMode; }


	private:
		FreeTypeFont( const Blueprint& bp );
		~FreeTypeFont();

		class MyGlyph
		{
		public:
			inline void	bitmapLost() { pSurface = nullptr; }
			inline bool isInitialized() { return character != 0; }

			uint16_t	character = 0;			// Unicode for character.
			spx			size = 0;				// size of character.

			spx			advance = 0;
			uint32_t	kerningIndex = 0;		//

			Surface_p 	pSurface;			// Set to null if no bitmap for glyph could be found.
			RectSPX		rect;
			spx			bearingX = 0;			// x offset when rendering the glyph (negated offset to glyph origo)
			spx			bearingY = 0;			// y offset when rendering the glyph (negated offset to glyph origo)
		};

		struct CachedFontSize
		{
			CachedFontSize() 
			{
				whitespaceAdvance = 0;
				for (int i = 0; i < 512; i++)
					page[i] = nullptr;
			}

			~CachedFontSize()
			{
				for (int i = 0; i < 512; i++)
					if( page[i] )
						delete [] page[i];
			}

			spx			whitespaceAdvance;
			MyGlyph*	page[512];
		};

		void 				_cacheSurfaceAdded(Surface* pSurface) override;
		void 				_cacheSurfacesRemoved(int nSurfaces, Surface* pSurfaces[]) override;
		void 				_cacheCleared() override;

		
		void 				_getCacheSlot(int width, int height, MyGlyph* pBitmap);

		void				_copyA8ToA8( const uint8_t * pSrc, int src_width, int src_height, int src_pitch, uint8_t * pDest, int dest_pitch );
		void				_copyA1ToA8( const uint8_t * pSrc, int src_width, int src_height, int src_pitch, uint8_t * pDest, int dest_pitch );
		void				_copyBGRA8ToBGRA8(const uint8_t* pSrc, int src_width, int src_height, int src_pitch, uint8_t* pDest, int dest_pitch);

		void				_generateBitmap( MyGlyph * pGlyph );
		void				_copyBitmap( FT_Bitmap * pBitmap, MyGlyph * pSlot );

		MyGlyph *			_addGlyph( uint16_t ch, spx size, spx advance, uint32_t kerningIndex );
		inline MyGlyph *	_findGlyph( uint16_t glyph, spx size ) const;

		void				_refreshRenderFlags();

		void				_growCachedFontSizes(int newSize);


		FT_Face				m_ftFace;
		Blob_p				m_pFontFile;
		
		BitmapCache_p		m_pCache;

		CachedFontSize **	m_pCachedFontSizes = nullptr;				//	[X sizes][512 codepages][128 glyphs/codepage]
		int					m_nCachedFontSizes = 0;

		uint32_t			m_surfCreationNb = 0;

		int					m_renderFlags;
		RenderMode			m_renderMode;
		spx					m_size;

		int					m_xDPI = 72;
		int					m_yDPI = 72;

		//____ Static stuff __________________________________________________________

		static int 				s_nInstances;		// List of alive FreeTypeFont-objects.

		static FT_Library		s_freeTypeLibrary;
	};

	//____ _findGlyphInIndex() _______________________________________________________

	FreeTypeFont::MyGlyph * FreeTypeFont::_findGlyph( uint16_t ch, spx size ) const
	{
		int sizeOfs = (size+32)/64;

		if( m_pCachedFontSizes[sizeOfs] != nullptr && m_pCachedFontSizes[sizeOfs]->page[ch>>7] != nullptr && m_pCachedFontSizes[sizeOfs]->page[ch >> 7][ch&0x7F].isInitialized() )
			return &m_pCachedFontSizes[sizeOfs]->page[ch >> 7][ch & 0x7F];

		return 0;
	}


} // namespace wg

#endif //WG_FREETYPEFONT_DOT_H
