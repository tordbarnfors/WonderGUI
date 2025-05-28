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

#include <wg2_pianokeyboard.h>
#include <wg2_gfxdevice.h>
#include <wg2_event.h>
#include <wg2_base.h>
#include <wg2_eventhandler.h>
#include <wg_util.h>

//#include <wg2_boxskin.h>

using namespace wg::Util;

static const char	c_widgetType[] = { "PianoKeyboard" };

//____ Constructor ____________________________________________________________

WgPianoKeyboard::WgPianoKeyboard()
{
}

//____ Destructor _____________________________________________________________

WgPianoKeyboard::~WgPianoKeyboard()
{
}

//____ Type() _________________________________________________________________

const char * WgPianoKeyboard::Type() const
{
	return GetClass();
}

const char * WgPianoKeyboard::GetClass(void)
{
	return c_widgetType;
}


//____ setLayout() ________________________________________________________

void WgPianoKeyboard::setLayout(int nbWhiteKeys, const std::bitset<7>& blackKeyPositions)
{
	if (nbWhiteKeys > c_maxKeys || nbWhiteKeys < 2)
	{
		//TODO: Error handling!

		return;
	}

	m_nbWhiteKeys = nbWhiteKeys;
	m_blackKeyPositions = blackKeyPositions;

	// Count and enumerate the keys

	KeyInfo * pInfo = m_keyInfo;
	int blackKeyIdx = 6;

	for (int i = 0; i < nbWhiteKeys-1; i++)
	{
		pInfo->keyPos = i;
		pInfo->isBlack = false;
		pInfo->isSetToPressed = false;
		pInfo++;

		if (blackKeyPositions.test(blackKeyIdx))
		{
			pInfo->keyPos = i;
			pInfo->isBlack = true;
			pInfo->isSetToPressed = false;
			pInfo++;
		}

		if (blackKeyIdx == 0)
			blackKeyIdx = 6;
		else
			blackKeyIdx--;
	}

	pInfo->keyPos = nbWhiteKeys - 1;
	pInfo->isBlack = false;
	pInfo->isSetToPressed = false;
	pInfo++;

	m_nbKeys = int(pInfo - m_keyInfo);
}

//____ setGeoTweak() ______________________________________________________

void WgPianoKeyboard::setGeoTweak( int keyIdx, const WgRect& geoTweak )
{
    if( keyIdx >= m_nbKeys )
    {
        //TODO: Error handling!

        return;
    }

    m_keyInfo[keyIdx].geoTweak = geoTweak;
}


//____ setSurfaces() ______________________________________________________

void WgPianoKeyboard::setSurfaces(const wg::Surface_p& pOddWhiteKeys, const wg::Surface_p& pEvenWhiteKeys, const wg::Surface_p& pBlackKeys, std::initializer_list<wg::State> states)
{
	if (!pOddWhiteKeys || !pEvenWhiteKeys || (!pBlackKeys && m_blackKeyPositions.count() > 0))
	{
		//TODO: Error handling!

		return;
	}

	if (pOddWhiteKeys->pixelSize() != pEvenWhiteKeys->pixelSize() || (pBlackKeys && pBlackKeys->pixelSize().w != pOddWhiteKeys->pixelSize().w) )
	{
		//TODO: Error handling!

		return;
	}

	m_pOddWhiteKeys		= pOddWhiteKeys;
	m_pEvenWhiteKeys	= pEvenWhiteKeys;
	m_pBlackKeys		= pBlackKeys;

	auto ptsSize = pOddWhiteKeys->pointSize();
	wg::SizeI pointSize = { (int) ptsSize.w, (int) ptsSize.h };
	pointSize.h /= states.size();
	m_preferredKeyboardSize	= pointSize;

	wg::SizeI pixelSize = pOddWhiteKeys->pixelSize();
	pixelSize.h /= states.size();
	m_keyboardSourceSize = pixelSize;

	m_blackKeySourceHeight	= m_pBlackKeys ? m_pBlackKeys->pixelSize().h  / states.size() : 0;

	// Fill in state offsets

	for (auto& ofs : m_stateOfsY)
		ofs = -1;

	wg::Bitmask<uint32_t>	stateMask;
	int ofs = 0;
	for (auto state : states)
	{
		int i = state;
		stateMask.setBit(i);
		m_stateOfsY[i] = ofs++;
	}

	for (int i = 0; i < wg::State::NbStates; i++)
	{
		if (!stateMask.bit(i))
		{
			int bestAlternative = bestStateIndexMatch(i, stateMask);
			m_stateOfsY[i] = m_stateOfsY[bestAlternative];
		}
	}
}


