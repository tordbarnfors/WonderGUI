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
#ifndef WG2_GFXANIM_DOT_H
#define WG2_GFXANIM_DOT_H

#ifndef WG2_ANIM_DOT_H
#	include <wg2_anim.h>
#endif

#ifndef	WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#ifndef WG2_BLOCKSET_DOT_H
#	include <wg2_blockset.h>
#endif

#include <wg_surface.h>

class WgGfxFrame;


#define MAX_ANIM_ALT 5

class WgBlock;

enum WgBlockFlags
{
	WG2_OPAQUE_CENTER    = 0x001,
	WG2_OPAQUE            = 0x003,

	WG2_TILE_CENTER        = 0x004,
	WG2_SCALE_CENTER        = 0x008,
	WG2_FIXED_CENTER        = 0x010,

	WG2_TILE_LEFT        = 0x020,
	WG2_TILE_RIGHT        = 0x040,
	WG2_TILE_TOP            = 0x080,
	WG2_TILE_BOTTOM        = 0x100,



	WG2_TILE_BORDERS        = WG2_TILE_LEFT | WG2_TILE_RIGHT | WG2_TILE_TOP | WG2_TILE_BOTTOM,
	WG2_TILE_ALL            = WG2_TILE_BORDERS | WG2_TILE_CENTER,

	// On a skip-request the block is assumed to be totally transparent and
	// will not be rendered at all (even if it actually contains graphics).

	WG2_SKIP_NORMAL        = 0x200,
	WG2_SKIP_MARKED        = 0x400,
	WG2_SKIP_SELECTED    = 0x800,
	WG2_SKIP_DISABLED    = 0x1000,
	WG2_SKIP_SPECIAL        = 0x2000,

	// Flags that can not be set by user

	WG2_HAS_BORDERS        = 0x4000,
};

//____ WgBlock ________________________________________________________________

class WgBlock
{
public:
	WgBlock() : m_pSurf(0), m_flags(0) { }

	WgBlock( wg::Surface * pSurf, const WgRect& rect, const WgBorders& sourceFrame, const WgBorders& canvasFrame, const WgBorders& padding, WgCoord contentShift, int scale, Uint32 flags )
	{
		m_pSurf            = pSurf;
		m_rect            = rect;
		m_sourceFrame    = sourceFrame;
		m_canvasFrame    = canvasFrame;
		m_flags            = flags;
		m_padding        = padding;
		m_contentShift    = contentShift;
		m_scale         = scale;

		if( m_sourceFrame.left || m_sourceFrame.right || m_sourceFrame.top || m_sourceFrame.bottom )
			m_flags |= WG2_HAS_BORDERS;

		if(m_rect.x + m_rect.w > (int)m_pSurf->pixelSize().w)
			m_rect.w = m_pSurf->pixelSize().w - m_rect.x;
		if(m_rect.y + m_rect.h > (int)m_pSurf->pixelSize().h)
			m_rect.h = m_pSurf->pixelSize().h - m_rect.y;
	}



	inline const WgRect&        Rect() const { return m_rect; }
	inline wg::Surface *        Surface() const { return m_pSurf; }
	inline const WgBorders&        SourceFrame() const { return m_sourceFrame; }
	inline const WgBorders&        CanvasFrame() const { return m_canvasFrame; }
	inline const WgRect            ContentRect( const WgRect& blockGeo ) const { return (blockGeo + m_contentShift) - m_padding; }
	inline Uint32                Flags() const { return m_flags; }
	inline int                    Width() const { return m_rect.w; }
	inline int                    Height() const { return m_rect.h; }
	inline WgSize                Size() const { return WgSize(m_rect.w, m_rect.h); }
	inline int                     OutputScale() const { return m_scale; }
	inline int                     SurfaceScale() const { return (int) m_pSurf->scale()*4096/64; }

	inline int                    MinWidth() const { return m_canvasFrame.width(); }
	inline int                    MinHeight() const { return m_canvasFrame.height(); }
	inline WgSize                MinSize() const { return m_canvasFrame.size(); }

	inline bool                    IsOpaque() const { return ((m_flags & WG2_OPAQUE) != 0); }
	inline bool                    HasOpaqueCenter() const { return ((m_flags & WG2_OPAQUE_CENTER) != 0); }
	inline bool                    HasBorders() const { return ((m_flags & WG2_HAS_BORDERS) != 0); }

	inline bool                    HasTiles() const { return ((m_flags & WG2_TILE_ALL) != 0); }
	inline bool                    HasTiledCenter() const { return ((m_flags & WG2_TILE_CENTER) != 0); }
	inline bool                    HasTiledTopBorder() const { return ((m_flags & WG2_TILE_TOP) != 0); }
	inline bool                    HasTiledBottomBorder() const { return ((m_flags & WG2_TILE_BOTTOM) != 0); }
	inline bool                    HasTiledLeftBorder() const { return ((m_flags & WG2_TILE_LEFT) != 0); }
	inline bool                    HasTiledRightBorder() const { return ((m_flags & WG2_TILE_RIGHT) != 0); }

