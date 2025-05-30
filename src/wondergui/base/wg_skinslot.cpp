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

#include <wg_skinslot.h>
#include <wg_base.h>

#include <assert.h>

namespace wg
{

	//____ set() _________________________________________________________

	void SkinSlot::set(Skin* pSkin)
	{
		if (pSkin == m_pSkin)
			return;

		Skin_p pOldSkin = m_pSkin;

		// Replace the skin

		if (m_pSkin)
			m_pSkin->_decUseCount();

		m_pSkin = pSkin;

		if (m_pSkin)
			m_pSkin->_incUseCount();

		// Check if we need a pocket and initialize or remove

		if (pSkin)
		{
			State state = m_pHolder->_skinState(this);

			if (pSkin->_animationLength(state))
			{
				_initPocket(state);
				m_pPocket->bAnimated = true;
			}
			else
				_releasePocket();

		}
		else if (m_pPocket)
			_releasePocket();
	}

	//____ stateChanged() _____________________________________________________

	void SkinSlot::stateChanged(State newState, State oldState)
	{
		if (!m_pSkin || newState == oldState || m_pSkin->_ignoresState() )
			return;

		if( newState.isPressed() != oldState.isPressed() )
		{
			int dummy = 0;
		}
		
		// Update transitions based on state change.

		Bitmask<uint8_t> changedStatesMask = newState.primStates() ^ oldState.primStates();

		if (m_pSkin->_transitioningStates() & changedStatesMask )
		{
			auto pTransitionTimes = m_pSkin->_transitionTimes();

			// Check if this state change starts any new transition

			bool bStartTransition = false;
			for (int stateIndex = 0; stateIndex < PrimState_Nb; stateIndex++)
			{
				if (changedStatesMask.bit(stateIndex) && pTransitionTimes[stateIndex] != 0)
				{
					bStartTransition = true;
					break;
				}
			}

			// Update transitions

			if (bStartTransition)
			{
				// Make sure we have a pocket

				if (!m_pPocket)
					_initPocket(oldState);

				m_pPocket->transitionTo = newState.primStates();

				// Step through statebits and update transition progress

				for (int i = 0; i < PrimState_Nb; i++)
				{
					if (changedStatesMask.bit(i))
					{
						if (pTransitionTimes[i] == 0 || (m_pPocket->fractionalState[i] > 0.f && m_pPocket->fractionalState[i] < 1.f) )
							m_pPocket->transitionFrom.flipBit(i);
					}
				}
			}
		}

		// Check if our skin is animated in newState.

		if (m_pSkin->_animationLength(newState))
		{
			if (!m_pPocket)
				_initPocket(newState);

			m_pPocket->bAnimated = true;
		}
		else
		{
			// Release pocket if there is no transition nor animation.

			if (m_pPocket && m_pPocket->transitionFrom == m_pPocket->transitionTo)
				_releasePocket();
		}

		// Check if we need to update skin right away due to state change.

		RectSPX	canvas		= m_pHolder->_skinSize(this);
		float	value1		= static_cast<SkinSlot::Holder*>(m_pHolder)->_skinValue(this);
		float	value2		= static_cast<SkinSlot::Holder*>(m_pHolder)->_skinValue2(this);

		int		animPos = m_pPocket ? m_pPocket->animationCounter : 0;
		float* pStateFractions = m_pPocket ? m_pPocket->fractionalState : nullptr;


		RectSPX dirtyRect = m_pSkin->_dirtyRect(canvas, m_pHolder->_scale(), oldState, newState, value1, value1, value2, value2, animPos, animPos, pStateFractions, pStateFractions);
		if (!dirtyRect.isEmpty())
			m_pHolder->_skinRequestRender(this, dirtyRect);
	}

	//____ valueChanged() _____________________________________________________

	void SkinSlot::valueChanged(float newValue, float oldValue, float newValue2, float oldValue2)
	{
		if (!m_pSkin || m_pSkin->_ignoresValue())
			return;

		RectSPX	canvas = m_pHolder->_skinSize(this);
		State	state = m_pHolder->_skinState(this);

		int		animPos = m_pPocket ? m_pPocket->animationCounter : 0;
		float*	pStateFractions = m_pPocket ? m_pPocket->fractionalState : nullptr;

		RectSPX dirtyRect = m_pSkin->_dirtyRect(canvas, m_pHolder->_scale(), state, state, newValue, oldValue, newValue2, oldValue2, animPos, animPos, pStateFractions, pStateFractions);
		if (!dirtyRect.isEmpty())
			m_pHolder->_skinRequestRender(this, dirtyRect);

	}

