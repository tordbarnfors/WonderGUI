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
#include <wg2_gfxanim.h>
#include <wg_surface.h>

//____ Constructor ____________________________________________________________

WgGfxAnim::WgGfxAnim()
{
	m_blockFlags = 0;
}


WgGfxAnim::WgGfxAnim( WgSize size, WgBorders gfxBorders, Uint32 blockFlags )
{
	m_size[0] = size;
	m_borders[0] = gfxBorders;
	m_blockFlags = blockFlags;

	m_activationScale[0] = 0;
	for( int i = 1 ; i < MAX_ANIM_ALT ; i++ )
	{
		m_activationScale[i] 	= 0x7FFFFFFF;
		m_size[i] 				= size;
		m_borders[i] 			= gfxBorders;

	}
}


//____ Size() ______________________________________________________________

WgSize WgGfxAnim::Size(int scale) const
{
	WgGfxFrame * pFrame = (WgGfxFrame *) WgAnim::_firstKeyFrame();
	int factor = pFrame ? pFrame->pSurf[0]->scale()*4096/64 : WG_SCALE_BASE;

	return m_size[0]*scale/factor;
}


//____ SetSize() ______________________________________________________________

void WgGfxAnim::SetSize( WgSize size )
{
	m_size[0] = size;
}

//____ SetAlternative() ______________________________________________________________

void WgGfxAnim::SetAlternative( int alt, int activationScale, WgSize size, WgBorders gfxBorders )
{
	if( alt < 1 || alt >= MAX_ANIM_ALT )
		return;

	m_activationScale[alt] = activationScale;
	m_size[alt] = size;
	m_borders[alt] = gfxBorders;
}


//____ SetGfxBorders() ________________________________________________________

void WgGfxAnim::SetGfxBorders( WgBorders borders )
{
	m_borders[0] = borders;
}

//____ SetBlockFlags() ________________________________________________________

void WgGfxAnim::SetBlockFlags( Uint32 flags )
{
	m_blockFlags = flags;
}



//____ InsertFrame() __________________________________________________________

bool WgGfxAnim::InsertFrame( int pos, wg::Surface * pSurf, WgCoord ofs, int duration )
{
	if( pSurf->pixelSize().w < (int)(ofs.x + m_size[0].w) || pSurf->pixelSize().h < (int)(ofs.y + m_size[0].h) )
		return false;


	WgGfxFrame * pFrame = new WgGfxFrame();

	pFrame->pSurf[0] = pSurf;
	pFrame->ofs[0] = ofs;

	bool bOk = WgAnim::_insertKeyFrame( pos, pFrame, duration );

	if( !bOk )
		delete pFrame;

	return bOk;
}

bool WgGfxAnim::InsertFrame( WgGfxFrame * pBefore, wg::Surface * pSurf, WgCoord ofs, int duration )
{
	WgGfxFrame * pFrame = new WgGfxFrame;

	pFrame->pSurf[0] = pSurf;
	pFrame->ofs[0] = ofs;

	bool bOk = WgAnim::_insertKeyFrame( pBefore, pFrame, duration );

	if( !bOk )
		delete pFrame;

	return bOk;
}

//____ AddFrame() _____________________________________________________________

bool WgGfxAnim::AddFrame( wg::Surface * pSurf, WgCoord ofs, int duration )
{
	if( pSurf->pixelSize().w < (int)(ofs.x + m_size[0].w) || pSurf->pixelSize().h < (int)(ofs.y + m_size[0].h) )
		return false;

	WgGfxFrame * pFrame = new WgGfxFrame;

	pFrame->pSurf[0] = pSurf;
	pFrame->ofs[0] = ofs;

	bool bOk = WgAnim::_addKeyFrame( pFrame, duration );

	if( !bOk )
		delete pFrame;

	return bOk;
}

//____ SetAltFrame() ___________________________________________________________

bool WgGfxAnim::SetAltFrame( int pos, int alt, wg::Surface * pSurf, WgCoord ofs )
{
	if( pos < 0 || pos >= m_keyframes.size() || alt <= 0 || alt >= MAX_ANIM_ALT )
		return false;

	WgGfxFrame * p = static_cast<WgGfxFrame*> (m_keyframes.get(pos));

	p->pSurf[alt] = pSurf;
	p->ofs[alt] = ofs;
	return true;
}

//____ AddFrames() ____________________________________________________________

int WgGfxAnim::AddFrames( wg::Surface * pSurf, int duration, int nFrames, WgSize spacing )
{
	if( pSurf == 0 )
		return 0;

	WgSize arraySize;

	arraySize.w = (pSurf->pixelSize().w+spacing.w) / (m_size[0].w+spacing.w);
	arraySize.h = (pSurf->pixelSize().h+spacing.h) / (m_size[0].h+spacing.h);

	return AddFrames( pSurf, WgCoord(0,0), arraySize, duration, nFrames, spacing );
}


