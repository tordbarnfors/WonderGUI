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

#ifndef	WG_DUMMYINFOPANEL_DOT_H
#define WG_DUMMYINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>

namespace wg
{
	class DummyInfoPanel;
	typedef	StrongPtr<DummyInfoPanel>	DummyInfoPanel_p;
	typedef	WeakPtr<DummyInfoPanel>	DummyInfoPanel_wp;



	class DummyInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static DummyInfoPanel_p		create( const Blueprint& blueprint, DebugPanel::Holder * pHolder, void * pDummy) { return DummyInfoPanel_p(new DummyInfoPanel(blueprint, pHolder, pDummy) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		DummyInfoPanel(const Blueprint& blueprint, DebugPanel::Holder * pHolder, void * pDummy );
		~DummyInfoPanel() {}
	};

} // namespace wg
#endif //WG_DUMMYINFOPANEL_DOT_H


