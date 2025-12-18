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
#include <wg_base.h>
#include <wg_animplayer.h>
#include <wg_util.h>
#include <wg_msgrouter.h>
#include <wg_gfxdevice.h>

#include <math.h>


namespace wg
{
	using namespace Util;

	const TypeInfo AnimPlayer::TYPEINFO = { "AnimPlayer", &Widget::TYPEINFO };


	//____ constructor _________________________________________________________________

	AnimPlayer::AnimPlayer() : frames(this)
	{
	}

	//____ ~AnimPlayer() _______________________________________________________

	AnimPlayer::~AnimPlayer()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& AnimPlayer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setPlayMode() ____________________________________________________________

	void AnimPlayer::setPlayMode(PlayMode mode)
	{
		bool bWasPingPong = (m_playMode == PlayMode::PingPong || m_playMode == PlayMode::BackwardPingPong);
		bool bIsPingPong = (mode == PlayMode::PingPong || mode == PlayMode::BackwardPingPong);

		if (bWasPingPong && !bIsPingPong)
		{
			m_cycleDuration = frames.duration();
			if (m_playPos >= m_cycleDuration)
				m_playPos = m_cycleDuration * 2 - m_playPos - 1;
		}
		else if (!bWasPingPong && bIsPingPong)
			m_cycleDuration = frames.duration() * 2;

		m_playMode = mode;
		_requestRender();
	}

	//____ setPlayPosFraction() _________________________________________________

	bool AnimPlayer::setPlayPosFraction( float _fraction )
	{
		m_playPos		= int(_fraction * m_cycleDuration);
		return _playPosUpdated();
	}

	//____ setPlayPos() ___________________________________________________________

	bool AnimPlayer::setPlayPos( int _ticks )
	{
		m_playPos		= _ticks;
		return _playPosUpdated();
	}

	//____ rewind() _______________________________________________________________

	bool AnimPlayer::rewind( int _ticks )
	{
		if( m_playPos - _ticks < 0 )
			m_playPos = 0;
		else
			m_playPos -= _ticks;

		return _playPosUpdated();
	}

	//____ fastForward() __________________________________________________________

	bool AnimPlayer::fastForward( int _ticks )
	{
	 	m_playPos += _ticks;
		return _playPosUpdated();
	}

	//____ setSpeed() _____________________________________________________________

	void AnimPlayer::setSpeed( float _speed )
	{
		m_speed = _speed;
	}

	//____ play() _________________________________________________________________

	bool AnimPlayer::play()
	{
		if( m_bPlaying )
			return true;

		if( m_cycleDuration == 0 )
			return false;

		m_bPlaying = true;
		_startReceiveUpdates();
		return true;
	}

	//____ stop() _________________________________________________________________

	void AnimPlayer::stop()
	{
		if( !m_bPlaying )
			return;

		m_bPlaying = false;
		_stopReceiveUpdates();
	}

	//____ _defaultSize() ___________________________________________________________

	SizeSPX AnimPlayer::_defaultSize(int scale) const
	{
		return m_skin.sizeForContent(align(ptsToSpx(frames.frameSize(),scale)),scale);
	}

	//____ _update() ______________________________________________________________

	void AnimPlayer::_update(int microPassed, int64_t microsecTimestamp)
	{
		if (m_cycleDuration == 0 || m_state.isDisabled())
			return;

		m_playPos += int(microPassed * m_speed / 1000);
		_playPosUpdated();

	}

	//____ _render() ________________________________________________________

	void AnimPlayer::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render( pDevice, _canvas, _window );

