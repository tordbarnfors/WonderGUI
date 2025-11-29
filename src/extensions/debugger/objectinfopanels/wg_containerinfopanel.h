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
#ifndef	WG_CONTAINERINFOPANEL_DOT_H
#define WG_CONTAINERINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>

namespace wg
{
	class ContainerInfoPanel;
	typedef	StrongPtr<ContainerInfoPanel>	ContainerInfoPanel_p;
	typedef	WeakPtr<ContainerInfoPanel>	ContainerInfoPanel_wp;



	class ContainerInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static ContainerInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, Container * pContainer) { return ContainerInfoPanel_p(new ContainerInfoPanel(blueprint, pHolder, pContainer) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		ContainerInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Container * pContainer );
		~ContainerInfoPanel() {}

		Container *		m_pInspected;
		TablePanel_p	m_pTable;
	};

} // namespace wg
#endif //WG_CONTAINERINFOPANEL_DOT_H

