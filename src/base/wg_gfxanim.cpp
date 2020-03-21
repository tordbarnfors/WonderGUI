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


#include <wg_gfxanim.h>
#include <wg_surface.h>

namespace wg
{

	const TypeInfo GfxAnim::TYPEINFO = { "GfxAnim", &Anim::TYPEINFO };

	//____ constructor ____________________________________________________________

	GfxAnim::GfxAnim()
	{
	}


	GfxAnim::GfxAnim( SizeI size )
	{
		m_size = size;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GfxAnim::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setSize() ______________________________________________________________

	void GfxAnim::setSize( SizeI size )
	{
		m_size = size;
	}

	//____ insertFrame() __________________________________________________________

	bool GfxAnim::insertFrame( int pos, Surface * pSurf, CoordI ofs, int duration )
	{
		if( !pSurf || pSurf->width() < (int)(ofs.x + m_size.w) || pSurf->height() < (int)(ofs.y + m_size.h) )
			return false;


		GfxFrame * pFrame = new GfxFrame();

		pFrame->pSurf = pSurf;
		pFrame->rect = RectI(ofs,m_size);

		bool bOk = Anim::_insertKeyFrame( pos, pFrame, duration );

		if( !bOk )
			delete pFrame;

		return bOk;
	}

	bool GfxAnim::insertFrame( GfxFrame * pBefore, Surface * pSurf, CoordI ofs, int duration )
	{
		if( !pSurf )
			return false;

		GfxFrame * pFrame = new GfxFrame;

		pFrame->pSurf = pSurf;
		pFrame->rect = RectI(ofs,m_size);

		bool bOk = Anim::_insertKeyFrame( pBefore, pFrame, duration );

		if( !bOk )
			delete pFrame;

		return bOk;
	}

	//____ addFrame() _____________________________________________________________

	bool GfxAnim::addFrame( Surface * pSurf, CoordI ofs, int duration )
	{
		if( !pSurf || pSurf->width() < (int)(ofs.x + m_size.w) || pSurf->height() < (int)(ofs.y + m_size.h) )
			return false;

		GfxFrame * pFrame = new GfxFrame;

		pFrame->pSurf = pSurf;
		pFrame->rect = RectI(ofs,m_size);

		bool bOk = Anim::_addKeyFrame( pFrame, duration );

		if( !bOk )
			delete pFrame;

		return bOk;
	}


	//____ addFrames() ____________________________________________________________

	int GfxAnim::addFrames( Surface * pSurf, int duration, int nFrames, SizeI spacing )
	{
		if( !pSurf )
			return 0;

		SizeI arraySize;

		arraySize.w = (pSurf->width()+spacing.w) / (m_size.w+spacing.w);
		arraySize.h = (pSurf->height()+spacing.h) / (m_size.h+spacing.h);

		return addFrames( pSurf, CoordI(0,0), arraySize, duration, nFrames, spacing );
	}


	int GfxAnim::addFrames( Surface * pSurf, CoordI arrayOfs, SizeI arraySize, int duration, int nFrames, SizeI spacing )
	{
		int nFramesX = arraySize.w;
		int nFramesY = arraySize.h;

		if( !pSurf || nFrames < 0 || nFrames > nFramesX*nFramesY || nFramesX <= 0 || nFramesY <= 0 ||
			((nFramesX*m_size.w+(nFramesX-1)*spacing.w ) > pSurf->width() - arrayOfs.x ) ||
			((nFramesY*m_size.h+(nFramesY-1)*spacing.h ) > pSurf->height() - arrayOfs.y ) )
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

				GfxFrame * pFrame = new GfxFrame;

				pFrame->pSurf = pSurf;
				pFrame->rect = RectI( arrayOfs.x + x*(m_size.w+spacing.w), arrayOfs.y + y*(m_size.h+spacing.h), m_size );

				bool bOk = Anim::_addKeyFrame( pFrame, duration );

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


	//____ getFrame() _____________________________________________________________

	GfxFrame * GfxAnim::getFrame( int64_t ticks, GfxFrame * pProximity ) const
	{
		AnimPlayPos playPos = _playPos( ticks, pProximity );

		return (GfxFrame *) playPos.pKeyFrame1;
	}

} // namespace wg