//____ pressKey() ________________________________________________________

void WgPianoKeyboard::pressKey(int keyIdx)
{
	if (keyIdx < 0 || keyIdx >= m_nbKeys)
	{
		//TODO: Error handling!
		return;
	}

	_setAPIPressed(keyIdx, true);
}

//____ releaseKey() ______________________________________________________

void WgPianoKeyboard::releaseKey(int keyIdx)
{
	if (keyIdx < 0 || keyIdx >= m_nbKeys)
	{
		//TODO: Error handling!
		return;
	}

	_setAPIPressed(keyIdx, false);
}

//____ isKeyPressed() ____________________________________________________

bool WgPianoKeyboard::isKeyPressed(int keyIdx) const
{
	if (keyIdx < 0 || keyIdx >= m_nbKeys)
	{
		//TODO: Error handling!
		return false;
	}

	return m_keyInfo[keyIdx].state.isPressed();
}

//____ setPressedKeys() __________________________________________________

void WgPianoKeyboard::setPressedKeys(std::initializer_list<int> pressedKeys)
{
	// Error handling

	for (auto idx : pressedKeys)
		if (idx < 0 || idx >= m_nbKeys)
		{
			//TODO: Error handling!
			return;
		}

	// First, release all that are not pressed anymore

	for (int i = 0; i < m_nbKeys; i++)
	{
		if (m_keyInfo[i].isSetToPressed)
		{
			bool bRemainPressed = false;

			for (auto idx : pressedKeys)
				if (idx == i)
				{
					bRemainPressed = true;
					break;
				}

			if (!bRemainPressed)
				_setAPIPressed(i,false);
		}
	}

	// Secondly, select all newly selected

	for (auto idx : pressedKeys)
		_setAPIPressed(idx, true);
}

//____ releaseAllKeys() __________________________________________________

void WgPianoKeyboard::releaseAllKeys()
{
	for (int i = 0; i < m_nbKeys; i++)
		_setAPIPressed(i, false);
}

//____ setFlipKeyOnPress() ________________________________________________

void WgPianoKeyboard::setFlipKeyOnPress(bool bFlip)
{
	m_bFlipOnPress = bFlip;
}

//____ selectKey() ________________________________________________________

void WgPianoKeyboard::selectKey(int keyIdx)
{
	if (keyIdx < 0 || keyIdx >= m_nbKeys)
	{
		//TODO: Error handling!
		return;
	}

	if (!m_keyInfo[keyIdx].state.isSelected())
	{
		m_keyInfo[keyIdx].state.setSelected(true);
		_requestRenderKey(keyIdx);
	}
}

//____ unselectKey() ______________________________________________________

void WgPianoKeyboard::unselectKey(int keyIdx)
{
	if (keyIdx < 0 || keyIdx >= m_nbKeys)
	{
		//TODO: Error handling!
		return;
	}

	if (m_keyInfo[keyIdx].state.isSelected())
	{
		m_keyInfo[keyIdx].state.setSelected(false);
		_requestRenderKey(keyIdx);
	}
}

//____ isKeySelected() ____________________________________________________

bool WgPianoKeyboard::isKeySelected(int keyIdx) const
{
	if (keyIdx < 0 || keyIdx >= m_nbKeys)
	{
		//TODO: Error handling!
		return false;
	}

	return m_keyInfo[keyIdx].state.isSelected();
}

//____ setSelectedKeys() __________________________________________________

void WgPianoKeyboard::setSelectedKeys(std::initializer_list<int> selectedKeys)
{
	// Error handling

	for (auto idx : selectedKeys)
		if (idx < 0 || idx >= m_nbKeys)
		{
			//TODO: Error handling!
			return;
		}

	// First, unselect all that are not selected anymore

	for (int i = 0; i < m_nbKeys; i++)
	{
		if( m_keyInfo[i].state.isSelected() )
		{
			bool bRemainSelected = false;

			for( auto idx : selectedKeys )
				if (idx == i)
				{
					bRemainSelected = true;
					break;
				}

			if (!bRemainSelected)
			{
				m_keyInfo[i].state.setSelected(false);
				_requestRenderKey(i);
			}
		}
	}

	// Secondly, select all newly selected

	for (auto idx : selectedKeys)
		if( !m_keyInfo[idx].state.isSelected() )
		{
			m_keyInfo[idx].state.setSelected(false);
			_requestRenderKey(idx);
		}
}

//____ unselectAllKeys() __________________________________________________

void WgPianoKeyboard::unselectAllKeys()
{
	for (int i = 0; i < m_nbKeys; i++)
	{
		if (m_keyInfo[i].state.isSelected())
		{
			m_keyInfo[i].state.setSelected(false);
			_requestRenderKey(i);
		}
	}
}

