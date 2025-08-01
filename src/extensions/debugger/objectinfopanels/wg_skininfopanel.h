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

#ifndef	WG_SKININFOPANEL_DOT_H
#define WG_SKININFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>

namespace wg
{
	class SkinInfoPanel;
	typedef	StrongPtr<SkinInfoPanel>	SkinInfoPanel_p;
	typedef	WeakPtr<SkinInfoPanel>	SkinInfoPanel_wp;



	class SkinInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static SkinInfoPanel_p		create( const Blueprint& blueprint, DebugPanel::Holder* pHolder, Skin * pSkin) { return SkinInfoPanel_p(new SkinInfoPanel(blueprint, pHolder, pSkin) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		SkinInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, Skin * pSkin );
		~SkinInfoPanel() {}
	};

} // namespace wg
#endif //WG_SKININFOPANEL_DOT_H

