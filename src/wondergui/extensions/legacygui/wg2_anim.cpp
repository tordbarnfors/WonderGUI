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
#include <wg2_anim.h>


//____ WgAnim() _______________________________________________________________

WgAnim::WgAnim()
{
	m_scale			= 1.f;
	m_duration		= 0;
	m_playMode	= WgAnimMode::Forward;
}

//____ ~WgAnim() ______________________________________________________________

WgAnim::~WgAnim()
{
}

//____ SetPlayMode() __________________________________________________________

bool WgAnim::SetPlayMode( WgAnimMode mode )
{
	m_playMode = mode;
	return true;
}

//____ SetTimeScaler() ________________________________________________________

bool WgAnim::SetTimeScaler( float scale )
{
	if( scale <= 0.f )
		return false;

	m_scale = scale;
	return true;
}


//____ DeleteKeyFrame() _______________________________________________________

bool WgAnim::DeleteKeyFrame( int pos )
{
	return false;
}

//____ DeleteKeyFrame() _______________________________________________________

bool WgAnim::DeleteKeyFrame( WgKeyFrame * pKeyFrame )
{
	delete pKeyFrame;
	return true;
}


//____ _insertKeyFrame() _______________________________________________________

bool WgAnim::_insertKeyFrame( int pos, WgKeyFrame * pFrame, int duration )
{
	WgKeyFrame * pPos = m_keyframes.first();

	for( int i = 0 ; i < pos ; i++ )
	{
		pPos = pPos->next();
		if( pPos == 0 )
			return false;
	}

	return _insertKeyFrame( pPos, pFrame, duration );
}


bool WgAnim::_insertKeyFrame( WgKeyFrame * pPos, WgKeyFrame * pFrame, int duration )
{
	if( pFrame->m_duration == 0 || pPos->chain() != &m_keyframes || m_duration + pFrame->m_duration < m_duration )
		return false;

	pFrame->moveBefore( pPos );
	pFrame->m_timestamp = pPos->m_timestamp;
	pFrame->m_duration = duration;

	while( pPos != 0 )
	{
		pPos->m_timestamp += duration;
		pPos = pPos->next();
	}
	m_duration += duration;
	return true;
}


//____ _addKeyFrame() __________________________________________________________

bool WgAnim::_addKeyFrame( WgKeyFrame * pFrame, int duration )
{
	if( duration == 0 || m_duration + duration < m_duration )
		return false;

	pFrame->m_duration = duration;
	pFrame->moveLast( &m_keyframes );
	pFrame->m_timestamp = m_duration;
	m_duration += pFrame->m_duration;
	return true;
}

//____ _playPos() ___________________________________________________________