//____ _PreferredPixelSize() __________________________________________________________

wg::SizeI WgPianoKeyboard::PreferredPixelSize() const
{
	return m_preferredKeyboardSize*m_scale/WG_SCALE_BASE + (m_pSkin ? _skinContentPadding(m_pSkin, m_scale) : wg::SizeI());
}

//____ MinPixelSize() __________________________________________________________

wg::SizeI WgPianoKeyboard::MinPixelSize() const
{
	return m_preferredKeyboardSize * m_scale / WG_SCALE_BASE + (m_pSkin ? _skinContentPadding(m_pSkin, m_scale) : wg::SizeI());
}

//____ MaxPixelSize() __________________________________________________________

wg::SizeI WgPianoKeyboard::MaxPixelSize() const
{
	return m_preferredKeyboardSize * m_scale / WG_SCALE_BASE + (m_pSkin ? _skinContentPadding(m_pSkin, m_scale) : wg::SizeI());
}


//____ _onCloneContent() _______________________________________________________

void WgPianoKeyboard::_onCloneContent(const WgWidget * _pOrg)
{
//	WgWidget::_onCloneContent(_pOrg);

	WgPianoKeyboard * pOrg = (WgPianoKeyboard*)_pOrg;

}

//____ _render() __________________________________________________________

void WgPianoKeyboard::_onRender(wg::GfxDevice * pDevice, const wg::RectI& _canvas, const wg::RectI& _window)
{
	//TODO: This could use _keyRect() to simplify and secure consistency.

	WgWidget::_onRender(pDevice, _canvas, _window);

	wg::RectI canvas = m_pSkin ? _skinContentRect(m_pSkin, _canvas, m_state, m_scale) : _canvas;

    float whiteKeySpacing = canvas.w / (float) m_nbWhiteKeys;
	int blackKeyHeight = canvas.h * m_blackKeySourceHeight / m_keyboardSourceSize.h;

	float xScaleFactor = m_keyboardSourceSize.w / float(canvas.w);
	float yScaleFactor = m_keyboardSourceSize.h / float(canvas.h);

	// First we render all the black keys, we render from left to right since there won't be any perspective induced overlap

	if (m_pBlackKeys)
	{
		KeyInfo * pKey = m_keyInfo;
        KeyInfo * pEnd = &m_keyInfo[m_nbKeys];

		pDevice->setBlitSource(m_pBlackKeys);

		while( pKey < pEnd )
		{
			if (pKey->isBlack)
			{
                int i = pKey->keyPos;
				wg::RectI dst = wg::RectI(int(i*whiteKeySpacing + whiteKeySpacing / 2), 0, int(whiteKeySpacing), blackKeyHeight);

                // Apply tweaked geo. We only tweak X here to not distured srcOfsY. Should be fine that way.

                float scaleX = canvas.w / (float) m_preferredKeyboardSize.w;
                dst.x += int( pKey->geoTweak.x * scaleX);
                dst.w += int( pKey->geoTweak.w * scaleX);

				float srcOfsY = float(m_stateOfsY[pKey->state] * m_blackKeySourceHeight);

				pDevice->precisionBlit((dst + canvas.pos()) * 64, wg::RectF(dst.x*xScaleFactor, srcOfsY, dst.w*xScaleFactor, dst.h*yScaleFactor)*64);
			}

            pKey++;
		}
	}

    // Now we render the white keys, alternating form left to right in order to render the outmost keys first and center last
    // This in order to handle any perspective induced overlap

	KeyInfo * pFirst = &m_keyInfo[0];
	KeyInfo * pLast = &m_keyInfo[m_nbKeys-1];

	bool bForward = true;
	while (pFirst <= pLast)
//	while (pFirst == &m_keyInfo[0])
	{
		KeyInfo * pKey = bForward ? pFirst : pLast;

		int i = pKey->keyPos;
        if (i % 2 == 0)
            pDevice->setBlitSource(m_pOddWhiteKeys);
        else
            pDevice->setBlitSource(m_pEvenWhiteKeys);

		wg::RectI dst = wg::RectI(int(i*whiteKeySpacing - whiteKeySpacing / 2), 0, int(whiteKeySpacing * 2), canvas.h);

        // Apply tweaked geo. We only tweak X here to not distured srcOfsY. Should be fine that way.

        float scaleX = canvas.w / (float) m_preferredKeyboardSize.w;
        dst.x += int( pKey->geoTweak.x * scaleX);
        dst.w += int( pKey->geoTweak.w * scaleX);

		if (dst.x < 0)
		{
			dst.w += dst.x;
			dst.x = 0;
		}

		if (dst.x + dst.w > canvas.w)
			dst.w = canvas.w - dst.x;

		float srcOfsY = float(m_stateOfsY[pKey->state] * m_keyboardSourceSize.h);

		pDevice->precisionBlit((dst + canvas.pos()) * 64, wg::RectF(dst.x*xScaleFactor, srcOfsY, dst.w*xScaleFactor, dst.h*yScaleFactor)*64);

		if (bForward)
		{
			pFirst++;
			if (pFirst->isBlack)
				pFirst++;
		}
		else
		{
			pLast--;
			if (pLast->isBlack)
				pLast--;
		}

		bForward = !bForward;
	}
}

