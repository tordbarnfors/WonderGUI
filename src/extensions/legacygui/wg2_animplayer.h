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
#ifndef	WG2_ANIMPLAYER_DOT_H
#define	WG2_ANIMPLAYER_DOT_H


#ifndef	WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef	WG2_GFXANIM_DOT_H
#	include <wg2_gfxanim.h>
#endif

#include <wg2_color.h>

class WgAnimPlayer:public WgWidget
{
public:
	WgAnimPlayer();
	~WgAnimPlayer();
	virtual const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgAnimPlayer(); };


	//____ Methods __________________________________________

	bool			SetAnimation( WgGfxAnim * pAnim );
	WgGfxAnim *		Animation() const { return m_pAnim; }

	void			SetSource( wg::Skin * pSkin ) { SetSkin(pSkin); }

	void            SetSkin(wg::Skin * pSkin);

	int				PlayPos();										/// Returns play position in ticks.
	bool			SetPlayPos( int ticks );						/// Position in ticks for next update.
	bool			SetPlayPosFractional( float fraction );			/// Position in fractions of duration.

	bool			Rewind( int ticks );
	bool			FastForward( int ticks );

	int				Duration();										/// Returns duration of animation (one-shot-through, no looping).
	int				DurationScaled();								/// Returns duration of animation, scaled by speed.

	float			Speed();
	bool			SetSpeed( float speed );

	bool			Play();
	bool			Stop();
	bool			IsPlaying() { return m_bPlaying; };

	bool			PlayTo(int pos );					/// Play to position measured in ticks
	bool			PlayToFractional(float fraction);	/// Play to position measured in ticks

    void            SetInterpolation(bool bInt) { m_bInterpolation = bInt; }
    bool            GetInterpolation() { return m_bInterpolation; }

	WgSize			PreferredPixelSize() const;

	void SetTint(WgColor kColor);
	const WgColor& GetTint() { return m_kTintColor; }
	void RemoveTint();

protected:
	void			_onCloneContent( const WgWidget * _pOrg );
	void			_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	void			_onRefresh();
	void			_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	bool			_onAlphaTest( const WgCoord& ofs );
    bool            _onAlphaTestWithGeo( const WgCoord& ofs, const WgRect& geo );
	void			_onEnable();
	void			_onDisable();

	void			_playPosUpdated();

private:

	WgGfxAnim *		m_pAnim;
	WgBlock			m_animFrame;			// Frame currently used by animation.
    WgBlock         m_animFrame2;           // Frame used by interpolation
    double          m_fInterpolationWeight = 0.0f; 

	bool			m_bPlaying;
	double			m_playPos;
	int				m_destinationPos;		// -1 = none.
	float			m_speed;
	float           m_playStateFraction = 0.0f;

	WgColor         m_kTintColor;

    bool            m_bInterpolation = false;
};


#endif //	WG2_ANIMPLAYER_DOT_H
