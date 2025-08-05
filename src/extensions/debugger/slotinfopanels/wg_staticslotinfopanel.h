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

#ifndef	WG_STATICSLOTINFOPANEL_DOT_H
#define WG_STATICSLOTINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>

namespace wg
{
	class StaticSlotInfoPanel;
	typedef	StrongPtr<StaticSlotInfoPanel>	StaticSlotInfoPanel_p;
	typedef	WeakPtr<StaticSlotInfoPanel>	StaticSlotInfoPanel_wp;



	class StaticSlotInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static StaticSlotInfoPanel_p		create( const Blueprint& blueprint, IDebugger * pHolder, StaticSlot * pStaticSlot) { return StaticSlotInfoPanel_p(new StaticSlotInfoPanel(blueprint, pHolder, pStaticSlot) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		StaticSlotInfoPanel(const Blueprint& blueprint, IDebugger * pHolder, StaticSlot * pStaticSlot );
		~StaticSlotInfoPanel() {}
	};

} // namespace wg
#endif //WG_STATICSLOTINFOPANEL_DOT_H


