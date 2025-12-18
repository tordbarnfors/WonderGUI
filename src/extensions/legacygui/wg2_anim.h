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
#ifndef WG2_ANIM_DOT_H
#define WG2_ANIM_DOT_H

#ifndef	WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#include <wg_chain.h>

class		WgKeyFrame;
struct	WgAnimPlayPos;


//____ Class WgKeyFrame _______________________________________________________

class WgKeyFrame : public wg::Link
{
	friend class WgAnim;
public:
	bool	SetDuration( int ticks );
	int		Duration( void ) { return m_duration; };
	int		Timestamp( void ) { return m_timestamp; };

protected:
	int		m_timestamp;
	int		m_duration;
	LINK_METHODS( WgKeyFrame );
};


//____ Class WgAnim ______________________________________________________

class WgAnim
{
	friend class WgKeyFrame;

public:
	WgAnim();
	virtual ~WgAnim();

	bool				SetPlayMode( WgAnimMode mode );
	bool				SetTimeScaler( float scale );

	bool				DeleteKeyFrame( int pos );
	bool				DeleteKeyFrame( WgKeyFrame * pKeyFrame );
	void				Clear( void ) { m_keyframes.clear(); };

	int					Duration( void ) { return m_duration; };
	WgAnimMode			PlayMode( void ) { return m_playMode; };
	float				TimeScaler( void ) { return m_scale; };
	int					DurationScaled( void ) { return (int) (m_duration * m_scale); };
	int					TimeToOfs( int64_t ticks );			/// Convert play-time to offset in animation by scaling with timeScaler and unwinding loops.

protected:

	// Meant to be overloaded with methods by the same name that builds up their
	// WgKeyFrame-derived class.

	bool				_insertKeyFrame( int pos, WgKeyFrame * pFrame, int duration );
	bool				_insertKeyFrame( WgKeyFrame * pBefore, WgKeyFrame * pFrame, int duration );
	bool				_addKeyFrame( WgKeyFrame * pFrame, int duration );

	// Meant to be overloaded with public methods returning right type.

	const WgKeyFrame *	_firstKeyFrame( void ) const {return m_keyframes.first();};
	const WgKeyFrame *	_lastKeyFrame( void ) const {return m_keyframes.last();};

	WgKeyFrame *		_firstKeyFrame( void ) {return m_keyframes.first();};
	WgKeyFrame *		_lastKeyFrame( void ) {return m_keyframes.last();};

	//

	WgAnimPlayPos		_playPos( int64_t ticks, WgKeyFrame * pProximity = 0 ) const;		// ticks gets scaled.

	WgKeyFrame *		_keyFrame( int64_t ticks, WgKeyFrame * pProximity = 0 ) const;

protected:

	float				m_scale;			// Only used for getKeyFrame
	int					m_duration;
	WgAnimMode			m_playMode;
	wg::Chain<WgKeyFrame>	m_keyframes;
};

//____ Struct WgAnimPlayPos ___________________________________________________

struct WgAnimPlayPos
{
	WgKeyFrame *	pKeyFrame1;					// KeyFrame we are transitioning from.
	WgKeyFrame *	pKeyFrame2;					// KeyFrame we are transitioning towards.
	float			transition;					// 0 -> 1.0, current position. (1.0 only when anim has ended).
	int				animOffset;					// Offset in ticks from start of animation (unwinding loops etc).
};


#endif //WG2_ANIM_DOT_H