		if (m_cycleDuration > 0 && !m_state.isDisabled())
		{
			RectSPX canv = _contentRect();
			pDevice->setBlitSource(frames._surface());
			auto * pFrame = frames.find(_playPosToTimestamp(m_playPos));
			pDevice->stretchFlipBlit(canv, align(ptsToSpx(Rect(pFrame->source(), frames.frameSize()),m_scale)), pFrame->flip());
		}
	}

	//____ _alphaTest() ______________________________________________________

	bool AnimPlayer::_alphaTest( const CoordSPX& ofs )
	{
		//TODO: Take flip into account!

		if (Widget::_alphaTest(ofs))
			return true;

		if (m_cycleDuration > 0 && !m_state.isDisabled())
		{
			Rect source = Rect(frames.find(_playPosToTimestamp(m_playPos))->source(), frames.frameSize());
			return Util::markTestStretchRect(ofs, frames._surface(), align(ptsToSpx(source, m_scale)), RectSPX(m_size), m_animMarkAlpha);
		}

		return false;
	}

	//____ _setState() ______________________________________________________

	void AnimPlayer::_setState( State state )
	{
		if( state.isDisabled() != m_state.isDisabled() && m_bPlaying )
		{
			if (state.isDisabled())
				_stopReceiveUpdates();
			else
				_startReceiveUpdates();

			_requestRender();
		}

		Widget::_setState(state);
	}

	//_____ _playPosUpdated() ______________________________________________________

	bool AnimPlayer::_playPosUpdated()
	{
		if (m_cycleDuration == 0)
		{
			m_playPos = 0;
			return false;
		}

		if (m_playPos >= m_cycleDuration)
		{
			if (m_playMode == PlayMode::Forward || m_playMode == PlayMode::Backward)
			{
				m_playPos = 0;
				stop();
				return false;
			}
			else
				m_playPos %= m_cycleDuration;
		}

		auto pNewFrame = frames.find(_playPosToTimestamp(m_playPos));
		if (pNewFrame->timestamp() != m_frameTimestamp)
		{
			m_frameTimestamp = pNewFrame->timestamp();
			_requestRender();
//			Base::msgRouter()->post(ValueUpdateMsg::create(this, (int)m_playPos, (float)(m_playPos / (m_cycleDuration)), true));
		}
		return true;
	}

	//____ _playPosToTimestamp() __________________________________________________________

	int AnimPlayer::_playPosToTimestamp(int playPos) const
	{
		switch (m_playMode)
		{
		case PlayMode::Forward:
		case PlayMode::Looping:
			break;
				
		case PlayMode::PingPong:
			if (playPos >= frames.duration())
				playPos = frames.duration() * 2 - (playPos + 1);
			break;

		case PlayMode::BackwardPingPong:
			if (playPos >= frames.duration())
				playPos = frames.duration() * 2 - (playPos + 1);
			// fall through on purpose.
		case PlayMode::Backward:
		case PlayMode::BackwardLooping:
			playPos = frames.duration() - (playPos + 1);
			break;
		}

		return playPos;
	}

	//____ _didSetAnimFrameSize() _____________________________________________

	void AnimPlayer::_didSetAnimFrameSize(AnimFrames* pComponent)
	{
		_requestResize();
	}

	//____ _didSetAnimSurface() _______________________________________________

	void AnimPlayer::_didSetAnimSurface(AnimFrames* pComponent)
	{
		_requestRender();
	}

	//____ _didAddEntries() ___________________________________________________

	void AnimPlayer::_didAddEntries(AnimFrame* pEntry, int nb)
	{
		int total = 0;

		for (int i = 0; i < nb; i++)
			total += pEntry[i].duration();

		//TODO: Compensate playPos for change

		if (m_playMode == PlayMode::PingPong || m_playMode == PlayMode::BackwardPingPong)
			total *= 2;
		m_cycleDuration += total;
	}

	//____ _didMoveEntries() ___________________________________________________

	void AnimPlayer::_didMoveEntries(AnimFrame* pFrom, AnimFrame* pTo, int nb)
	{
		//TODO: Compensate playPos for change
	}

	//____ _willEraseEntries() _________________________________________________

	void AnimPlayer::_willEraseEntries(AnimFrame* pEntry, int nb)
	{
		int total = 0;

		for (int i = 0; i < nb; i++)
			total += pEntry[i].duration();

		//TODO: Compensate playPos for change

		if (m_playMode == PlayMode::PingPong || m_playMode == PlayMode::BackwardPingPong)
			total *= 2;
		m_cycleDuration -= total;

	}


} // namespace wg