WgAnimPlayPos WgAnim::_playPos( int64_t ticks, WgKeyFrame * pProximity ) const
{
	ticks = (int64_t)(ticks * ((double)m_scale));

	WgAnimPlayPos	pos;

	switch( m_playMode )
	{
		case WgAnimMode::Forward:
		{
			if( ticks >= m_duration )
			{
				pos.pKeyFrame1		= m_keyframes.last();
				pos.pKeyFrame2		= m_keyframes.last();
				pos.transition		= 1.f;													// 1.f only when it has stopped.
				pos.animOffset		= m_duration;
			}
			else
			{
				WgKeyFrame * pFrame = _keyFrame( ticks, pProximity );
				pos.pKeyFrame1			= pFrame;
				pos.transition			= (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
				pos.animOffset			= (int) ticks;
				pos.pKeyFrame2			= pFrame->next();

				if( pos.pKeyFrame2 == 0 )
					pos.pKeyFrame2		= m_keyframes.last();
			}
			break;
		}

		case WgAnimMode::Backward:
		{
			if( ticks >= m_duration )
			{
				pos.pKeyFrame1			= m_keyframes.first();
				pos.pKeyFrame2			= m_keyframes.first();
				pos.transition			= 1.f;											// 1.f only when it has stopped.
				pos.animOffset			= 0;
			}
			else
			{
				WgKeyFrame * pFrame = _keyFrame( m_duration - ticks, pProximity );
				pos.pKeyFrame2			= pFrame;
				pos.transition			= 1.f - (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
				pos.animOffset			= m_duration - (int) ticks;
				pos.pKeyFrame1			= pFrame->next();

				if( pos.pKeyFrame1 == 0 )
					pos.pKeyFrame1		= m_keyframes.last();
			}
			break;
		}

		case WgAnimMode::Looping:
		{
			ticks = ticks % m_duration;

			WgKeyFrame * pFrame		= _keyFrame( ticks, pProximity );
			pos.pKeyFrame1			= pFrame;
			pos.transition			= (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
			pos.animOffset			= (int) ticks;
			pos.pKeyFrame2			= pFrame->next();

			if( pos.pKeyFrame2 == 0 )
				pos.pKeyFrame2		= m_keyframes.first();
			break;
		}

		case WgAnimMode::BackwardLooping:
		{
			ticks = ticks % m_duration;

			WgKeyFrame * pFrame =	_keyFrame( m_duration - ticks, pProximity );
			pos.pKeyFrame2			= pFrame;
			pos.transition			= 1.f - (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
			pos.animOffset			= m_duration - (int) ticks;
			pos.pKeyFrame1			= pFrame->next();

			if( pos.pKeyFrame1 == 0 )
				pos.pKeyFrame1		= m_keyframes.first();
			break;
		}


		case WgAnimMode::PingPong:
		{
			int64_t	ts;

			if( ((ticks/m_duration) % 2) == 0 )
			{
				ts = ticks % m_duration;
				WgKeyFrame * pFrame		= _keyFrame( ts, pProximity );
				pos.pKeyFrame1			= pFrame;
				pos.transition	= (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
				pos.animOffset			= (int) ts;
				pos.pKeyFrame2			= pFrame->next();

				if( pos.pKeyFrame2 == 0 )
					pos.pKeyFrame2		= m_keyframes.last();
			}
			else
			{
				ts = m_duration - (ticks % m_duration);

				WgKeyFrame * pFrame		= _keyFrame( ts, pProximity );
				pos.pKeyFrame2			= pFrame;
				pos.transition			= 1.f - (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
				pos.animOffset			= (int) ts;
				pos.pKeyFrame1			= pFrame->next();

				if( pos.pKeyFrame1 == 0 )
					pos.pKeyFrame1		= m_keyframes.last();

			}
			break;
		}

		case WgAnimMode::BackwardPingPong:
		{
			int64_t		ts;

			if( ((ticks/m_duration) % 2) == 1 )
			{
				ts = ticks % m_duration;
				WgKeyFrame * pFrame		= _keyFrame( ts, pProximity );
				pos.pKeyFrame1			= pFrame;
				pos.transition	= (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
				pos.animOffset			= (int) ts;
				pos.pKeyFrame2			= pFrame->next();

				if( pos.pKeyFrame2 == 0 )
					pos.pKeyFrame2		= m_keyframes.last();
			}
			else
			{
				ts = m_duration - (ticks % m_duration);

				WgKeyFrame * pFrame		= _keyFrame( ts, pProximity );
				pos.pKeyFrame2			= pFrame;
				pos.transition	= 1.f - (ticks-pFrame->m_timestamp) / (float) pFrame->m_duration;
				pos.animOffset			= (int) ts;
				pos.pKeyFrame1			= pFrame->next();

				if( pos.pKeyFrame1 == 0 )
					pos.pKeyFrame1		= m_keyframes.last();
			}
			break;
		}
	}
	return pos;
}

//____ _keyFrame() __________________________________________________________

WgKeyFrame * WgAnim::_keyFrame( int64_t ticks, WgKeyFrame * pProximity ) const
{
	// If proximity isn't set, we'll start from first or last depending
	// on which we belive is closest.


	if( !pProximity )
	{
		if( ticks < m_duration - ticks )
			pProximity = m_keyframes.first();
		else
			pProximity = m_keyframes.last();
	}


	// Step forward or backward until we find the right keyframe

	if( pProximity->m_timestamp > ticks )
	{
		pProximity = pProximity->prev();
		while( pProximity->m_timestamp > ticks )
			pProximity = pProximity->prev();

	}
	else
	{
		// If no next exist, return last valid frame (should always be last)
		while (pProximity->next() && pProximity->m_timestamp + pProximity->m_duration <= ticks)
		{
			pProximity = pProximity->next();
		}
	}

	return pProximity;
}



//____ TimeToOfs() _____________________________________________________________

int WgAnim::TimeToOfs( int64_t ticks )
{
	ticks = (int64_t)(ticks * ((double)m_scale));

	switch( m_playMode )
	{
		case WgAnimMode::Forward:
			if( ticks >= m_duration )
				return m_duration;

			return (int) ticks;

		case WgAnimMode::Backward:
			if( ticks >= m_duration )
				return 0;

			return m_duration - (int)ticks;

		case WgAnimMode::Looping:
			return (int) (ticks % m_duration);

		case WgAnimMode::BackwardLooping:
			return m_duration - (int)(ticks % m_duration);


		case WgAnimMode::PingPong:
			if( ((ticks/m_duration) % 2) == 0 )
				return (int) (ticks % m_duration);
			else
				return m_duration - (int)(ticks % m_duration);

		case WgAnimMode::BackwardPingPong:
			if( ((ticks/m_duration) % 2) == 1 )
				return (int) (ticks % m_duration);
			else
				return m_duration - (int)(ticks % m_duration);


		default:
			return 0;
	}
}




//____ WgKeyFrame::SetDuration() ______________________________________________

bool WgKeyFrame::SetDuration( int ticks )
{

	// Ugly hack to get a pointer to our WgAnim object.

	wg::Chain<WgKeyFrame> * pChain = chain();
	WgAnim * pAnim = 0;
	char * ofs = (char *) &(pAnim->m_keyframes);
	pAnim = (WgAnim *)(((char *)pChain) - ofs);

	//

	int	change = ticks - m_duration;

	if( pAnim->m_duration + change < pAnim->m_duration )
		return false;

	pAnim->m_duration += change;

	m_duration = ticks;

	WgKeyFrame * pFrame = next();
	while( pFrame )
	{
		pFrame->m_duration += change;
		pFrame = pFrame->next();
	}
	return true;
}
