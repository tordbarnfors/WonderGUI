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

#include <wg_pianokeyboard.h>
#include <wg_gfxdevice.h>
#include <wg_msg.h>
#include <wg_base.h>
#include <wg_msgrouter.h>

//#include <wg_boxstateskin.h>

namespace wg
{
	using namespace Util;

	const TypeInfo PianoKeyboard::TYPEINFO = { "PianoKeyboard", &Widget::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PianoKeyboard::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setLayout() ________________________________________________________

	void PianoKeyboard::setLayout(int nbWhiteKeys, const std::bitset<7>& blackKeyPositions)
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

	//____ setSurfaces() ______________________________________________________

	void PianoKeyboard::setSurfaces(const Surface_p& pOddWhiteKeys, const Surface_p& pEvenWhiteKeys, 
									const Surface_p& pBlackKeys, const std::initializer_list<State>& states)
	{
		if (states.size() == 0)
			return;

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

		Size pointSize = pOddWhiteKeys->pointSize();
		pointSize.h /= states.size();

		m_defaultKeyboardSize = pointSize;
		m_keyboardSourceSize	= align(ptsToSpx(pointSize, pOddWhiteKeys->scale()));

		m_blackKeyHeight		= m_pBlackKeys ? (m_pBlackKeys->pointSize().h / states.size()) / pointSize.h : 0;
		m_blackKeySourceHeight	= m_pBlackKeys ? (m_pBlackKeys->pixelSize().h / states.size())*64 : 0;

		//

		State usedStates[State::NbStates];

		int nbStates = 0;
		for (auto state : states)
			usedStates[nbStates++] = state;

		for (int i = 0; i < State::NbStates; i++)
		{
			int bestAlternative = State((StateEnum)i).bestMatch(nbStates, usedStates);
			m_stateOfsY[i] = bestAlternative;
		}
	}


	//____ pressKey() ________________________________________________________

	void PianoKeyboard::pressKey(int keyIdx)
	{
		if (keyIdx < 0 || keyIdx >= m_nbKeys)
		{
			//TODO: Error handling!
			return;
		}

		_setAPIPressed(keyIdx, true);
	}

	//____ releaseKey() ______________________________________________________

	void PianoKeyboard::releaseKey(int keyIdx)
	{
		if (keyIdx < 0 || keyIdx >= m_nbKeys)
		{
			//TODO: Error handling!
			return;
		}

		_setAPIPressed(keyIdx, false);
	}

	//____ isKeyPressed() ____________________________________________________

	bool PianoKeyboard::isKeyPressed(int keyIdx) const
	{
		if (keyIdx < 0 || keyIdx >= m_nbKeys)
		{
			//TODO: Error handling!
			return false;
		}

		return m_keyInfo[keyIdx].state.isPressed();
	}

	//____ setPressedKeys() __________________________________________________

	void PianoKeyboard::setPressedKeys(std::initializer_list<int> pressedKeys)
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

	void PianoKeyboard::releaseAllKeys()
	{
		for (int i = 0; i < m_nbKeys; i++)
			_setAPIPressed(i, false);
	}

	//____ setFlipKeyOnPress() ________________________________________________

	void PianoKeyboard::setFlipKeyOnPress(bool bFlip)
	{
		m_bFlipOnPress = bFlip;
	}

	//____ selectKey() ________________________________________________________

	void PianoKeyboard::selectKey(int keyIdx)
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

	void PianoKeyboard::unselectKey(int keyIdx)
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

	bool PianoKeyboard::isKeySelected(int keyIdx) const
	{
		if (keyIdx < 0 || keyIdx >= m_nbKeys)
		{
			//TODO: Error handling!
			return false;
		}

		return m_keyInfo[keyIdx].state.isSelected();
	}

	//____ setSelectedKeys() __________________________________________________

	void PianoKeyboard::setSelectedKeys(std::initializer_list<int> selectedKeys)
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

	void PianoKeyboard::unselectAllKeys()
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

	//____ _defaultSize() __________________________________________________________

	SizeSPX PianoKeyboard::_defaultSize(int scale) const
	{
		return align(ptsToSpx(m_defaultKeyboardSize,scale)) + m_skin.contentBorderSize(scale);
	}

	//____ _minSize() __________________________________________________________

	SizeSPX PianoKeyboard::_minSize(int scale) const
	{
		return align(ptsToSpx(m_defaultKeyboardSize, scale)) + m_skin.contentBorderSize(scale);
	}

	//____ _maxSize() __________________________________________________________

	SizeSPX PianoKeyboard::_maxSize(int scale) const
	{
		return SizeSPX(spx_max, spx_max);
	}

	//____ _render() __________________________________________________________

	void PianoKeyboard::_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		//TODO: This could use _keyRect() to simplify and secure consistency.

		Widget::_render(pDevice, _canvas, _window);

		RectSPX canvas = m_skin.contentRect(_canvas, m_scale, m_state);


		spx whiteKeySpacing = canvas.w / m_nbWhiteKeys;

		float xScaleFactor = m_keyboardSourceSize.w / float(canvas.w);
		float yScaleFactor = m_keyboardSourceSize.h / float(canvas.h);

		//

		if (m_pBlackKeys)
		{
			KeyInfo * pKey = m_keyInfo;

			pDevice->setBlitSource(m_pBlackKeys);

			for (int i = 0; i < m_nbWhiteKeys-1; i++)
			{
				pKey++;

				if (pKey->isBlack)
				{
					RectSPX dst = align(RectSPX(i*whiteKeySpacing + whiteKeySpacing / 2, 0, whiteKeySpacing, m_blackKeyHeight*canvas.h));

					float srcOfsY = float(m_stateOfsY[pKey->state] * m_blackKeySourceHeight);

					pDevice->stretchBlit(dst + canvas.pos(), RectSPX(dst.x*xScaleFactor, srcOfsY, dst.w*xScaleFactor, m_blackKeySourceHeight) );
					pKey++;
				}
			}
		}

		//


		KeyInfo * pFirst = &m_keyInfo[0];
		KeyInfo * pLast = &m_keyInfo[m_nbKeys-1];

		bool bForward = true;
		while (pFirst <= pLast)
		{
			KeyInfo * pKey = bForward ? pFirst : pLast;

			int i = pKey->keyPos;

			RectSPX dst = align(RectSPX(i*whiteKeySpacing - whiteKeySpacing / 2, 0, whiteKeySpacing * 2, canvas.h));

			if (i % 2 == 0)
				pDevice->setBlitSource(m_pOddWhiteKeys);
			else
				pDevice->setBlitSource(m_pEvenWhiteKeys);

			if (dst.x < 0)
			{
				dst.w += dst.x;
				dst.x = 0;
			}

			if (dst.x + dst.w > canvas.w )
				dst.w = canvas.w - dst.x;

			float srcOfsY = float(m_stateOfsY[pKey->state] * m_keyboardSourceSize.h);

			pDevice->stretchBlit(dst + canvas.pos(), RectSPX(dst.x*xScaleFactor, srcOfsY, dst.w*xScaleFactor, dst.h*yScaleFactor) );

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

	//____ _receive() _________________________________________________________

	void PianoKeyboard::_receive(Msg * pMsg)
	{
		switch (pMsg->type())
		{
			case MsgType::MouseEnter:
			case MsgType::MouseMove:
			{
				CoordSPX pos = _toLocal(static_cast<InputMsg*>(pMsg)->pointerSpxPos());
				int keyIdx = _markTestKey(pos);
				_setHoveredKey(keyIdx, static_cast<InputMsg*>(pMsg)->timestamp());
				break;
			}

			case MsgType::MouseLeave:
			{
				_setHoveredKey(-1, static_cast<InputMsg*>(pMsg)->timestamp());
				break;
			}

			case MsgType::MousePress:
			{
				if (m_hoveredKey >= 0)
				{
					if (m_bFlipOnPress)
					{
						_setAPIPressed(m_hoveredKey, !m_keyInfo[m_hoveredKey].isSetToPressed);
					
						if(m_keyInfo[m_hoveredKey].isSetToPressed)
							Base::msgRouter()->post(new PianoKeyPressMsg(this, m_hoveredKey, static_cast<InputMsg*>(pMsg)->timestamp()));
						else
							Base::msgRouter()->post(new PianoKeyReleaseMsg(this, m_hoveredKey, static_cast<InputMsg*>(pMsg)->timestamp()));
					}
					else
					{
						_setHoverPress(true, static_cast<InputMsg*>(pMsg)->timestamp());
					}
				}
				break;
			}

			case MsgType::MouseRelease:
			{
				if (m_hoveredKey >= 0)
				{
					if (!m_bFlipOnPress)
						_setHoverPress(false, static_cast<InputMsg*>(pMsg)->timestamp());
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

	void PianoKeyboard::_setHoveredKey(int keyIdx, int64_t timestamp)
	{
		if (keyIdx == m_hoveredKey)
			return;

		if (m_hoveredKey >= 0 && !m_keyInfo[m_hoveredKey].isSetToPressed)
		{
			State& state = m_keyInfo[m_hoveredKey].state;
			State oldState = state;
			state.setHovered(false);

			if (m_bIsHoveredPressed)
				Base::msgRouter()->post(new PianoKeyReleaseMsg(this, m_hoveredKey, timestamp));

			if (m_stateOfsY[oldState] != m_stateOfsY[state])
				_requestRenderKey(m_hoveredKey);
		}

		m_hoveredKey = keyIdx;

		if (m_hoveredKey >= 0)
		{
			State& state = m_keyInfo[m_hoveredKey].state;
			State oldState = state;
			state.setHovered(true);

			if (m_bIsHoveredPressed)
			{
				state.setPressed(true);
				Base::msgRouter()->post(new PianoKeyPressMsg(this, m_hoveredKey, timestamp));
			}

			if (m_stateOfsY[oldState] != m_stateOfsY[state])
				_requestRenderKey(m_hoveredKey);
		}
	}

	//____ _setHoverPress() _______________________________________________

	void PianoKeyboard::_setHoverPress(bool bPressed, int64_t timestamp)
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

			Base::msgRouter()->post(new PianoKeyPressMsg(this, m_hoveredKey, timestamp));
		}
		else if( !key.isSetToPressed )
		{
			key.state.setPressed(false);
			_requestRenderKey(m_hoveredKey);

			Base::msgRouter()->post(new PianoKeyReleaseMsg(this, m_hoveredKey, timestamp));
		}
	}

	//____ _setAPIPressed() ___________________________________________________

	void PianoKeyboard::_setAPIPressed(int keyIdx, bool bPressed)
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

	bool PianoKeyboard::_alphaTest(const CoordSPX& ofs)
	{
		if (Widget::_alphaTest(ofs) || _markTestKey(ofs) >= 0)
			return true;

		return false;
	}

	//____ _requestRenderKey() ________________________________________________

	void PianoKeyboard::_requestRenderKey(int keyIdx)
	{
		_requestRender(_keyRect(keyIdx));
	}

	//____ _markTestKey() _____________________________________________________

	int PianoKeyboard::_markTestKey(const CoordSPX& pos)
	{
		// Convert coordinate from canvas to source

		RectSPX canvas = m_skin.contentRect(m_size, m_scale, m_state);

		if (!canvas.contains(pos))
			return -1;

		float xScaleFactor = m_keyboardSourceSize.w / float(canvas.w);
		float yScaleFactor = m_keyboardSourceSize.h / float(canvas.h);

		CoordSPX srcPos = { int((pos.x - canvas.pos().x) * xScaleFactor), int((pos.y - canvas.pos().y) * yScaleFactor) };

		spx whiteKeySpacing = m_keyboardSourceSize.w / m_nbWhiteKeys;

		int keyPos = -2;
		bool isBlack = false;

		// alpha test on keys

		if (m_pOddWhiteKeys->alpha(srcPos))
			keyPos = ((srcPos.x + whiteKeySpacing / 2) / (whiteKeySpacing * 2)) * 2;
		else if (m_pEvenWhiteKeys->alpha(srcPos))
			keyPos = ((srcPos.x - whiteKeySpacing / 2) / (whiteKeySpacing * 2)) * 2 + 1;
		else if (m_pBlackKeys && (srcPos.y < m_pBlackKeys->pixelSize().h*64) && m_pBlackKeys->alpha(srcPos))
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

	RectSPX PianoKeyboard::_keyRect(int keyIdx)
	{
		RectSPX canvas = m_skin.contentRect(m_size, m_scale, m_state);

		spx whiteKeySpacing = float(canvas.w) / m_nbWhiteKeys;

		RectSPX dst;
		
		if( m_keyInfo[keyIdx].isBlack )
			dst = { m_keyInfo[keyIdx].keyPos*whiteKeySpacing + whiteKeySpacing / 2, 0, whiteKeySpacing, spx(m_blackKeyHeight*canvas.h) };
		else
			dst = { m_keyInfo[keyIdx].keyPos*whiteKeySpacing - whiteKeySpacing / 2, 0, whiteKeySpacing * 2, canvas.h };

		if (dst.x < 0)
		{
			dst.w += dst.x;
			dst.x = 0;
		}

		if (dst.x + dst.w > canvas.w )
			dst.w = canvas.w - dst.x;

		return align(dst) + canvas.pos();
	}


} // namespace wg
