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

#ifndef	WG_CAPSULEINFOPANEL_DOT_H
#define WG_CAPSULEINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_capsule.h>

namespace wg
{
	class CapsuleInfoPanel;
	typedef	StrongPtr<CapsuleInfoPanel>	CapsuleInfoPanel_p;
	typedef	WeakPtr<CapsuleInfoPanel>	CapsuleInfoPanel_wp;



	class CapsuleInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static CapsuleInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, Capsule * pPanel) { return CapsuleInfoPanel_p(new CapsuleInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		CapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Capsule * pPanel );
		~CapsuleInfoPanel() {}

		Capsule *		m_pInspected;
		DrawerPanel_p	m_pSlotDrawer;

	};

} // namespace wg
#endif //WG_CAPSULEINFOPANEL_DOT_H