//____ _onEvent() _________________________________________________________

void WgPianoKeyboard::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	switch (pEvent->Type())
	{
		case WG_EVENT_MOUSE_ENTER:
		case WG_EVENT_MOUSE_MOVE:
		{
			wg::CoordI pos = pEvent->PointerPixelPos();
			int keyIdx = _markTestKey(pos);
			_setHoveredKey(keyIdx, pEvent->Timestamp());
			break;
		}

		case WG_EVENT_MOUSE_LEAVE:
		{
			_setHoveredKey(-1, pEvent->Timestamp());
			break;
		}

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			if (m_hoveredKey >= 0)
			{
				if (m_bFlipOnPress)
				{
					_setAPIPressed(m_hoveredKey, !m_keyInfo[m_hoveredKey].isSetToPressed);

					if(m_keyInfo[m_hoveredKey].isSetToPressed)
						_eventHandler()->QueueEvent(new WgEvent::PianoKeyPress(this, m_hoveredKey, pEvent->Timestamp()));
					else
						_eventHandler()->QueueEvent(new WgEvent::PianoKeyRelease(this, m_hoveredKey, pEvent->Timestamp()));
				}
				else
				{
					_setHoverPress(true, pEvent->Timestamp());
				}
			}
			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			if (m_hoveredKey >= 0)
			{
				if (!m_bFlipOnPress)
					_setHoverPress(false, pEvent->Timestamp());
			}
			else
				m_bIsHoveredPressed = false;
			break;
		}
        default:
            break;
	}
}

//____ _setHoveredKey() ___________________________________________________

void WgPianoKeyboard::_setHoveredKey(int keyIdx, uint64_t timestamp)
{
	if (keyIdx == m_hoveredKey)
		return;

	if (m_hoveredKey >= 0 && !m_keyInfo[m_hoveredKey].isSetToPressed)
	{
		wg::State& state = m_keyInfo[m_hoveredKey].state;
		wg::State oldState = state;
		state.setHovered(false);

		if (m_bIsHoveredPressed)
		_eventHandler()->QueueEvent(new WgEvent::PianoKeyRelease(this, m_hoveredKey, timestamp));

		if (m_stateOfsY[oldState] != m_stateOfsY[state])
			_requestRenderKey(m_hoveredKey);
	}

	m_hoveredKey = keyIdx;

	if (m_hoveredKey >= 0)
	{
		wg::State& state = m_keyInfo[m_hoveredKey].state;
		wg::State oldState = state;
		state.setHovered(true);

		if (m_bIsHoveredPressed)
		{
			state.setPressed(true);
			_eventHandler()->QueueEvent(new WgEvent::PianoKeyPress(this, m_hoveredKey, timestamp));
		}

		if (m_stateOfsY[oldState] != m_stateOfsY[state])
			_requestRenderKey(m_hoveredKey);
	}
}

//____ _setHoverPress() _______________________________________________

void WgPianoKeyboard::_setHoverPress(bool bPressed, uint64_t timestamp)
{
	if (m_hoveredKey == -1)
		return;

	m_bIsHoveredPressed = bPressed;

	auto& key = m_keyInfo[m_hoveredKey];

	if (key.state.isPressed() == bPressed)
		return;

	if( bPressed )
	{
		key.state.setPressed(true);
		_requestRenderKey(m_hoveredKey);

		_eventHandler()->QueueEvent(new WgEvent::PianoKeyPress(this, m_hoveredKey, timestamp));
	}
	else if( !key.isSetToPressed )
	{
		key.state.setPressed(false);
		_requestRenderKey(m_hoveredKey);

		_eventHandler()->QueueEvent(new WgEvent::PianoKeyRelease(this, m_hoveredKey, timestamp));
	}
}

//____ _setAPIPressed() ___________________________________________________