int WgGfxAnim::AddFrames( wg::Surface * pSurf, WgCoord arrayOfs, WgSize arraySize, int duration, int nFrames, WgSize spacing )
{
	int nFramesX = arraySize.w;
	int nFramesY = arraySize.h;

	if( pSurf == 0 || nFrames < 0 || nFrames > nFramesX*nFramesY || nFramesX <= 0 || nFramesY <= 0 ||
		((nFramesX*m_size[0].w+(nFramesX-1)*spacing.w ) > pSurf->pixelSize().w - arrayOfs.x ) ||
		((nFramesY*m_size[0].h+(nFramesY-1)*spacing.h ) > pSurf->pixelSize().h - arrayOfs.y ) )
		return 0;

	if( nFrames == 0 )
		nFrames = nFramesX*nFramesY;

	int framesAdded = 0;
	for( int y = 0 ; y < nFramesY ; y++ )
	{
		for( int x = 0 ; x < nFramesX ; x++ )
		{
			if( framesAdded == nFrames )
				break;

			WgGfxFrame * pFrame = new WgGfxFrame;

			pFrame->pSurf[0] = pSurf;
			pFrame->ofs[0].x = arrayOfs.x + x*(m_size[0].w+spacing.w);
			pFrame->ofs[0].y = arrayOfs.y + y*(m_size[0].h+spacing.h);

			bool bOk = WgAnim::_addKeyFrame( pFrame, duration );

			if( !bOk )
			{
				delete pFrame;
				return framesAdded;
			}

			framesAdded++;
		}
	}
	return framesAdded;
}


//____ GetFrame() _____________________________________________________________

WgGfxFrame * WgGfxAnim::GetFrame( int64_t ticks, WgGfxFrame * pProximity ) const
{
	WgAnimPlayPos playPos = _playPos( ticks, pProximity );

	return (WgGfxFrame *) playPos.pKeyFrame1;
}

//____ GetBlock() _____________________________________________________________

WgBlock WgGfxAnim::GetBlock( int64_t ticks, int scale, WgGfxFrame * pProximity ) const
{
	WgAnimPlayPos playPos = _playPos( ticks, pProximity );
	WgGfxFrame * pFrame = (WgGfxFrame*) playPos.pKeyFrame1;

	int alt = 0;
	for( int i = 1 ; i < MAX_ANIM_ALT ; i++ )
	{
		if( m_activationScale[i] <= scale && pFrame->pSurf[i] != 0 )
			alt = i;
	}

	return WgBlock( pFrame->pSurf[alt], WgRect( pFrame->ofs[alt], m_size[alt]), m_borders[alt], WgBorders(), WgBorders(), WgCoord(), WG_SCALE_BASE, m_blockFlags );
}

//____ GetInterpolatedBlock() _____________________________________________________________

std::tuple<WgBlock, WgBlock, float> WgGfxAnim::GetInterpolatedBlock( double ticks, int scale, WgGfxFrame * pProximity) const
{
    WgAnimPlayPos playPos = _playPos( ticks, pProximity );
    WgGfxFrame * pFrame1 = (WgGfxFrame*) playPos.pKeyFrame1;
    WgGfxFrame * pFrame2 = (WgGfxFrame*) playPos.pKeyFrame2;
    double weight = (ticks - (double)pFrame1->Timestamp())/(double)pFrame1->Duration();

    int alt = 0;
    for( int i = 1 ; i < MAX_ANIM_ALT ; i++ )
    {
        if( m_activationScale[i] <= scale && pFrame1->pSurf[i] != 0 )
            alt = i;
    }

    return { WgBlock( pFrame1->pSurf[alt], WgRect( pFrame1->ofs[alt], m_size[alt]), m_borders[alt], WgBorders(), WgBorders(), WgCoord(), WG_SCALE_BASE, m_blockFlags ),
        WgBlock( pFrame2->pSurf[alt], WgRect( pFrame2->ofs[alt], m_size[alt]), m_borders[alt], WgBorders(), WgBorders(), WgCoord(), WG_SCALE_BASE, m_blockFlags ),
        (float) weight };
}

//___________________________________________________________________

WgGfxFrame::WgGfxFrame()
{
		for( int i = 0 ; i < MAX_ANIM_ALT ; i++ )
		{
			pSurf[i] = 0;
			ofs[i].x = 0;
			ofs[i].y = 0;
		}
}
