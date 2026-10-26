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
#ifndef	WG2_CURSOR_DOT_H
#define WG2_CURSOR_DOT_H

#ifndef	WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#ifndef WG2_GFXANIM_DOT_H
#	include <wg2_gfxanim.h>
#endif

//class WgGfxAnim;
class WgGlyphset;


//____ WgCursor _______________________________________________________________

class WgCursor
{
	friend class WgGlyphset;

public:
	WgCursor();
	WgCursor( WgCursor& in );

	enum Mode
	{
		EOL = 0,
		INS,
		OVR
	};

	enum BlitMode
	{
		NORMAL,
		TINTED,
		INVERT_BG
	};


	bool				SetMode( Mode m, WgGfxAnim * pAnim, WgCoord bearing = WgCoord(), int advance = 0, float size_ratio = 1.f  );

	void				SetBearing( Mode m, WgCoord bearing );
	void				SetAdvance( Mode m, int advance );
	void				SetAnim( Mode m, WgGfxAnim * pAnim );

	void				SetSizeRatio( Mode m, float ratio );

	void				SetBlitMode( BlitMode mode );
	BlitMode			GetBlitMode() const { return m_blitMode; }

	int					BearingX( Mode m ) const { return m_bearing[m].x; };
	int					BearingY( Mode m ) const { return m_bearing[m].y; };
	WgCoord				Bearing( Mode m ) const { return m_bearing[m]; }
	int					Advance( Mode m ) const { return m_advance[m]; };
	int					Width( Mode m ) const { return m_pAnim[m]->Size(WG_SCALE_BASE).w; }
	WgGfxAnim * 		Anim( Mode m ) const { return m_pAnim[m]; };
	float				SizeRatio(Mode m ) const { return m_sizeRatio[m]; }



private:
	enum { N_MODES = 3 };

	WgGfxAnim *			m_pAnim[N_MODES];
	WgCoord				m_bearing[N_MODES];
	int					m_advance[N_MODES];

	float				m_sizeRatio[N_MODES];			// ratio <= 1.f. Cursors height relative fonts lineheight.

	BlitMode			m_blitMode;
};





#endif // WG2_CURSOR_DOT_H
