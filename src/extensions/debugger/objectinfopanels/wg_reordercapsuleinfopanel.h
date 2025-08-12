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

#ifndef	WG_REORDERCAPSULEINFOPANEL_DOT_H
#define WG_REORDERCAPSULEINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_reordercapsule.h>

namespace wg
{
	class ReorderCapsuleInfoPanel;
	typedef	StrongPtr<ReorderCapsuleInfoPanel>	ReorderCapsuleInfoPanel_p;
	typedef	WeakPtr<ReorderCapsuleInfoPanel>	ReorderCapsuleInfoPanel_wp;



	class ReorderCapsuleInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static ReorderCapsuleInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, ReorderCapsule * pPanel) { return ReorderCapsuleInfoPanel_p(new ReorderCapsuleInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		ReorderCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, ReorderCapsule * pPanel );
		~ReorderCapsuleInfoPanel() {}

		ReorderCapsule *	m_pInspected;
		TablePanel_p		m_pTable;

		Object_p			m_displayedTransitionPtr;
		Object_p			m_displayedTransitionSkinPtr;
	};

} // namespace wg
#endif //WG_REORDERCAPSULEINFOPANEL_DOT_H