	//____ render() ___________________________________________________________

	void SkinSlot::render(GfxDevice* pDevice, const RectSPX& canvas, int scale, State state, float value, float value2) const
	{
		if (m_pSkin)
		{
			int animPos = m_pPocket ? m_pPocket->animationCounter : 0;
			float* pFractionalState = m_pPocket ? m_pPocket->fractionalState : nullptr;

			m_pSkin->_render(pDevice, canvas, scale, state, value, value2, animPos, pFractionalState);
		}
	}

	//____ _update() __________________________________________________________

	bool SkinSlot::_update(SkinSlotPocket * pPocket, int msPassed)
	{
		assert(pPocket == m_pPocket);

		// Prepare our dirtyRect check

		int oldAnimPos = 0;
		int newAnimPos = 0;

		float* pOldStateFractions = nullptr;
		float* pNewStateFractions = nullptr;

		float oldFractionalState[PrimState_Nb];

		// Update possible state transition

		if (m_pPocket->transitionFrom != m_pPocket->transitionTo)
		{
			auto pTransitionTimes = m_pSkin->_transitionTimes();

			for (int i = 0; i < PrimState_Nb; i++)
			{
				oldFractionalState[i] = m_pPocket->fractionalState[i];

				if (m_pPocket->transitionFrom.bit(i) != m_pPocket->transitionTo.bit(i) )
				{
					if (m_pPocket->transitionTo.bit(i))
					{
						m_pPocket->fractionalState[i] += float(msPassed) / pTransitionTimes[i];
						if (m_pPocket->fractionalState[i] >= 1.f)
						{
							m_pPocket->fractionalState[i] = 1.f;
							m_pPocket->transitionFrom.flipBit(i);
						}
					}
					else
					{
						m_pPocket->fractionalState[i] -= float(msPassed) / pTransitionTimes[i];
						if (m_pPocket->fractionalState[i] <= 0.f)
						{
							m_pPocket->fractionalState[i] = 0.f;
							m_pPocket->transitionFrom.flipBit(i);
						}
					}

				}
			}

			pOldStateFractions = oldFractionalState;
			pNewStateFractions = m_pPocket->fractionalState;
		}

		// Update animation

		if (m_pPocket->bAnimated)
		{
			oldAnimPos = m_pPocket->animationCounter;
			m_pPocket->animationCounter += msPassed;
			newAnimPos = m_pPocket->animationCounter;
		}

		// Request render if needed

		RectSPX	canvas = m_pHolder->_skinSize(this);
		State	state = m_pHolder->_skinState(this);
		float	value1 = static_cast<SkinSlot::Holder*>(m_pHolder)->_skinValue(this);
		float	value2 = static_cast<SkinSlot::Holder*>(m_pHolder)->_skinValue2(this);

		float	animPos = m_pPocket ? m_pPocket->animationCounter : 0.f;
		float* pStateFractions = m_pPocket ? m_pPocket->fractionalState : nullptr;

		RectSPX dirtyRect = m_pSkin->_dirtyRect(canvas, m_pHolder->_scale(), state, state, value1, value1, value2, value2, newAnimPos, oldAnimPos, pNewStateFractions, pOldStateFractions);
		if (!dirtyRect.isEmpty())
			m_pHolder->_skinRequestRender(this, dirtyRect);

		return (m_pPocket->bAnimated == false && m_pPocket->transitionFrom == m_pPocket->transitionTo);
	}

	//____ _willRemovePocket() ___________________________________________________

	void SkinSlot::_willRemovePocket(SkinSlotPocket* pPocket)
	{
		m_pPocket = nullptr;
	}


	//____ _initPocket() ______________________________________________________

	void SkinSlot::_initPocket(State state)
	{
		if (!m_pPocket)
		{
			m_pPocket = Base::skinSlotManager()->allocPocket();
			m_pPocket->pHolder = this;
		}

		Bitmask<uint8_t> statemask = state.primStates();

		m_pPocket->transitionFrom = statemask;
		m_pPocket->transitionTo = statemask;

		for (int i = 0; i < PrimState_Nb; i++)
			m_pPocket->fractionalState[i] = statemask.bit(i) ? 1.f : 0.f;

		m_pPocket->bAnimated = false;
		m_pPocket->animationCounter = 0;
	}

	//____ _releasePocket() ___________________________________________________

	void SkinSlot::_releasePocket()
	{
		if (m_pPocket)
		{
			Base::skinSlotManager()->freePocket(m_pPocket);
			m_pPocket = nullptr;
		}
	}


};