void WgPianoKeyboard::_setAPIPressed(int keyIdx, bool bPressed)
{
	auto& key = m_keyInfo[keyIdx];

	if( key.isSetToPressed != bPressed )
	{
		key.isSetToPressed = bPressed;

		if (bPressed  )
		{
			if( !key.state.isPressed() )
			{
				key.state.setPressed(true);
				_requestRenderKey(keyIdx);
			}
		}
		else
		{
			if (!m_bIsHoveredPressed || m_hoveredKey != keyIdx)
			{
				key.state.setPressed(false);
				_requestRenderKey(keyIdx);
			}
		}
	}
}

//____ _alphaTest() _______________________________________________________

bool WgPianoKeyboard::_onAlphaTest(const wg::CoordI& ofs)
{
	if (WgWidget::_onAlphaTest(ofs) || _markTestKey(ofs) >= 0)
		return true;

	return false;
}

//____ _requestRenderKey() ________________________________________________

void WgPianoKeyboard::_requestRenderKey(int keyIdx)
{
	_requestRender(_keyRect(keyIdx));
}

//____ _markTestKey() _____________________________________________________

int WgPianoKeyboard::_markTestKey(const wg::CoordI& pos)
{
	// Convert coordinate from canvas to source

	wg::RectI canvas = m_pSkin ? _skinContentRect(m_pSkin, PixelSize(), m_state, m_scale) : wg::RectI(0, 0, PixelSize());

	if (!canvas.contains(pos))
		return -1;

	float xScaleFactor = m_keyboardSourceSize.w / float(canvas.w);
	float yScaleFactor = m_keyboardSourceSize.h / float(canvas.h);

	wg::CoordI srcPos = { int((pos.x - canvas.pos().x) * xScaleFactor), int((pos.y - canvas.pos().y) * yScaleFactor) };

	float whiteKeySpacing = m_keyboardSourceSize.w / (float) m_nbWhiteKeys;

	int keyPos;
	bool isBlack = false;

	// alpha test on keys

	if (m_pOddWhiteKeys->alpha(srcPos*64))
		keyPos = int((srcPos.x + whiteKeySpacing / 2) / (whiteKeySpacing * 2)) * 2;
	else if (m_pEvenWhiteKeys->alpha(srcPos*64))
		keyPos = int((srcPos.x - whiteKeySpacing / 2) / (whiteKeySpacing * 2)) * 2 + 1;
	else if (m_pBlackKeys && (srcPos.y < m_pBlackKeys->pixelSize().h) && m_pBlackKeys->alpha(srcPos*64))
	{
		keyPos = int((srcPos.x - whiteKeySpacing / 2) / whiteKeySpacing);
		isBlack = true;
	}
	else
		return -1;

	// Find index for the key we have found

	for (auto& key : m_keyInfo)
	{
		if (key.keyPos == keyPos && key.isBlack == isBlack )
			return int(&key - m_keyInfo);
	}

	return -1;
}

//____ _keyRect() _________________________________________________________

wg::RectI WgPianoKeyboard::_keyRect(int keyIdx)
{
	wg::RectI canvas = m_pSkin ? _skinContentRect(m_pSkin, PixelSize(), m_state, m_scale) : wg::RectI(0, 0, PixelSize());

	float whiteKeySpacing = canvas.w / (float) m_nbWhiteKeys;
	int blackKeyHeight = canvas.h * m_blackKeySourceHeight / m_keyboardSourceSize.h;

	wg::RectI dst;

	if( m_keyInfo[keyIdx].isBlack )
		dst = { int( m_keyInfo[keyIdx].keyPos*whiteKeySpacing + whiteKeySpacing / 2) , 0, int(whiteKeySpacing), blackKeyHeight };
	else
		dst = { int( m_keyInfo[keyIdx].keyPos*whiteKeySpacing - whiteKeySpacing / 2), 0, int(whiteKeySpacing * 2), canvas.h };

    // Apply tweaked geo

    float scaleX = canvas.w / (float) m_preferredKeyboardSize.w;
    float scaleY = canvas.h / (float) m_preferredKeyboardSize.h;
    dst.x += int( m_keyInfo[keyIdx].geoTweak.x * scaleX);
    dst.y += int( m_keyInfo[keyIdx].geoTweak.y * scaleY);
    dst.w += int( m_keyInfo[keyIdx].geoTweak.w * scaleX);
    dst.h += int( m_keyInfo[keyIdx].geoTweak.h * scaleY);

    //

	if (dst.x < 0)
	{
		dst.w += dst.x;
		dst.x = 0;
	}

	if (dst.x + dst.w > canvas.w )
		dst.w = canvas.w - dst.x;

	return dst + canvas.pos();
}