	inline bool                    IsScaled() const { return ((m_flags & WG2_SCALE_CENTER) != 0); }
	inline bool                    IsFixedSize() const { return ((m_flags & WG2_FIXED_CENTER) != 0); }

	inline bool                    IsSkipable() const    { return ((m_flags & WG2_SKIP_NORMAL) != 0); }

	inline bool                    IsValid() const { return (m_pSurf && m_rect.w != 0 && m_rect.h != 0 )?true:false; }


	bool    operator==( const WgBlock& b) const
	{
		if( m_pSurf == b.m_pSurf && m_rect == b.m_rect /*Ś&& m_frame == b.m_frame &&
														m_flags == b.m_flags && m_padding == b.m_padding &&
														m_contentShift == b.m_contentShift*/ )
			return true;

		return false;
	}

	bool    operator!=( const WgBlock& b) const
	{
		if( m_pSurf == b.m_pSurf && m_rect == b.m_rect && m_sourceFrame == b.m_sourceFrame &&
		   m_canvasFrame == b.m_canvasFrame &&
		   m_flags == b.m_flags && m_padding == b.m_padding &&
		   m_contentShift == b.m_contentShift )
			return false;

		return true;
	}

private:

	wg::Surface *       m_pSurf;
	WgRect                m_rect;
	WgBorders             m_sourceFrame;
	WgBorders            m_canvasFrame;
	WgBorders            m_padding;
	WgCoord                m_contentShift;
	Uint32                m_flags;
	int                 m_scale = WG_SCALE_BASE;
};

//____ Class WgGfxAnim ________________________________________________________

class	WgGfxAnim : public WgAnim
{
public:
	WgGfxAnim();
	WgGfxAnim( WgSize size, WgBorders gfxBorders = WgBorders(), Uint32 blockFlags = 0 );

	void		SetSize( WgSize size );
	WgSize		Size(int scale = WG_SCALE_BASE) const;

	void		SetAlternative( int alt, int activationScale, WgSize size, WgBorders gfxBorders );

	void		SetGfxBorders( WgBorders borders );
	WgBorders	GfxBorders() const { return m_borders[0]; }

	void		SetBlockFlags( Uint32 flags );
	Uint32		BlockFlags() const { return m_blockFlags; }

	bool		InsertFrame( int pos, wg::Surface * pSurf, WgCoord ofs, int duration );
	bool		InsertFrame( WgGfxFrame * pBefore, wg::Surface * pSurf, WgCoord ofs, int duration );
	bool		AddFrame( wg::Surface * pSurf, WgCoord ofs, int duration );
	int			AddFrames( wg::Surface * pSurf, WgCoord arrayOfs, WgSize arraySize, int duration, int nFrames = 0, WgSize spacing = WgSize() );
	int			AddFrames( wg::Surface * pSurf, int duration, int nFrames = 0, WgSize spacing = WgSize() );

	bool		SetAltFrame( int pos, int alt, wg::Surface * pSurf, WgCoord ofs );

	int			SetAltFrames( int frameOffset, int scaleFactor, wg::Surface * pSurf, WgCoord arrayOfs, WgSize arraySize, int nFrames = 0, WgSize spacing = WgSize() );

	WgGfxFrame * GetFrame( int64_t ticks, WgGfxFrame * pProximity = 0 ) const;

	WgGfxFrame * GetFirstFrame(void) {return (WgGfxFrame *) WgAnim::_firstKeyFrame(); };
	WgGfxFrame * GetLastFrame(void) {return (WgGfxFrame *) WgAnim::_lastKeyFrame(); };

	WgBlock		 GetBlock( int64_t tick, int scale, WgGfxFrame * pProximity = 0 ) const;
    std::tuple<WgBlock, WgBlock, float> GetInterpolatedBlock( double tick, int scale, WgGfxFrame * pProximity = 0 ) const;

protected:

	int			m_activationScale[MAX_ANIM_ALT];		// Ńeeds to be in increasing order
	WgSize		m_size[MAX_ANIM_ALT];
	WgBorders	m_borders[MAX_ANIM_ALT];
	Uint32		m_blockFlags;
};

//____ Class WgGfxFrame _______________________________________________________

class WgGfxFrame : public WgKeyFrame
{
public:
	WgGfxFrame();


	// Derived from WgKeyFrame: Uint32	timestamp

	WgGfxFrame * GetNext(void) {return (WgGfxFrame *) WgKeyFrame::next();};
	WgGfxFrame * GetPrev(void) {return (WgGfxFrame *) WgKeyFrame::prev();};

	wg::Surface_p pSurf[MAX_ANIM_ALT];
	WgCoord		ofs[MAX_ANIM_ALT];
};

#endif //WG2_GFXANIM_DOT_H
