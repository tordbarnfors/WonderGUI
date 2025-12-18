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
#include <wg_timer.h>
#include <wg_gfxdevice.h>
#include <wg_msg.h>
#include <wg_base.h>
#include <wg_msgrouter.h>

namespace wg
{
	using namespace Util;

	const TypeInfo Timer::TYPEINFO = { "Timer", &Widget::TYPEINFO };


//____ typeInfo() _________________________________________________________

	const TypeInfo& Timer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDuration() ________________________________________________________

	void Timer::setDuration(int millisec)
	{
		if (m_duration != millisec)
		{
			int value = int( int64_t(m_value) * millisec / m_duration );
			m_duration = millisec;
			if (value != m_value)
				_setValue(value);
		}
	}

	//____ setPlayMode() ______________________________________________________

	void Timer::setPlayMode(PlayMode playMode)
	{
		if (playMode != m_playMode)
		{
			m_playMode = playMode;

			if (playMode == PlayMode::Backward || playMode == PlayMode::BackwardLooping || playMode == PlayMode::BackwardPingPong)
				m_direction = -1;
			else
				m_direction = 1;
		}
	}

	//____ setStepSize() ______________________________________________________

	void Timer::setStepSize(int millisec)
	{
		m_stepSize = millisec;
	}


	//____ start() ____________________________________________________________

	void Timer::start()
	{
		setOn(true);
	}

	//____ stop() _____________________________________________________________

	void Timer::stop()
	{
		setOn(false);
	}

	//____ reset() ____________________________________________________________

	void Timer::reset()
	{
		int value = 0;

		if (m_playMode == PlayMode::Backward || m_playMode == PlayMode::BackwardLooping || m_playMode == PlayMode::BackwardPingPong)
			value = m_duration;

		if (value != m_value)
			_setValue(value);
	}

	//____ setOn() _______________________________________________________

	void Timer::setOn(bool bOn)
	{
		if (bOn == m_bOn)
			return;

		m_bOn = bOn;

		if (bOn)
		{
			if (m_value == 0 || m_value == m_duration)
				reset();

			_startReceiveUpdates();
		}
		else
			_stopReceiveUpdates();
	}

	//____ setValue() _________________________________________________________

	void Timer::setValue(int value)
	{
		if (value != m_value)
			_setValue(value);
	}

	//____ _update() __________________________________________________________

	void Timer::_update(int microPassed, int64_t microsecTimestamp)
	{
		int value = m_value + microPassed/1000 * m_direction;

		switch (m_playMode)
		{
			case PlayMode::Forward:
				if (value >= m_duration)
					stop();
				break;

			case PlayMode::Looping:
				value %= m_duration;
				break;

			case PlayMode::PingPong:
			case PlayMode::BackwardPingPong:
				while (value < 0 || value > m_duration)
				{
					if (value < 0)
						value = 0 - value;
					else
						value = m_duration - (value - m_duration);

					m_direction *= -1;
				}
				break;

			case PlayMode::Backward:
				if (value <= 0)
					stop();
				break;

			case PlayMode::BackwardLooping:
				if (value < 0)
					value = m_duration + (value % m_duration);
				break;
		}
		_setValue(value);
	}

	//____ _render() __________________________________________________________

	void Timer::_render(GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		if (!m_skin.isEmpty())
		{
			int quantizedValue = m_value - (m_value % m_stepSize);
			m_renderedFraction = quantizedValue / float(m_duration);

			m_skin.render(pDevice, canvas, m_scale, m_state, m_renderedFraction );
		}
	}

	//____ _setValue() __________________________________________________________

	void Timer::_setValue(int value)
	{
		limit(value, 0, m_duration);

		if (value != m_value)
		{
			m_value = value;

			int quantizedValue = value - (value % m_stepSize);
			float fraction = quantizedValue / float(m_duration);

			m_skin.valueChanged(fraction, m_renderedFraction);
		}
	}

	//____ _skinValue() _______________________________________________________

	float Timer::_skinValue(const SkinSlot* pSlot) const
	{
		return float(m_value)/m_duration;
	}


} // namespace wg
