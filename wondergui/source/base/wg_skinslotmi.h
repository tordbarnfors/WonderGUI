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
#ifndef	WG_CSKINSLOTMI_DOT_H
#define	WG_CSKINSLOTMI_DOT_H
#pragma once

#include <wg_skin.h>
#include <wg_skinslotmanager.h>

namespace wg
{
	class SkinSlotMI : protected SkinSlotPocket::Holder
	{
	public:
		class Holder
		{
		public:
			virtual Object* _object() = 0;
			virtual const Object* _object() const = 0;

			virtual void	_skinChanged(const SkinSlotMI* pSlot, Skin* pNewSkin, Skin* pOldSkin) = 0;

			virtual State	_skinInstanceState(void* instance) = 0;
			virtual SizeSPX	_skinInstanceSize(void* instance) = 0;
			virtual void	_skinInstanceRequestRender(void* instance) = 0;
			virtual void	_skinInstanceRequestRender(void* instance, const RectSPX& rect) = 0;
			virtual float	_skinInstanceValue(void* instance) = 0;
			virtual float	_skinInstanceValue2(void* instance) = 0;
			virtual int		_scale() = 0;
		};

		SkinSlotMI(Holder* pHolder) : m_pHolder(pHolder) {}
		~SkinSlotMI() { if (m_pSkin) m_pSkin->_decUseCount(); }


		//.____ Content _______________________________________________________

		virtual void	set(Skin* pSkin);
		inline bool		isEmpty() const { return m_pSkin == nullptr; }

		//.____ Operators __________________________________________

		inline const Skin* operator->() const { return m_pSkin; }
		inline operator Skin* () const { return m_pSkin; }

		inline SkinSlotMI& operator=(Skin* pSkin) { set(pSkin); return *this; }
		inline operator Skin_p() const { return m_pSkin; }

	protected:

		// Methods for holder

		void			_addInstance(SkinSlotPocket*& pPocket, void* instance, State state);
		inline void		_removeInstance(SkinSlotPocket*& pPocket) { _releasePocket(pPocket); }
		void			_changeInstance(SkinSlotPocket*& pPocket, void* newInstance);

		void			_stateChanged(SkinSlotPocket*& pPocket, void* instance, State newState, State oldState);
		void			_valueChanged(SkinSlotPocket*& pPocket, void* instance, float newValue, float oldValue, float newValue2 = -1, float oldValue2 = -1);

		void 			_render(SkinSlotPocket* pPocket, GfxDevice* pDevice, const RectSPX& canvas, int scale, State state, float value = 1.f, float value2 = -1.f) const;

		inline SizeSPX	_minSize(int scale) const { return m_pSkin ? m_pSkin->_minSize(scale) : SizeSPX(); }
		inline SizeSPX	_defaultSize(int scale) const { return m_pSkin ? m_pSkin->_defaultSize(scale) : SizeSPX(); }

		inline SizeSPX	_contentBorderSize(int scale) const { return m_pSkin ? m_pSkin->_contentBorderSize(scale) : SizeSPX(); }
		inline SizeSPX	_sizeForContent(const SizeSPX& contentSize, int scale) const { return m_pSkin ? m_pSkin->_sizeForContent(contentSize, scale) : contentSize; }
		inline CoordSPX	_contentOfs(int scale, State state) const { return m_pSkin ? m_pSkin->_contentOfs(scale, state) : CoordSPX(); }
		inline RectSPX	_contentRect(const RectSPX& canvas, int scale, State state) const { return m_pSkin ? m_pSkin->_contentRect(canvas, scale, state) : canvas; }


		inline bool		_markTest(const CoordSPX& ofs, const RectSPX& canvas, int scale, State state,
			float value = 1.f, float value2 = -1.f) const
		{
			return m_pSkin ? m_pSkin->_markTest(ofs, canvas, scale, state, value, value2) : false;
		}

		inline bool		_isContentShifting() { return m_pSkin ? m_pSkin->_isContentShifting() : false; }

		//

		bool	_update(SkinSlotPocket * pPocket, int msPassed) override;
		void	_willRemovePocket(SkinSlotPocket* pPocket) override;

		void	_initPocket(SkinSlotPocket*& pPocket, void * instance, State state);
		void	_releasePocket(SkinSlotPocket*& pPocket);

		//

		Skin_p	m_pSkin;
		Holder* m_pHolder;

	};



};

#endif //WG_CSKINSLOTMI_DOT_H
