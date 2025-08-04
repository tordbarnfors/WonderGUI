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

#ifndef	WG_SKINTESTPANEL_DOT_H
#define WG_SKINTESTPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>

namespace wg
{
	class SkinTestPanel;
	typedef	StrongPtr<SkinTestPanel>	SkinTestPanel_p;
	typedef	WeakPtr<SkinTestPanel>	SkinTestPanel_wp;



	class SkinTestPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static SkinTestPanel_p		create( const Blueprint& blueprint, DebugPanel::Holder * pHolder, Skin * pSkin) { return SkinTestPanel_p(new SkinTestPanel(blueprint, pHolder, pSkin) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		SkinTestPanel(const Blueprint& blueprint, DebugPanel::Holder * pHolder, Skin * pSkin );
		~SkinTestPanel() {}
	};

} // namespace wg
#endif //WG_SKINTESTPANEL_DOT_H


