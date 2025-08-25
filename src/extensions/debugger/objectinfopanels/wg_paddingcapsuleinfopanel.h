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

#ifndef	WG_PADDINGCAPSULEINFOPANEL_DOT_H
#define WG_PADDINGCAPSULEINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_paddingcapsule.h>

namespace wg
{
	class PaddingCapsuleInfoPanel;
	typedef	StrongPtr<PaddingCapsuleInfoPanel>	PaddingCapsuleInfoPanel_p;
	typedef	WeakPtr<PaddingCapsuleInfoPanel>	PaddingCapsuleInfoPanel_wp;



	class PaddingCapsuleInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static PaddingCapsuleInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, PaddingCapsule * pPanel) { return PaddingCapsuleInfoPanel_p(new PaddingCapsuleInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		PaddingCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, PaddingCapsule * pPanel );
		~PaddingCapsuleInfoPanel() {}

		PaddingCapsule *	m_pInspected;
		DrawerPanel_p		m_pPaddingDrawer;

		Border				m_displayedPadding;
	};

} // namespace wg
#endif //WG_PADDINGCAPSULEINFOPANEL_DOT_H

